#include "vga.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* VGA text-mode buffer at physical 0xB8000 */
static uint16_t *const VGA_BUFFER = (uint16_t *)0xB8000;

static size_t terminal_row = 0;
static size_t terminal_column = 0;
static uint8_t terminal_color = 0x07; /* light grey on black */

/* low-level port I/O used for hardware cursor */
static inline void outb(uint16_t port, uint8_t value)
{
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void update_hardware_cursor(void)
{
    uint16_t pos = (uint16_t)(terminal_row * VGA_WIDTH + terminal_column);
    outb(0x3D4, 14);
    outb(0x3D5, (uint8_t)(pos >> 8));
    outb(0x3D4, 15);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
}

uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
    return fg | (bg << 4);
}

uint16_t vga_entry(unsigned char uc, uint8_t color)
{
    return (uint16_t)uc | ((uint16_t)color << 8);
}

void vga_setcolor(uint8_t fg, uint8_t bg)
{
    terminal_color = vga_entry_color((enum vga_color)fg, (enum vga_color)bg);
}

static void vga_scroll_if_needed(void)
{
    if (terminal_row < VGA_HEIGHT)
        return;

    /* scroll up by one line */
    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            VGA_BUFFER[(y - 1) * VGA_WIDTH + x] = VGA_BUFFER[y * VGA_WIDTH + x];
        }
    }

    /* clear last line */
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        VGA_BUFFER[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
    }

    terminal_row = VGA_HEIGHT - 1;
}

void vga_putc(char c)
{
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        vga_scroll_if_needed();
        update_hardware_cursor();
        return;
    }

    if (c == '\r') {
        terminal_column = 0;
        update_hardware_cursor();
        return;
    }

    if (c == '\t') {
        size_t spaces = 4 - (terminal_column % 4);
        for (size_t i = 0; i < spaces; i++)
            vga_putc(' ');
        return;
    }

    if ((unsigned char)c < ' ' ) {
        /* non-printable, skip */
        return;
    }

    VGA_BUFFER[terminal_row * VGA_WIDTH + terminal_column] = vga_entry((uint8_t)c, terminal_color);
    terminal_column++;
    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
        vga_scroll_if_needed();
    }
    update_hardware_cursor();
}

void vga_puts(const char *s)
{
    for (const char *p = s; *p; ++p)
        vga_putc(*p);
}

void vga_clear(void)
{
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            VGA_BUFFER[y * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
        }
    }
    terminal_row = 0;
    terminal_column = 0;
    update_hardware_cursor();
}

void vga_init(void)
{
    vga_clear();
}

/* small helpers to print numbers */
static void vga_putc_hex_digit(uint8_t d)
{
    if (d < 10) vga_putc('0' + d);
    else vga_putc('A' + (d - 10));
}

void vga_write_hex(uint32_t value)
{
    vga_puts("0x");
    bool started = false;
    for (int i = 7; i >= 0; --i) {
        uint8_t nibble = (value >> (i * 4)) & 0xF;
        if (!started && nibble == 0 && i != 0) continue;
        started = true;
        vga_putc_hex_digit(nibble);
    }
}

void vga_write_dec(int32_t value)
{
    if (value == 0) {
        vga_putc('0');
        return;
    }

    if (value < 0) {
        vga_putc('-');
        value = -value;
    }

    char buf[12];
    int i = 0;
    while (value > 0 && i < (int)sizeof(buf)) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }
    while (--i >= 0) vga_putc(buf[i]);
}
