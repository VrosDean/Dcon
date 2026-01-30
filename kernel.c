/* --- 1. HARDWARE COMMUNICATION --- */
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void outb(unsigned short port, unsigned char data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

/* --- 2. GLOBAL VARIABLES --- */
volatile char* vidptr = (volatile char*)0xb8000;
int cursor = 0;
char cmd_buffer[81]; /* Fixed: Added [81] to make it an array */
int cmd_idx = 0;
int is_green_screen = 0;

unsigned char kbd_map[] = { 
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=','\b','\t',
    'Q','W','E','R','T','Y','U','I','O','P','[',']','\n', 0,
    'A','S','D','F','G','H','J','K','L',';','\'','`', 0,
    '\\','Z','X','C','V','B','N','M',',','.','/', 0,'*',0,' ' 
};

/* --- 3. HELPER FUNCTIONS --- */
void print_at(const char* message, int row, int col) {
    int index = (row * 80 + col) * 2;
    for (int i = 0; message[i] != '\0'; i++) {
        vidptr[index] = message[i];
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
    for (int row = 14; row < 24; row++) {
        for (int col = 4; col < 76; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';
            vidptr[index + 1] = 0x00; 
        }
    }
    print_at("DCON v1.1", 5, 35);
    is_green_screen = 0;
}

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
    for(int i=1; i<10; i+=2) vidptr[pos + i] = 0x0F;
}

/* --- 4. COMMAND BRAIN --- */
void run_command() {
    if (cmd_buffer[0] == 'P' && cmd_buffer[1] == 'A') {
        print_at("PackON: INSTALLING...", 21, 5);
    } 
    else if (cmd_buffer[0] == 'W' && cmd_buffer[1] == 'I') {
        print_at("WIFI: E1000 LINK ACTIVE", 21, 5);
    }
    else if (cmd_buffer[0] == 'C' && cmd_buffer[1] == 'L') {
        draw_ui();
    }
    for(int i=0; i<80; i++) cmd_buffer[i] = 0;
    cmd_idx = 0;
}

/* --- 5. KEYBOARD DRIVER --- */
void check_keyboard() {
    if (inb(0x64) & 0x01) {
        unsigned char scancode = inb(0x60);
        if (scancode == 0x1C) {
            run_command();
            cursor = 21 * 80 + 5;
        } else if (scancode < 128) {
            char letter = kbd_map[scancode];
            if (letter) {
                vidptr[cursor * 2] = letter;
                cmd_buffer[cmd_idx++] = letter;
                cursor++;
            }
        }
    }
}

/* --- 6. MAIN ENTRY --- */
void kernel_main() {
    draw_ui();
    cursor = 21 * 80 + 5;
    while(1) {
        check_keyboard();
        draw_clock();
        for(volatile int d=0; d<1000; d++);
    }
}

