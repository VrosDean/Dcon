/* 1. HARDWARE PORTS */
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}
void outb(unsigned short port, unsigned char data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

/* 2. GLOBAL VARIABLES (Must be ABOVE everything else) */
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

/* 4. COMMAND BRAIN (Now with Lowercase Support) */
void run_command() {
    /* DEAN / dean check */
    if (cmd_idx == 4 && 
       (cmd_buffer[0] == 'D' || cmd_buffer[0] == 'd') && 
       (cmd_buffer[1] == 'E' || cmd_buffer[1] == 'e') && 
       (cmd_buffer[2] == 'A' || cmd_buffer[2] == 'a') && 
       (cmd_buffer[3] == 'N' || cmd_buffer[3] == 'n')) {
        for(int i = 0; i < 4000; i += 2) { vidptr[i] = ' '; vidptr[i+1] = 0x2F; }
        print_at("DEAN GREEN SCREEN ACTIVATED", 12, 25);
        is_green_screen = 1;
    }
    /* CLS / cls check */
    else if (cmd_idx == 3 && 
            (cmd_buffer[0] == 'C' || cmd_buffer[0] == 'c') && 
            (cmd_buffer[1] == 'L' || cmd_buffer[1] == 'l') && 
            (cmd_buffer[2] == 'S' || cmd_buffer[2] == 's')) {
        draw_ui();
        cursor = 14 * 80 + 5;
    }

    for(int i=0; i<80; i++) cmd_buffer[i] = 0;
    cmd_idx = 0;
}

/* 5. KEYBOARD & MAIN */
void check_keyboard() {
    if (inb(0x64) & 0x01) {
        unsigned char scancode = inb(0x60);
        if (scancode == 0x1C) { run_command(); if(!is_green_screen) cursor = 21 * 80 + 5; }
        else if (scancode < 128) {
            char letter = kbd_map[scancode];
            if (letter) { vidptr[cursor*2] = letter; cmd_buffer[cmd_idx++] = letter; cursor++; }
        }
    }
}

void kernel_main() {
    draw_ui();
    cursor = 14 * 80 + 5;
    while(1) { check_keyboard(); }
}

