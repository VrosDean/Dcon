/* 1. Hardware & Variables */
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

volatile char* vidptr = (volatile char*)0xb8000;
int cursor = 0;
char cmd_buffer[80]; // Holds your typed command
int cmd_idx = 0;

unsigned char kbd_map[] = { 
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=','\b','\t',
    'Q','W','E','R','T','Y','U','I','O','P','[',']','\n', 0,
    'A','S','D','F','G','H','J','K','L',';','\'','`', 0,
    '\\','Z','X','C','V','B','N','M',',','.','/', 0,'*',0,' ' 
};

/* 2. Screen Helpers */
void print_at(const char* message, int row, int col, unsigned char color) {
    int index = (row * 80 + col) * 2;
    for (int i = 0; message[i] != '\0'; i++) {
        vidptr[index] = message[i];
        vidptr[index + 1] = color; 
        index += 2;
    }
}

void draw_ui() {
    // 4 Black Lines at the Top
    for (int i = 0; i < 80 * 4 * 2; i += 2) {
        vidptr[i] = ' ';
        vidptr[i+1] = 0x00; 
    }
    // Rainbow for the rest
    for (int row = 4; row < 25; row++) {
        unsigned char color = (unsigned char)((row % 16) << 4);
        for (int col = 0; col < 80; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';
            vidptr[index + 1] = color;
        }
    }
    print_at("DCON v1.0", 5, 35, 0x0F);
    // Underline for Command Area
    print_at("________________________________________________________________________________", 6, 0, 0x0F);
}

/* 3. The "Brain" (Command Processor) */
void run_command() {
    // Check if user typed "CLEAR"
    if (cmd_buffer[0] == 'C' && cmd_buffer[1] == 'L' && cmd_buffer[2] == 'E' && cmd_buffer[3] == 'A' && cmd_buffer[4] == 'R') {
        draw_ui();
    } 
    // Check if user typed "DEAN" (Blue Screen)
    else if (cmd_buffer[0] == 'D' && cmd_buffer[1] == 'E' && cmd_buffer[2] == 'A' && cmd_buffer[3] == 'N') {
        for(int i=0; i<4000; i+=2) { vidptr[i]=' '; vidptr[i+1]=0x1F; }
        print_at("DCON: DEAN MODE ACTIVATED", 10, 25, 0x1F);
    }
    
    // Clear buffer for next command
    for(int i=0; i<80; i++) cmd_buffer[i] = 0;
    cmd_idx = 0;
}

/* 4. Keyboard Logic */
void check_keyboard() {
    if (inb(0x64) & 0x01) {
        unsigned char scancode = inb(0x60);
        if (scancode == 0x0E && cmd_idx > 0) { // Backspace
            cmd_idx--;
            cursor--;
            vidptr[cursor * 2] = ' ';
        } 
        else if (scancode == 0x1C) { // ENTER: Run Command!
            run_command();
            cursor = ((cursor / 80) + 1) * 80;
        }
        else if (scancode < 128) {
            char letter = kbd_map[scancode];
            if (letter != 0 && cmd_idx < 79) {
                cmd_buffer[cmd_idx++] = letter;
                vidptr[cursor * 2] = letter;
                vidptr[cursor * 2 + 1] = (vidptr[cursor * 2 + 1] & 0xF0) | 0x0F;
                cursor++;
            }
        }
    }
}

void kernel_main() {
    draw_ui();
    cursor = 8 * 80; // Start typing below the underline
    while(1) { check_keyboard(); }
}

