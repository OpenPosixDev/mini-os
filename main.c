#include "vga.h"

/* Minimal kernel entry printing Hello, World! using the VGA text-mode driver.
 * This file assumes the build/linker script will call kernel_main as the entry
 * point (or you rename/adapt as needed).
 */

void kernel_main(void)
{
    /* initialize VGA driver and print message */
    vga_init();
    vga_setcolor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts("Hello, World!\n");

    /* Halt the CPU in an infinite loop */
    for (;;) {
        asm volatile ("hlt");
    }
}
