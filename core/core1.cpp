#include "core1.hpp"

#include <cstdio>

#include <pico/multicore.h>
#include <pico/stdio.h>

#include <hardware/sync.h>

#include <tusb.h>

#include <badge/lcd.hpp>

namespace
{

    // Because of how the pico SDK and micropython port is set up, we allocate all the "scratch"
    // memory to core0 (8 KiB) and put a smaller core1 stack somewhere else in RAM.
    uint32_t _core1_stack[core1::CORE1_STACK_SIZE / sizeof(uint32_t)];

    struct CoreSync {

        spin_lock_t *_lock = nullptr;
        uint32_t _saved_irq = 0;

        volatile bool protected_ready = false;
        volatile bool protected_want_swap = false;

        void init() {
            // Grab a spinlock instance if we haven't already.
            if (_lock == nullptr) {
                const auto lock_num = spin_lock_claim_unused(true);
                _lock = spin_lock_instance(lock_num);
            }

            // Reset state.
            protected_ready = false;
            protected_want_swap = false;
        }

        void lock() {
            _saved_irq = spin_lock_blocking(_lock);
        }

        void unlock() {
            spin_unlock(_lock, _saved_irq);
        }

        void do_protected(auto action) {
            lock();
            action();
            unlock();
        }

        void lock_when(auto predicate) {
            while (true) {
                lock();
                if (predicate())
                    break;
                unlock();
                sleep_us(10);
            }
        }

        void wait_until(auto predicate) {
            lock_when(predicate);
            unlock();
        }
    };

    CoreSync _sync = {};

    [[noreturn]] void core1_main() {
        printf("> Core 1 starting...\n");

        lcd::internal::init();
        lcd::internal::exit_sleep();
        lcd::internal::display_on();
        lcd::internal::read_id();
        lcd::internal::read_status();

        lcd::backlight_on(20);

        printf("> Core 1 running...\n");
        stdio_flush();

        _sync.do_protected([]{ _sync.protected_ready = true; });

        while (true) {

            // Wait until core0 says it wants to do a swap.
            _sync.lock_when([] { return _sync.protected_want_swap; });

            // Start the swap by exchanging buffers. This is the fast part that we do while holding the lock.
            lcd::internal::begin_swap();

            // It's now safe to clear the swap flag and release the lock.
            _sync.protected_want_swap = false;
            _sync.unlock();

            // Finish the swap by writing to the LCD. This is the slow part that we do while core0 does other stuff.
            lcd::internal::end_swap();
        }
    }

}

namespace core1
{

    void reset_and_launch() {
        printf("> Launching core 1...\n");

        // Reset core1 first, in case it's actually running (which it REALLY shouldn't).
        multicore_reset_core1();

        // Initialize/reset our sync data structure.
        _sync.init();

        // Launch core1 code.
        multicore_launch_core1_with_stack(core1_main, _core1_stack, sizeof(_core1_stack));

        // Wait until core1 says it's ready for us to continue.
        _sync.wait_until([]{ return _sync.protected_ready; });
    }

    void swap_frame() {
        // Grab the lock, and set the flag to indicate we want to swap buffers.
        _sync.do_protected([]{ _sync.protected_want_swap = true; });

        // Now wait until core1 clears the flag to indicate it's done with the swap and we can continue.
        _sync.wait_until([] {
            while (tud_task_event_ready())
                tud_task();
            return _sync.protected_want_swap == false;
        });
    }

}