# --- MULTIBOOT2 HEADER ---
.section .multiboot_header
.align 8
header_start:
    .long 0xe85250d6                # Magic number (multiboot2)
    .long 0                         # Architecture 0 (i386 protected mode)
    .long header_end - header_start # Header length
    .long 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) # Checksum

    # End tag (required by spec)
    .short 0, 0
    .long 8
header_end:

# --- KERNEL START ---
.section .text
.global _start
.extern kernel_main

_start:
    # Set up stack pointer (ESP) before calling C
    mov $stack_top, %esp

    # Push arguments for kernel_main(magic, info_addr)
    push %ebx   # Pointer to Multiboot info structure
    push %eax   # Magic number to verify bootloader

    # Enter C logic
    call kernel_main

    # If kernel_main returns, stop the CPU
    cli
halt_loop:
    hlt
    jmp halt_loop

# --- STACK RESERVATION ---
.section .bss
.align 16
stack_bottom:
    .skip 16384 # 16KB stack
stack_top:

