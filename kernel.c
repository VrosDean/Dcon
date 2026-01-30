/* --- 1. HARDWARE & GLOBAL VARIABLES --- */
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

volatile char* vidptr = (volatile char*)0xb8000;
int cursor = 0;
char cmd_buffer[81]; 
int cmd_idx = 0;
int is_green_screen = 0; /* THE FLAG: Must be at the top! */

unsigned char kbd_map[] = { 
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=','\b','\t',
    'Q','W','E','R','T','Y','U','I','O','P','[',']','\n', 0,
    'A','S','D','F','G','H','J','K','L',';','\'','`', 0,
    '\\','Z','X','C','V','B','N','M',',','.','/', 0,'*',0,' ' 
};

/* --- 2. SCREEN HELPERS --- */
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
    /* Rainbow up to row 23 */
    for (int row = 0; row < 24; row++) {
        unsigned char bg = colors[row % 13] << 4;
        for (int col = 0; col < 80; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';
            vidptr[index + 1] = bg;
        }
    }
    /* Command Box (Black) */
    for (int row = 14; row < 24; row++) {
        for (int col = 4; col < 76; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';
            vidptr[index + 1] = 0x00; 
        }
    }
    /* THE VERY LAST LINE (Black) */
    for (int col = 0; col < 80; col++) {
        int index = (24 * 80 + col) * 2;
        vidptr[index] = ' ';
        vidptr[index + 2] = 0x00;
    }
    print_at("DCON v1.0", 5, 35);
    print_at("DEAN CONSOLE", 6, 34);
    is_green_screen = 0;
}

/* --- 3. COMMAND BRAIN --- */
void run_command() {
    /* Check letters 1 by 1: D-E-A-N */
    if (cmd_buffer[0] == 'D' && cmd_buffer[1] == 'E' && 
        cmd_buffer[2] == 'A' && cmd_buffer[3] == 'N') {
        for(int i = 0; i < 4000; i += 2) {
            vidptr[i] = ' ';
            vidptr[i+1] = 0x2F; /* Green Screen color */
        }
        print_at("DEAN GREEN SCREEN ACTIVATED. PRESS R TO RESET.", 12, 18);
        is_green_screen = 1;
    }
    /* Reset buffer */
    for(int i=0; i<80; i++) cmd_buffer[i] = 0;
    cmd_idx = 0;
}

/* --- 4. KEYBOARD DRIVER --- */
void check_keyboard() {
    if (inb(0x64) & 0x01) {
        unsigned char scancode = inb(0x60);
        
        if (scancode == 0x0E) { /* BACKSPACE */
            if (cmd_idx > 0) {
                cmd_idx--; cursor--;
                vidptr[cursor * 2] = ' ';
            }
        } 
        else if (scancode == 0x1C) { /* ENTER */
            run_command();
            void run_command() {
    /* 1. Check for "DEAN" */
    if (cmd_buffer[0] == 'D' && cmd_buffer[1] == 'E' && 
        cmd_buffer[2] == 'A' && cmd_buffer[3] == 'N') {
        for(int i = 0; i < 4000; i += 2) {
            vidptr[i] = ' ';
            vidptr[i+1] = 0x2F; 
        }
        print_at("DEAN GREEN SCREEN ACTIVATED. PRESS R TO RESET.", 12, 18);
        is_green_screen = 1;
    }
    /* 2. NEW: Check for "HELP" */
    else if (cmd_buffer[0] == 'H' && cmd_buffer[1] == 'E' && 
             cmd_buffer[2] == 'L' && cmd_buffer[3] == 'P') {
        print_at("DCON COMMANDS:        ", 15, 5);
        print_at("- DEAN : SECRET SCREEN", 16, 5);
        print_at("- HELP : SHOW THIS    ", 17, 5);
        print_at("- R    : RESET RAINBOW", 18, 5);
        print_at("- CLS  : CLEAR BOX    ", 19, 5);
    }
    /* 3. Check for "CLS" */
    else if (cmd_buffer[0] == 'C' && cmd_buffer[1] == 'L' && cmd_buffer[2] == 'S') {
        draw_ui();
    }

    /* Reset buffer */
    for(int i=0; i<80; i++) cmd_buffer[i] = 0;
    cmd_idx = 0;
}

        }
        else if (scancode == 0x13 && is_green_screen == 1) { /* SMART R */
            draw_ui();
            cursor = 14 * 80 + 5;
        }
        else if (scancode < 128) { /* LETTERS */
            char letter = kbd_map[scancode];
            if (letter != 0 && cmd_idx < 79) {
                cmd_buffer[cmd_idx++] = letter;
                vidptr[cursor * 2] = letter;
                /* Use green background if in green screen mode */
                vidptr[cursor * 2 + 1] = (is_green_screen) ? 0x2F : 0x0F;
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

