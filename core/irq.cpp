#include <cstdio>

#include <pico.h>
#include <hardware/irq.h>
#include <tusb.h>


extern "C" [[noreturn]] void __attribute__ ((naked)) isr_handler()
{
    uint32_t sp, lr, ipsr;
    asm volatile("mov %0, sp" : "=r"(sp));
    asm volatile("mov %0, lr" : "=r"(lr));
    asm volatile("mrs %0, ipsr" : "=r"(ipsr));

    const auto *stack = reinterpret_cast<uint32_t *>(sp);
    const auto r0 = stack[0];
    const auto r1 = stack[1];
    const auto r2 = stack[2];
    const auto r3 = stack[3];
    const auto r12 = stack[4];
    const auto old_lr = stack[5];
    const auto ret_addr = stack[6];
    const auto xspr = stack[7];

    const auto cpuid = *reinterpret_cast<uint32_t *>(0xD0000000);
    const auto icsr = scb_hw->icsr;
    printf(
        "\n"
        "\n"
        "! CORE %d IRQ\n"
        "! ICSR = 0x%08x\n"
        "! SP   = 0x%08x\n"
        "! LR'  = 0x%08x\n"
        "! R0   = 0x%08x\n"
        "! R1   = 0x%08x\n"
        "! R2   = 0x%08x\n"
        "! R3   = 0x%08x\n"
        "! R12  = 0x%08x\n"
        "! LR   = 0x%08x\n"
        "! Ret. = 0x%08x\n"
        "! xPSR = 0x%08x\n"
        "! IPSR = 0x%08x\n"
        "\n"
        "\n",
        cpuid, icsr,
        sp, lr,
        r0, r1, r2, r3, r12, old_lr, ret_addr, xspr,
        ipsr
    );

    while (true) tight_loop_contents();
}

extern "C" [[noreturn]] void __attribute__ ((naked)) isr_hardfault() __attribute__((alias("isr_handler")));
extern "C" [[noreturn]] void __attribute__ ((naked)) isr_nmi() __attribute__((alias("isr_handler")));
// extern "C" [[noreturn]] void __attribute__ ((naked)) isr_svcall() __attribute__((alias("isr_handler")));
// extern "C" [[noreturn]] void __attribute__ ((naked)) isr_svpend() __attribute__((alias("isr_handler")));
// extern "C" [[noreturn]] void __attribute__ ((naked)) isr_systick() __attribute__((alias("isr_handler")));

extern "C" void isr_usbctrl() {
    tud_int_handler(0);
}
