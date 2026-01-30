/* 1. Hardware functions FIRST */
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

/* 2. Variables SECOND */
volatile char* vidptr = (volatile char*)0xb8000;
int cursor = 0;
unsigned char kbd_map[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', 0,
    '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0, '*', 0, ' '
};

/* 3. Helper functions THIRD */
void print_at(const char* message, int row, int col) {
    int index = (row * 80 + col) * 2;
    int i = 0;
    while (message[i] != '\0') {
        vidptr[index] = message[i];
        vidptr[index + 1] = (vidptr[index + 1] & 0xF0) | 0x0F; 
        i++;
        index += 2;
    }
}

/* 4. Keyboard driver FOURTH */
void check_keyboard() {
    if (inb(0x64) & 0x01) {
        unsigned char scancode = inb(0x60);
        if (scancode == 0x0E) {
            if (cursor > 0) {
                cursor--;
                vidptr[cursor * 2] = ' ';
            }
        } else if (scancode == 0x1C) {
            cursor = ((cursor / 80) + 1) * 80;
        } else if (scancode < 128) {
            char letter = kbd_map[scancode];
            if (letter != 0) {
                vidptr[cursor * 2] = letter;
                vidptr[cursor * 2 + 1] = (vidptr[cursor * 2 + 1] & 0xF0) | 0x0F;
                cursor++;
                if (letter == '!') {
                    for(int i = 0; i < 80*25*2; i += 2) {
                        vidptr[i] = ' '; 
                        vidptr[i+1] = 0x1F; 
                    }
                    print_at("DCON: DEAN BLUESCREEN ACTIVATED", 10, 25);
                }
            }
        }
    }
}

/* 5. kernel_main LAST */
void kernel_main() {
    for (int row = 0; row < 25; row++) {
        unsigned char color = (unsigned char)((row % 16) << 4);
        for (int col = 0; col < 80; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';
            vidptr[index + 1] = color; 
        }
    }
    print_at("DCON v1.0", 12, 35);
    cursor = 14 * 80;
    while(1) {
        check_keyboard();
        for(int d = 0; d < 1000; d++) { __asm__("nop"); }
    }
}

