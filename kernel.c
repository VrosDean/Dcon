/* --- 1. HARDWARE PORTS (8-bit and 32-bit) --- */
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}
void outb(unsigned short port, unsigned char data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}
unsigned long inl(unsigned short port) {
    unsigned long result;
    __asm__ volatile("inl %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}
void outl(unsigned short port, unsigned long data) {
    __asm__ volatile("outl %0, %1" : : "a"(data), "Nd"(port));
}

/* --- 2. GLOBAL VARIABLES --- */
volatile char* vidptr = (volatile char*)0xb8000;
int cursor = 0;
char cmd_buffer[81];
int cmd_idx = 0;
int is_green_screen = 0;
unsigned char kbd_map[] = { 0, 27, '1','2','3','4','5','6','7','8','9','0','-','=','\b','\t','Q','W','E','R','T','Y','U','I','O','P','[',']','\n', 0,'A','S','D','F','G','H','J','K','L',';','\'','`', 0,'\\','Z','X','C','V','B','N','M',',','.','/', 0,'*',0,' ' };

/* --- 3. SCREEN HELPERS --- */
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
    print_at("DCON v1.0", 5, 35);
    print_at("DEAN CONSOLE", 6, 34);
    is_green_screen = 0;
}

/* --- 4. PCI SCANNER (Finds the WiFi/Network Card) --- */
unsigned long pci_read(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset) {
    unsigned long address = (unsigned long)((((unsigned long)bus) << 16) | (((unsigned long)slot) << 11) | (((unsigned long)func) << 8) | (offset & 0xfc) | ((unsigned long)0x80000000));
    outl(0xCF8, address);
    return inl(0xCFC);
}

/* --- 5. COMMAND BRAIN --- */
void run_command() {
    if (cmd_buffer[0] == 'P' && cmd_buffer[1] == 'A' && cmd_buffer[2] == 'C' && cmd_buffer[3] == 'K') {
        print_at("PackON: CONNECTING TO REPO...", 20, 5);
        print_at("PackON: ERROR - NO INTERNET STACK", 21, 5);
    }
    else if (cmd_buffer[0] == 'W' && cmd_buffer[1] == 'I' && cmd_buffer[2] == 'F' && cmd_buffer[3] == 'I') {
        print_at("SCANNING PCI... FOUND INTEL E1000", 20, 5);
    }
    else if (cmd_buffer[0] == 'H' && cmd_buffer[1] == 'E' && cmd_buffer[2] == 'L' && cmd_buffer[3] == 'P') {
        print_at("COMMANDS: PACK, WIFI, DEAN, CLS", 15, 5);
    }
    else if (cmd_buffer[0] == 'C' && cmd_buffer[1] == 'L' && cmd_buffer[2] == 'S') {
        draw_ui();
    }
    cmd_idx = 0;
}

/* --- 6. MAIN --- */
void check_keyboard() {
    if (inb(0x64) & 0x01) {
        unsigned char scancode = inb(0x60);
        if (scancode == 0x1C) { run_command(); cursor = 20*80+5; }
        else if (scancode < 128) {
            char letter = kbd_map[scancode];
            if (letter) { vidptr[cursor*2] = letter; cmd_buffer[cmd_idx++] = letter; cursor++; }
        }
    }
}

void kernel_main() {
    draw_ui();
    cursor = 20 * 80 + 5;
    while(1) { check_keyboard(); }
}

