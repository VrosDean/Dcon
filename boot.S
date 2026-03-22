# --- Multiboot2 Header ---
# Must be 64-bit aligned and within the first 32KB of the file
.section .multiboot_header
.align 8
header_start:
    .long 0xe85250d6                # Magic number (multiboot2)
    .long 0                         # Architecture 0 (i386 protected mode)
    .long header_end - header_start # Header length
    # Checksum: -(magic + arch + length)
    .long 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

    # End tag (required)
    .short 0    # type
    .short 0    # flags
    .long 8     # size
header_end:

# --- Kernel Entry Point ---
.section .text
.global _start
.extern kernel_main

_start:
    # 1. Initialize the stack pointer
    # C code requires a valid stack for local variables and function calls
    mov $stack_top, %esp

    # 2. Push Multiboot info (optional)
    # GRUB puts the magic number in EAX and info pointer in EBX
    push %rbx    # Push Multiboot info pointer (now 64-bit)
    push %rax    # Push Magic number (now 64-bit)

    # 3. Call your C kernel
    call kernel_main

    # 4. Halt if kernel_main ever returns
    cli
halt_loop:
    hlt
    jmp halt_loop

# --- Stack Space ---
.section .bss
.align 16
stack_bottom:
    .skip 16384                     # Reserve 16KB for the stack
stack_top:

