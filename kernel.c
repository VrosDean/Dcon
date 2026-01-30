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
char cmd_buffer[81]; 
int cmd_idx = 0;
int is_green_screen = 0;

unsigned char kbd_map[] = { 
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=','\b','\t',
    'Q','W','E','R','T','Y','U','I','O','P','[',']','\n', 0,
    'A','S','D','F','G','H','J','K','L',';','\'','`', 0,
    '\\','Z','X','C','V','B','N','M',',','.','/', 0,'*',0,' ' 
};

/* --- 3. DISK DRIVERS (ATA PIO) --- */
void read_disk(int LBA, unsigned short* target) {
    outb(0x1F6, (0xE0 | ((LBA >> 24) & 0x0F)));
    outb(0x1F2, 1);
    outb(0x1F3, (unsigned char)LBA);
    outb(0x1F4, (unsigned char)(LBA >> 8));
    outb(0x1F5, (unsigned char)(LBA >> 16));
    outb(0x1F7, 0x20);
    while (inb(0x1F7) & 0x80);
    while (!(inb(0x1F7) & 0x08));
    for (int i = 0; i < 256; i++) {
        unsigned short tmp = inb(0x1F0);
        tmp |= (inb(0x1F0) << 8);
        target[i] = tmp;
    }
}

void write_disk(int LBA, unsigned short* source) {
    outb(0x1F6, (0xE0 | ((LBA >> 24) & 0x0F)));
    outb(0x1F2, 1);
    outb(0x1F3, (unsigned char)LBA);
    outb(0x1F4, (unsigned char)(LBA >> 8));
    outb(0x1F5, (unsigned char)(LBA >> 16));
    outb(0x1F7, 0x30);
    while (inb(0x1F7) & 0x80);
    while (!(inb(0x1F7) & 0x08));
    for (int i = 0; i < 256; i++) {
        outb(0x1F0, (unsigned char)(source[i] & 0xFF));
        outb(0x1F0, (unsigned char)((source[i] >> 8) & 0xFF));
    }
    outb(0x1F7, 0xE7);
}

/* --- 4. SCREEN & CLOCK HELPERS --- */
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

/* --- 5. COMMAND BRAIN --- */
void run_command() {
    if (cmd_idx == 6 && cmd_buffer[0] == 'P' && cmd_buffer[1] == 'A' && cmd_buffer[2] == 'C' && cmd_buffer[3] == 'K' && cmd_buffer[4] == 'O' && cmd_buffer[5] == 'N') {
        print_at("PackON: INSTALLING DEAN_APP...", 15, 5);
        unsigned short p_data[256];
        read_disk(100, p_data);
        write_disk(200, p_data);
        print_at("PackON: SUCCESS! SECTOR 200 UPDATED.", 16, 5);
    }
    else if (cmd_idx == 3 && cmd_buffer[0] == 'C' && cmd_buffer[1] == 'L' && cmd_buffer[2] == 'S') {
        draw_ui();
        cursor = 14 * 80 + 5;
    }
    else if (cmd_idx == 4 && cmd_buffer[0] == 'R' && cmd_buffer[1] == 'E' && cmd_buffer[2] == 'B' && cmd_buffer[3] == 'O') {
        outb(0x64, 0xFE);
    }
    
    for(int i=0; i<80; i++) cmd_buffer[i] = 0;
    cmd_idx = 0;
}

/* --- 6. KEYBOARD & MAIN --- */
void check_keyboard() {
    if (inb(0x64) & 0x01) {
        unsigned char scancode = inb(0x60);
        if (scancode == 0x0E && cmd_idx > 0) { cmd_idx--; cursor--; vidptr[cursor * 2] = ' '; }
        else if (scancode == 0x1C) { run_command(); cursor = 21 * 80 + 5; }
        else if (scancode < 0x80) {
            char letter = kbd_map[scancode];
            if (letter) { 
                vidptr[cursor * 2] = letter; 
                vidptr[cursor * 2 + 1] = 0x0F; 
                cmd_buffer[cmd_idx++] = letter; 
                cursor++; 
            }
        }
    }
}

void kernel_main() {
    draw_ui();
    cursor = 14 * 80 + 5;
    while(1) {
        check_keyboard();
        draw_clock();
        for(volatile int d=0; d<1000; d++);
    }
}

