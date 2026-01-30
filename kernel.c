/* --- Hardware Communication --- */
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

/* --- Screen Display Helpers --- */
volatile char* vidptr = (volatile char*)0xb8000;

void print_at(const char* message, int row, int col) {
    int index = (row * 80 + col) * 2;
    int i = 0;
    while (message[i] != '\0') {
        unsigned char existing_color = vidptr[index + 1];
        vidptr[index] = message[i];
        vidptr[index + 1] = existing_color | 0x0F; 
        i++;
        index += 2;
    }
}

/* --- Keyboard Driver --- */
void check_keyboard() {
    /* Check status port 0x64. Bit 0 (0x01) must be 1 if a key is ready */
    unsigned char status = inb(0x64);
    if (status & 0x01) {
        unsigned char scancode = inb(0x60);
        
        /* Scan codes: 0x1E is 'A', 0x30 is 'B', 0x20 is 'D' for Dean! */
        if (scancode == 0x1E) {
            print_at("USER PRESSED: A", 0, 0);
        } else if (scancode == 0x30) {
            print_at("USER PRESSED: B", 0, 0);
        } else if (scancode == 0x20) {
            print_at("USER PRESSED: D", 0, 0);
        }
    }
}

/* --- Entry Point --- */
void kernel_main() {
    /* 1. Rainbow Background */
    for (int row = 0; row < 25; row++) {
        unsigned char color = (unsigned char)((row % 16) << 4);
        for (int col = 0; col < 80; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';
            vidptr[index + 1] = color; 
        }
    }

    /* 2. Brand your Console */
    print_at("DCON v1.0 (Dean Console)", 12, 28);
    print_at("LISTENING FOR USER...", 14, 30);

    /* 3. Loop */
    while(1) {
        check_keyboard();
        /* Small delay trick so the CPU doesn't jam the keyboard buffer */
        for(int d = 0; d < 1000; d++) { __asm__("nop"); }
    }
}

