/* The bootloader starts at the _start symbol defined in boot.s */
ENTRY(_start)

SECTIONS
{
    /* We begin putting sections at 1 MiB, a conventional for kernels */
    . = 1M;

    /* First and most important: the code section */
    .text BLOCK(4K) : ALIGN(4K)
    {
        /* Force the multiboot header to the very start of the file */
        KEEP(*(.multiboot))
        *(.text)
    }

    /* Read-only data (your Hello World string lives here) */
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    /* Initialized read-write data */
    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    /* Uninitialized read-write data and stack */
    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }

    /* The compiler may produce other sections, by default we put them here. */
}

