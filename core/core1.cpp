#include "core/core1.hpp"

#include <cstdio>

#include <pico/multicore.h>
#include <pico/stdio.h>

#include <hardware/sync.h>

#include "board/lcd.hpp"

namespace
{

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

    void dumb_irq_handler() {
        uint32_t ipsr;
        asm volatile("mrs %0, ipsr" : "=r"(ipsr));
        printf("\n\n! CORE 1 IRQ - IPSR = 0x%08x\n\n", ipsr);
    }

    extern "C" uint32_t __StackTop;
    extern "C" uint32_t __StackBottom;
    extern "C" uint32_t __StackOneTop;
    extern "C" uint32_t __StackOneBottom;

    [[noreturn]] void core1_main() {
        printf("> Core 1 starting...\n");
        uint32_t sp;
        asm volatile("mov %0, sp" : "=r"(sp));
        printf("  __StackOneTop = 0x%08x\n", reinterpret_cast<intptr_t>(&__StackOneTop));
        printf("  __StackOneBottom = 0x%08x\n", reinterpret_cast<intptr_t>(&__StackOneBottom));
        printf("  SP = 0x%08x\n", sp);

        /*
        for (int i = 0; i <= 25; i++) {
            const auto current = irq_get_vtable_handler(i);
            if (current == __unhandled_user_irq) {
                printf("  Setting IRQ %d\n", i);
                irq_set_exclusive_handler(i, dumb_irq_handler);
            }
            else {
                printf("  Existing IRQ %d = 0x%08x\n", i, reinterpret_cast<intptr_t>(current));
            }
        }
        */

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
            _sync.lock_when([]{ return _sync.protected_want_swap;});

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
        // Reset core1 first, in case it's actually running (which it REALLY shouldn't).
        multicore_reset_core1();

        printf("> Launching core 1...\n");
        uint32_t sp;
        asm volatile("mov %0, sp" : "=r"(sp));
        printf("  __StackTop = 0x%08x\n", reinterpret_cast<intptr_t>(&__StackTop));
        printf("  __StackBottom = 0x%08x\n", reinterpret_cast<intptr_t>(&__StackBottom));
        printf("  SP = 0x%08x\n", sp);

        // Initialize/reset our sync data structure.
        _sync.init();

        // Launch core1 code.
        multicore_launch_core1(core1_main);

        // Wait until core1 says it's ready for us to continue.
        _sync.wait_until([]{ return _sync.protected_ready; });
    }

    void swap_frame() {
        // Grab the lock, and set the flag to indicate we want to swap buffers.
        _sync.do_protected([]{ _sync.protected_want_swap = true; });

        // Now wait until core1 clears the flag to indicate it's done with the swap and we can continue.
        _sync.wait_until([]{ return _sync.protected_want_swap == false; });
    }

}