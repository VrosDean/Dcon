/* --- 1. Hardware Communication --- */
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void outb(unsigned short port, unsigned char data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

/* --- 2. Variables --- */
volatile char* vidptr = (volatile char*)0xb8000;
int cursor = 0;
char dean_check[5] = "    "; 

unsigned char kbd_map[] = { 
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=','\b','\t',
    'Q','W','E','R','T','Y','U','I','O','P','[',']','\n', 0,
    'A','S','D','F','G','H','J','K','L',';','\'','`', 0,
    '\\','Z','X','C','V','B','N','M',',','.','/', 0,'*',0,' ' 
};

/* --- 3. Helpers --- */
void print_at(const char* message, int row, int col) {
    int index = (row * 80 + col) * 2;
    for (int i = 0; message[i] != '\0'; i++) {
        vidptr[index] = message[i];
        vidptr[index + 1] = (vidptr[index + 1] & 0xF0) | 0x0F; 
        index += 2;
    }
}

void draw_rainbow() {
    for (int row = 0; row < 25; row++) {
        unsigned char color = (unsigned char)((row % 16) << 4);
        for (int col = 0; col < 80; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';
            vidptr[index + 1] = color; 
        }
    }
}

/* --- 4. Keyboard & Recovery Logic --- */
void check_keyboard() {
    if (inb(0x64) & 0x01) {
        unsigned char scancode = inb(0x60);
        
        if (scancode == 0x0E && cursor > 0) { // Backspace
            cursor--;
            vidptr[cursor * 2] = ' ';
        } 
        else if (scancode == 0x1C) { // Enter
            cursor = ((cursor / 80) + 1) * 80;
        } 
        else if (scancode == 0x13) { // 'R' Key (Recover)
            draw_rainbow();
            print_at("DCON RECOVERED", 0, 0);
            
            // 3-Second Wait Loop (approximate)
            for(volatile int wait = 0; wait < 50000000; wait++); 
            
            // Clean up the recovery message
            print_at("              ", 0, 0); 
            cursor = 14 * 80;
        }
        else if (scancode < 128) {
            char letter = kbd_map[scancode];
            if (letter) {
                vidptr[cursor * 2] = letter;
                vidptr[cursor * 2 + 1] = (vidptr[cursor * 2 + 1] & 0xF0) | 0x0F;
                cursor++;

                dean_check[0] = dean_check[1];
                dean_check[1] = dean_check[2];
                dean_check[2] = dean_check[3];
                dean_check[3] = letter;

                if (dean_check[0] == 'D' && dean_check[1] == 'E' && 
                    dean_check[2] == 'A' && dean_check[3] == 'N') {
                    for(int i=0; i<4000; i+=2) {
                        vidptr[i] = ' ';
                        vidptr[i+1] = 0x1F; 
                    }
                    print_at("DCON: DEAN BLUESCREEN. PRESS 'R' TO RECOVER", 10, 15);
                }
            }
        }
    }
}

void kernel_main() {
    draw_rainbow();
    print_at("DCON v1.0 - DISK DRIVER LOADED", 12, 25);
    cursor = 14 * 80;
    while(1) { 
        check_keyboard(); 
    }
}

