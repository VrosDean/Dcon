ENTRY(_start)

SECTIONS
{
    /* GRUB and most bootloaders expect the kernel at 1MB */
    . = 1M;

    .text BLOCK(4K) : ALIGN(4K)
    {
        /* Force the multiboot header to the absolute top of the file */
        KEEP(*(.multiboot_header))
        *(.text)
    }

    /* Read-only data */
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    /* Initialized data */
    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    /* Unitialized data (stack goes here) */
    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }
}

