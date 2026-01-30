/* 1. Hardware & Variables (Same as before) */
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

volatile char* vidptr = (volatile char*)0xb8000;
int cursor = 0;
char cmd_buffer[80]; 
int cmd_idx = 0;

unsigned char kbd_map[] = { 
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=','\b','\t',
    'Q','W','E','R','T','Y','U','I','O','P','[',']','\n', 0,
    'A','S','D','F','G','H','J','K','L',';','\'','`', 0,
    '\\','Z','X','C','V', 'B','N','M',',','.','/', 0,'*',0,' ' 
};

/* 2. Screen Helpers (Updated print_at) */
// This version automatically uses the system's current default color
void print_at(const char* message, int row, int col, unsigned char color) {
    int index = (row * 80 + col) * 2;
    for (int i = 0; message[i] != '\0'; i++) {
        vidptr[index] = message[i];
        vidptr[index + 1] = color; 
        index += 2;
    }
}

void draw_ui() {
    // Start with the rainbow in the command area
    for (int row = 0; row < 25; row++) {
        unsigned char color = (unsigned char)((row % 16) << 4) | 0x0F; // Background + White Text
        for (int col = 0; col < 80; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';
            vidptr[index + 1] = color;
        }
    }

    // DCON Title with Colorful Background (Blue background, Yellow text)
    print_at("      DCON v1.0      ", 5, 29, 0x1E); // Background 0x10, Foreground 0x0E

    // 4 Black lines, 4 chars from the edge (start at row 7, which is 2 lines after the title)
    for (int row = 7; row < 11; row++) {
        for (int col = 4; col < 76; col++) { // 4 spaces in from left and right
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';
            vidptr[index + 1] = 0x00; // Black background/text
        }
    }
}

/* 3. The "Brain" (Command Processor) - Same as before */
void run_command() {
    // ... logic for CLEAR/DEAN commands ...
    // Clear buffer for next command
    for(int i=0; i<80; i++) cmd_buffer[i] = 0;
    cmd_idx = 0;
}

/* 4. Keyboard Logic (Same as before) */
void check_keyboard() {
    // ... logic for keys ...
}

/* 5. MAIN ENTRY POINT */
void kernel_main() {
    draw_ui();
    cursor = 11 * 80 + 5; // Start typing inside the black box
    while(1) { check_keyboard(); }
}

