/* --- 1. Hardware Communication (I/O) --- */
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void outb(unsigned short port, unsigned char data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

/* --- 2. Global Variables --- */
volatile char* vidptr = (volatile char*)0xb8000;
int cursor = 0;
char cmd_buffer[80]; 
int cmd_idx = 0;

unsigned char kbd_map[] = { 
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=','\b','\t',
    'Q','W','E','R','T','Y','U','I','O','P','[',']','\n', 0,
    'A','S','D','F','G','H','J','K','L',';','\'','`', 0,
    '\\','Z','X','C','V','B','N','M',',','.','/', 0,'*',0,' ' 
};

/* --- 3. Screen Helpers --- */
void print_at(const char* message, int row, int col, unsigned char color) {
    int index = (row * 80 + col) * 2;
    for (int i = 0; message[i] != '\0'; i++) {
        vidptr[index] = message[i];
        vidptr[index + 1] = color; 
        index += 2;
    }
}

void draw_ui() {
    /* 1. Rainbow Background (Full Screen) */
    for (int row = 0; row < 25; row++) {
        unsigned char color = (unsigned char)((row % 16) << 4) | 0x0F;
        for (int col = 0; col < 80; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';
            vidptr[index + 1] = color;
        }
    }

    /* 2. DCON Title (Big White Spaced Text) */
    print_at("D  C  O  N    v 1 . 0", 5, 28, 0x0F);

    /* 3. Command Box (10 lines thick, Moved up by 2) */
    for (int row = 13; row < 23; row++) {
        for (int col = 4; col < 76; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';
            vidptr[index + 1] = 0x00; /* Solid Black */
        }
    }
}

/* --- 4. The Shell (Command Processor) --- */
void run_command() {
    /* Check for "CL" (Clear/Reset UI) */
    if (cmd_buffer[0] == 'C' && cmd_buffer[1] == 'L') {
        draw_ui();
    }
    /* Check for "DEAN" (Secret Blue Screen) */
    else if (cmd_buffer[0] == 'D' && cmd_buffer[1] == 'E' && 
             cmd_buffer[2] == 'A' && cmd_buffer[3] == 'N') {
        for(int i=0; i<4000; i+=2) { vidptr[i]=' '; vidptr[i+1]=0x1F; }
        print_at("DCON: DEAN MODE ACTIVATED", 10, 25, 0x1F);
    }

    /* Clear buffer for next use */
    for(int i = 0; i < 80; i++) cmd_buffer[i] = 0;
    cmd_idx = 0;
}

/* --- 5. Keyboard Logic --- */
void check_keyboard() {
    if (inb(0x64) & 0x01) {
        unsigned char scancode = inb(0x60);
        
        if (scancode == 0x0E && cmd_idx > 0) { /* Backspace */
            cmd_idx--;
            cursor--;
            vidptr[cursor * 2] = ' ';
        } 
        else if (scancode == 0x1C) { /* Enter */
            run_command();
            cursor = 13 * 80 + 5; /* Reset cursor to the start of the black box */
        }
        else if (scancode < 128) {
            char letter = kbd_map[scancode];
            if (letter != 0 && cmd_idx < 70) {
                cmd_buffer[cmd_idx++] = letter;
                vidptr[cursor * 2] = letter;
                vidptr[cursor * 2 + 1] = 0x0F; /* White text on black background */
                cursor++;
            }
        }
    }
}

/* --- 6. Entry Point --- */
void kernel_main() {
    draw_ui();
    cursor = 13 * 80 + 5; /* Set cursor inside the moved black box */

    while(1) {
        check_keyboard();
        for(volatile int d=0; d<1000; d++); 
    }
}

