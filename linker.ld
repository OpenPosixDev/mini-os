OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
  . = 0x00100000;

  .text ALIGN(0x1000) :
  {
    *(.multiboot)
    *(.text)
    *(.text.*)
  }

  .rodata ALIGN(0x1000) :
  {
    *(.rodata)
    *(.rodata.*)
  }

  .data ALIGN(0x1000) :
  {
    *(.data)
    *(.data.*)
  }

  .bss ALIGN(0x1000) :
  {
    __bss_start = .;
    *(.bss)
    *(.bss.*)
    *(COMMON)
    __bss_end = .;
  }

  /DISCARD/ : { *(.eh_frame) }
}