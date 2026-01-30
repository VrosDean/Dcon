/* 1. HARDWARE PORTS */
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

/* 2. VARIABLES & MAP */
volatile char* vidptr = (volatile char*)0xb8000;
int cursor = 0;
unsigned char kbd_map[] = { 
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=','\b','\t',
    'Q','W','E','R','T','Y','U','I','O','P','[',']','\n', 0,
    'A','S','D','F','G','H','J','K','L',';','\'','`', 0,
    '\\','Z','X','C','V','B','N','M',',','.','/', 0,'*',0,' ' 
};

/* 3. SCREEN HELPERS */
void print_at(const char* message, int row, int col) {
    int index = (row * 80 + col) * 2;
    for (int i = 0; message[i] != '\0'; i++) {
        vidptr[index] = message[i];
        vidptr[index + 1] = (vidptr[index + 1] & 0xF0) | 0x0F; 
        index += 2;
    }
}

void draw_rainbow() {
    for (int row = 0; row < 25; row++) {
        unsigned char color = (unsigned char)((row % 16) << 4);
        for (int col = 0; col < 80; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';
            vidptr[index + 1] = color; 
        }
    }
}

/* 4. KEYBOARD DRIVER */
void check_keyboard() {
    if (inb(0x64) & 0x01) {
        unsigned char scancode = inb(0x60);
        if (scancode < 128) {
            char letter = kbd_map[scancode];
            if (letter != 0) {
                vidptr[cursor * 2] = letter;
                vidptr[cursor * 2 + 1] = (vidptr[cursor * 2 + 1] & 0xF0) | 0x0F;
                cursor++;
            }
        }
    }
}

/* 5. MAIN ENTRY POINT */
void kernel_main() {
    draw_rainbow();
    print_at("DCON v1.0 - DEAN CONSOLE", 12, 28);
    cursor = 14 * 80; /* Start typing below the title */

    while(1) {
        check_keyboard();
        for(volatile int d=0; d<1000; d++); 
    }
}

