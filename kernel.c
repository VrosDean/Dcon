/* --- Hardware Communication (Port I/O) --- */
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

/* --- Global Variables --- */
volatile char* vidptr = (volatile char*)0xb8000;
int cursor = 0;

/* Scancode to ASCII Map (Scan Code Set 1) */
unsigned char kbd_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', 0,
    '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0, '*', 0, ' '
};

/* --- Screen Functions --- */
void print_at(const char* message, int row, int col) {
    int index = (row * 80 + col) * 2;
    int i = 0;
    while (message[i] != '\0') {
        vidptr[index] = message[i];
        vidptr[index + 1] = 0x0F; // White text on whatever background is there
        i++;
        index += 2;
    }
}

/* --- The Keyboard Driver --- */
void check_keyboard() {
    /* Check if a key is ready (Port 0x64, Bit 0) */
    if (inb(0x64) & 0x01) {
        unsigned char scancode = inb(0x60);

        /* Handle Backspace (Scancode 0x0E) */
        if (scancode == 0x0E) {
            if (cursor > 0) {
                cursor--;
                vidptr[cursor * 2] = ' '; // Replace with space
            }
        } 
        /* Handle Normal Keys (Presses are < 128) */
        else if (scancode < 128) {
            char letter = kbd_map[scancode];
            if (letter != 0) {
                vidptr[cursor * 2] = letter;
                vidptr[cursor * 2 + 1] = 0x0F; // White text
                cursor++;
            }
        }
    }
}

/* --- Main Entry Point --- */
void kernel_main() {
    /* 1. Draw the Rainbow Background */
    for (int row = 0; row < 25; row++) {
        unsigned char color = (unsigned char)((row % 16) << 4);
        for (int col = 0; col < 80; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';      // Clear old text
            vidptr[index + 1] = color; // Paint row color
        }
    }

    /* 2. DCON Branding */
    print_at("DCON v1.0 - DEAN CONSOLE", 12, 28);
    print_at("TYPE ANYTHING:", 14, 33);
    
    /* Set cursor to start typing on line 16 */
    cursor = 16 * 80; 

    /* 3. The Infinite Loop */
    while(1) {
        check_keyboard();
        /* Tiny delay so the emulator doesn't choke */
        for(int d = 0; d < 1000; d++) { __asm__("nop"); }
    }
}

