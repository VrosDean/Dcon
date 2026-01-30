/* --- Hardware Ports --- */
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

/* --- Variables --- */
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

/* --- Screen Helpers --- */
void print_at(const char* message, int row, int col, unsigned char color) {
    int index = (row * 80 + col) * 2;
    for (int i = 0; message[i] != '\0'; i++) {
        vidptr[index] = message[i];
        vidptr[index + 1] = color; 
        index += 2;
    }
}

void draw_ui() {
    /* 1. Bright Rainbow (No Gray/Dark Gray) */
    for (int row = 0; row < 25; row++) {
        int color_idx = row % 16;
        if (color_idx == 7 || color_idx == 8) color_idx = 10; /* Change Gray to Green */
        unsigned char color = (unsigned char)(color_idx << 4) | 0x0F;
        
        for (int col = 0; col < 80; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';
            vidptr[index + 1] = color;
        }
    }

    /* 2. Classic DCON Text (Simple White Text) */
    print_at("DCON v1.0", 5, 35, 0x0F);
    print_at("DEAN CONSOLE", 6, 34, 0x0F);

    /* 3. The 10-Line Command Box at the bottom */
    for (int row = 14; row < 24; row++) {
        for (int col = 4; col < 76; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';
            vidptr[index + 1] = 0x00; /* Solid Black */
        }
    }
}

/* --- Logic & Main --- */
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
            if (letter != 0 && cmd_idx < 70) {
                cmd_buffer[cmd_idx++] = letter;
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

