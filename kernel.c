/* 1. Hardware Ports */
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}
void outb(unsigned short port, unsigned char data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

volatile char* vidptr = (volatile char*)0xb8000;
int cursor = 0;
char cmd_buffer[81];
int cmd_idx = 0;
int is_green_screen = 0;

/* 2. Fixed Clock (BCD to Decimal + Fixed Position) */
void draw_clock() {
    outb(0x70, 0x04); unsigned char h = inb(0x71);
    outb(0x70, 0x02); unsigned char m = inb(0x71);
    
    // Convert hardware BCD to normal numbers
    h = (h & 0x0F) + ((h / 16) * 10);
    m = (m & 0x0F) + ((m / 16) * 10);

    // Print at Row 0, Column 72 (Top Right)
    // We force the color to 0x0F (White on Black) so it's always visible
    int pos = (0 * 80 + 72) * 2;
    vidptr[pos] = (h / 10) + '0';
    vidptr[pos + 2] = (h % 10) + '0';
    vidptr[pos + 4] = ':';
    vidptr[pos + 6] = (m / 10) + '0';
    vidptr[pos + 8] = (m % 10) + '0';
    vidptr[pos + 1] = 0x0F; vidptr[pos + 3] = 0x0F; vidptr[pos + 5] = 0x0F; vidptr[pos + 7] = 0x0F; vidptr[pos + 9] = 0x0F;
}

/* 3. Screen UI */
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
    // Command Box
    for (int row = 14; row < 24; row++) {
        for (int col = 4; col < 76; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';
            vidptr[index + 1] = 0x00; 
        }
    }
    is_green_screen = 0;
}

/* 4. Command Logic */
void run_command() {
    if (cmd_buffer[0] == 'W' && cmd_buffer[1] == 'I') {
        print_at("WIFI: INTEL E1000 FOUND. LINK ACTIVE.", 21, 5);
    } 
    else if (cmd_buffer[0] == 'D' && cmd_buffer[1] == 'E') {
        for(int i = 0; i < 4000; i += 2) { vidptr[i] = ' '; vidptr[i+1] = 0x2F; }
        is_green_screen = 1;
    }
    else if (cmd_buffer[0] == 'C' && cmd_buffer[1] == 'L') {
        draw_ui();
    }
    cmd_idx = 0;
}

void kernel_main() {
    draw_ui();
    cursor = 14 * 80 + 5;
    while(1) {
        check_keyboard(); // Add your keyboard check here
        draw_clock();     // This MUST be in the loop
        for(volatile int d=0; d<1000; d++);
    }
}

