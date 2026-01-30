/* 1. HARDWARE PORTS */
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}
void outb(unsigned short port, unsigned char data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

/* 2. VARIABLES */
volatile char* vidptr = (volatile char*)0xb8000;
int cursor = 0;
char cmd_buffer[81]; 
int cmd_idx = 0;
int is_green_screen = 0;
unsigned char kbd_map[] = { 0, 27, '1','2','3','4','5','6','7','8','9','0','-','=','\b','\t','Q','W','E','R','T','Y','U','I','O','P','[',']','\n', 0,'A','S','D','F','G','H','J','K','L',';','\'','`', 0,'\\','Z','X','C','V','B','N','M',',','.','/', 0,'*',0,' ' };

/* 3. SCREEN HELPERS */
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

/* 4. CLOCK (With YEAR) */
void draw_clock() {
    outb(0x70, 0x04); unsigned char h = inb(0x71);
    outb(0x70, 0x02); unsigned char m = inb(0x71);
    outb(0x70, 0x09); unsigned char y = inb(0x71);

    h = (h & 0x0F) + ((h / 16) * 10);
    m = (m & 0x0F) + ((m / 16) * 10);
    y = (y & 0x0F) + ((y / 16) * 10);

    int pos = (0 * 80 + 65) * 2;
    vidptr[pos] = (h / 10) + '0'; vidptr[pos+2] = (h % 10) + '0';
    vidptr[pos+4] = ':';
    vidptr[pos+6] = (m / 10) + '0'; vidptr[pos+8] = (m % 10) + '0';
    vidptr[pos+10] = ' ';
    vidptr[pos+12] = '2'; vidptr[pos+14] = '0';
    vidptr[pos+16] = (y / 10) + '0'; vidptr[pos+18] = (y % 10) + '0';
    for(int i=1; i<20; i+=2) vidptr[pos + i] = (vidptr[pos+i] & 0xF0) | 0x0F;
}

/* 5. COMMAND BRAIN */
void run_command() {
    // HELP or HLEP
    if ((cmd_buffer[0] == 'H' && cmd_buffer[1] == 'E' && cmd_buffer[2] == 'L' && cmd_buffer[3] == 'P') ||
        (cmd_buffer[0] == 'H' && cmd_buffer[1] == 'L' && cmd_buffer[2] == 'E' && cmd_buffer[3] == 'P')) {
        print_at("DCON COMMANDS: REBO, WI, DEAN, CLS", 15, 5);
    }
    else if (cmd_buffer[0] == 'W' && cmd_buffer[1] == 'I') {
        print_at("WIFI: INTEL E1000 LINK ACTIVE", 21, 5);
    } 
    else if (cmd_buffer[0] == 'R' && cmd_buffer[1] == 'E' && cmd_buffer[2] == 'B' && cmd_buffer[3] == 'O') {
        outb(0x64, 0xFE); 
    }
    else if (cmd_buffer[0] == 'C' && cmd_buffer[1] == 'L' && cmd_buffer[2] == 'S') {
        draw_ui();
        cursor = 14 * 80 + 5;
    }
    else if (cmd_buffer[0] == 'D' && cmd_buffer[1] == 'E' && cmd_buffer[2] == 'A' && cmd_buffer[3] == 'N') {
        for(int i = 0; i < 4000; i += 2) { vidptr[i] = ' '; vidptr[i+1] = 0x2F; }
        is_green_screen = 1;
    }

    for(int i=0; i<80; i++) cmd_buffer[i] = 0;
    cmd_idx = 0;
}

/* 6. KEYBOARD & MAIN */
void check_keyboard() {
    if (inb(0x64) & 0x01) {
        unsigned char scancode = inb(0x60);
        if (scancode == 0x0E && cmd_idx > 0) { cmd_idx--; cursor--; vidptr[cursor * 2] = ' '; }
        else if (scancode == 0x1C) { run_command(); cursor = 21 * 80 + 5; }
        else if (scancode == 0x13 && is_green_screen) { draw_ui(); cursor = 14 * 80 + 5; }
        else if (scancode < 0x80) {
            char letter = kbd_map[scancode];
            if (letter) { vidptr[cursor * 2] = letter; vidptr[cursor*2+1]=0x0F; cmd_buffer[cmd_idx++] = letter; cursor++; }
        }
    }
}

void kernel_main() {
    draw_ui();
    cursor = 14 * 80 + 5;
    while(1) { check_keyboard(); draw_clock(); for(volatile int d=0; d<1000; d++); }
}

