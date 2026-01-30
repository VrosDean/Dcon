/* --- 1. Hardware Communication --- */
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void outb(unsigned short port, unsigned char data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

/* --- 2. Global Variables --- */
volatile char* vidptr = (volatile char*)0xb8000;
int cursor = 0;
char cmd_buffer[81]; 
int cmd_idx = 0;
int is_green_screen = 0;

unsigned char kbd_map[] = { 
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=','\b','\t',
    'Q','W','E','R','T','Y','U','I','O','P','[',']','\n', 0,
    'A','S','D','F','G','H','J','K','L',';','\'','`', 0,
    '\\','Z','X','C','V','B','N','M',',','.','/', 0,'*',0,' ' 
};

/* --- 3. Screen Helpers --- */
void print_at(const char* message, int row, int col) {
    int index = (row * 80 + col) * 2;
    for (int i = 0; message[i] != '\0'; i++) {
        vidptr[index] = message[i];
        /* Keep background, make text White */
        vidptr[index + 1] = (vidptr[index + 1] & 0xF0) | 0x0F; 
        index += 2;
    }
}

void draw_ui() {
    unsigned char colors[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF};
    for (int row = 0; row < 24; row++) {
        unsigned char bg = colors[row % 13] << 4;
        for (int col = 0; col < 80; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';
            vidptr[index + 1] = bg;
        }
    }
    /* Command Box (Rows 14-23) */
    for (int row = 14; row < 24; row++) {
        for (int col = 4; col < 76; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';
            vidptr[index + 1] = 0x00; 
        }
    }
    /* Bottom Line */
    for (int col = 0; col < 80; col++) {
        int index = (24 * 80 + col) * 2;
        vidptr[index] = ' ';
        vidptr[index + 1] = 0x00;
    }
    print_at("DCON v1.1", 5, 35);
    print_at("DEAN CONSOLE", 6, 34);
    is_green_screen = 0;
}

/* --- 4. Clock --- */
void draw_clock() {
    outb(0x70, 0x04); unsigned char h = inb(0x71);
    outb(0x70, 0x02); unsigned char m = inb(0x71);
    h = (h & 0x0F) + ((h / 16) * 10);
    m = (m & 0x0F) + ((m / 16) * 10);
    int pos = (0 * 80 + 72) * 2;
    vidptr[pos] = (h / 10) + '0';
    vidptr[pos + 2] = (h % 10) + '0';
    vidptr[pos + 4] = ':';
    vidptr[pos + 6] = (m / 10) + '0';
    vidptr[pos + 8] = (m % 10) + '0';
    for(int i=1; i<10; i+=2) vidptr[pos + i] = (vidptr[pos + i] & 0xF0) | 0x0F;
}

/* --- 5. Command Logic --- */
void run_command() {
    if (cmd_buffer[0] == 'D' && cmd_buffer[1] == 'E' && cmd_buffer[2] == 'A' && cmd_buffer[3] == 'N') {
        for(int i = 0; i < 4000; i += 2) { vidptr[i] = ' '; vidptr[i+1] = 0x2F; }
        print_at("GREEN MODE ON. PRESS R TO RESET.", 12, 18);
        is_green_screen = 1;
    }
    else if (cmd_buffer[0] == 'C' && cmd_buffer[1] == 'L' && cmd_buffer[2] == 'S') {
        draw_ui();
        cursor = 14 * 80 + 5;
    }
    for(int i=0; i<80; i++) cmd_buffer[i] = 0;
    cmd_idx = 0;
}

/* --- 6. Keyboard Driver --- */
void check_keyboard() {
    if (inb(0x64) & 0x01) {
        unsigned char scancode = inb(0x60);
        
        if (scancode == 0x0E) { /* BACKSPACE - STOP THE CIRCLES */
            if (cmd_idx > 0) {
                cmd_idx--; cursor--;
                vidptr[cursor * 2] = ' ';
                vidptr[cursor * 2 + 1] = 0x00;
            }
        } 
        else if (scancode == 0x1C) { /* ENTER */
            run_command();
            if (!is_green_screen) cursor = 14 * 80 + 5;
        }
        else if (scancode == 0x13 && is_green_screen == 1) { /* R - RESET */
            draw_ui();
            cursor = 14 * 80 + 5;
        }
        else if (scancode < 0x80) { /* LETTERS - FORCE WHITE */
            char letter = kbd_map[scancode];
            if (letter != 0 && cmd_idx < 79) {
                cmd_buffer[cmd_idx++] = letter;
                vidptr[cursor * 2] = letter;
                vidptr[cursor * 2 + 1] = 0x0F; /* White on Black */
                cursor++;
            }
        }
    }
}

/* --- 7. Main Entry --- */
void kernel_main() {
    draw_ui();
    cursor = 14 * 80 + 5;
    while(1) { 
        check_keyboard(); 
        draw_clock();
        for(volatile int d=0; d<1000; d++); 
    }
}

