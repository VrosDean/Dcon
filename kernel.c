/* --- 1. Hardware Communication (Port I/O) --- */
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

/* --- 2. Variables & Keyboard Map --- */
volatile char* vidptr = (volatile char*)0xb8000;
int cursor = 0;
char dean_check[5] = "    "; // Buffer to watch for "DEAN"

unsigned char kbd_map[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', 0,
    '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0, '*', 0, ' '
};

/* --- 3. Screen Helpers --- */
void print_at(const char* message, int row, int col) {
    int index = (row * 80 + col) * 2;
    int i = 0;
    while (message[i] != '\0') {
        vidptr[index] = message[i];
        // Keep the rainbow color, but set text to white
        vidptr[index + 1] = (vidptr[index + 1] & 0xF0) | 0x0F; 
        i++;
        index += 2;
    }
}

/* --- 4. The Keyboard Driver & Secret Command --- */
void check_keyboard() {
    if (inb(0x64) & 0x01) {
        unsigned char scancode = inb(0x60);

        if (scancode == 0x0E) { /* Backspace */
            if (cursor > 0) {
                cursor--;
                vidptr[cursor * 2] = ' '; // Overwrite letter with space
                // Background color is left alone!
            }
        } 
        else if (scancode == 0x1C) { /* Enter Key */
            cursor = ((cursor / 80) + 1) * 80;
        }
        else if (scancode < 128) { /* Key Pressed */
            char letter = kbd_map[scancode];
            if (letter != 0) {
                vidptr[cursor * 2] = letter;
                vidptr[cursor * 2 + 1] = (vidptr[cursor * 2 + 1] & 0xF0) | 0x0F;
                cursor++;

                /* --- DEAN SECRET CHECK --- */
                // Move letters through the "conveyor belt"
                dean_check[0] = dean_check[1];
                dean_check[1] = dean_check[2];
                dean_check[2] = dean_check[3];
                dean_check[3] = letter;

                if (dean_check[0] == 'D' && dean_check[1] == 'E' && 
                    dean_check[2] == 'A' && dean_check[3] == 'N') {
                    
                    // Activate Blue Screen
                    for(int i = 0; i < 80*25*2; i += 2) {
                        vidptr[i] = ' '; 
                        vidptr[i+1] = 0x1F; // Blue Background
                    }
                    print_at("DCON: DEAN BLUESCREEN ACTIVATED", 10, 25);
                    // Reset buffer
                    dean_check[0] = ' '; dean_check[1] = ' ';
                }
            }
        }
    }
}

/* --- 5. Main Entry Point --- */
void kernel_main() {
    /* Draw the Rainbow Stripes */
    for (int row = 0; row < 25; row++) {
        unsigned char color = (unsigned char)((row % 16) << 4);
        for (int col = 0; col < 80; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';
            vidptr[index + 1] = color; 
        }
    }

    print_at("DCON v1.0 (Dean Console)", 12, 28);
    cursor = 14 * 80; // Start typing below the title

    while(1) {
        check_keyboard();
        for(int d = 0; d < 1000; d++) { __asm__("nop"); }
    }
}

