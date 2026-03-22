# --- Multiboot2 Header ---
.section .multiboot_header
.align 8
header_start:
    .long 0xe85250d6                # Magic number (multiboot2)
    .long 0                         # Architecture 0 (i386 protected mode)
    .long header_end - header_start # Header length
    # Checksum: -(magic + arch + length)
    .long 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

    # End tag (required by spec)
    .short 0    # type
    .short 0    # flags
    .long 8     # size
header_end:

# --- Kernel Entry Point ---
.section .text
.global _start
.extern kernel_main

_start:
    # 1. Setup the stack
    mov $stack_top, %esp

    # 2. Push Multiboot info for your C code
    # We use 64-bit registers because you're targeting x86_64
    push %rbx   # Pointer to Multiboot info structure
    push %rax   # Magic number (0x36d76289)

    # 3. Enter the C Kernel
    call kernel_main

    # 4. If kernel_main returns, halt the CPU
    cli
hang:
    hlt
    jmp hang

# --- Stack Reserved Space ---
.section .bss
.align 16
stack_bottom:
    .skip 16384 # 16KB stack
stack_top:

