/* 1. HARDWARE COMMUNICATION */
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void outb(unsigned short port, unsigned char data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

/* 2. GLOBAL VARIABLES (Must be here so functions below can see them!) */
volatile char* vidptr = (volatile char*)0xb8000;
int cursor = 0;
char cmd_buffer[80];
int cmd_idx = 0;
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
        /* The & 0xF0 trick: keeps the rainbow, makes text white */
        vidptr[index + 1] = (vidptr[index + 1] & 0xF0) | 0x0F; 
        index += 2;
    }
}

void draw_ui() {
    unsigned char colors[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF};
    int color_count = 13;

    /* Rainbow Background */
    for (int row = 0; row < 25; row++) {
        unsigned char bg = colors[row % color_count] << 4;
        for (int col = 0; col < 80; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';
            vidptr[index + 1] = bg;
        }
    }

    /* Title (No Box!) */
    print_at("DCON v1.0", 5, 35);
    print_at("DEAN CONSOLE", 6, 34);

    /* Command Box */
    for (int row = 14; row < 24; row++) {
        for (int col = 4; col < 76; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';
            vidptr[index + 1] = 0x00; /* Solid Black */
        }
    }
}

/* 4. KEYBOARD & MAIN */
void check_keyboard() {
    if (inb(0x64) & 0x01) {
        unsigned char scancode = inb(0x60);
        if (scancode == 0x0E && cmd_idx > 0) {
            cmd_idx--; cursor--; vidptr[cursor * 2] = ' ';
        } else if (scancode == 0x1C) {
            draw_ui();
            cursor = 14 * 80 + 5;
            cmd_idx = 0;
        } else if (scancode < 128) {
            char letter = kbd_map[scancode];
            if (letter != 0) {
                vidptr[cursor * 2] = letter;
                vidptr[cursor * 2 + 1] = 0x0F;
                cursor++;
            }
        }
    }
}

void kernel_main() {
    draw_ui();
    cursor = 14 * 80 + 5;
    while(1) { check_keyboard(); }
}

