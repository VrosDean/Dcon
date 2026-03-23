ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
    /* Start at 1MB */
    . = 1M;

    /* The 'boot' section MUST be at the very top */
    .boot :
    {
        KEEP(*(.multiboot_header))
    }

    .text :
    {
        *(.text)
    }

    .rodata : { *(.rodata) }
    .data   : { *(.data)   }
    .bss    : { *(.bss)    }
}

