/* --- DCON v1.0 MASTER --- */
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void outb(unsigned short port, unsigned char data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

/* STORAGE */
void write_disk(int LBA, unsigned short* source) {
    outb(0x1F6, (0xE0 | ((LBA >> 24) & 0x0F)));
    outb(0x1F2, 1);
    outb(0x1F3, (unsigned char)LBA);
    outb(0x1F4, (unsigned char)(LBA >> 8));
    outb(0x1F5, (unsigned char)(LBA >> 16));
    outb(0x1F7, 0x30); 
    while (inb(0x1F7) & 0x80);
    while (!(inb(0x1F7) & 0x08));
    for (int i = 0; i < 256; i++) {
        outb(0x1F0, (unsigned char)(source[i] & 0xFF));
        outb(0x1F0, (unsigned char)((source[i] >> 8) & 0xFF));
    }
    outb(0x1F7, 0xE7);
}

/* MEMORY */
void* dcon_malloc(); // (Code from above)

/* --- Rest of Dean's Kernel (Rainbow, Keyboard, Main) --- */
void kernel_main() {
    // ... draw_rainbow ...
    print_at("DCON v1.0", 12, 25);
    // ... check_keyboard ...
}

