/* 1. HARDWARE PORTS */
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

/* 2. GLOBAL VARIABLES */
volatile char* vidptr = (volatile char*)0xb8000;
int cursor = 14 * 80 + 5;
char cmd_buffer[81]; 
int cmd_idx = 0;
int is_green_mode = 0;

unsigned char kbd_map[] = { 
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b', 
    '\t', 'q','w','e','r','t','y','u','i','o','p','[',']','\n', 
    0, 'a','s','d','f','g','h','j','k','l',';','\'','`', 
    0, '\\','z','x','c','v','b','n','m',',','.','/', 0, '*', 0, ' ' 
};

/* 3. HELPERS */
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

void print_at(const char* message, int row, int col) {
    int index = (row * 80 + col) * 2;
    for (int i = 0; message[i] != '\0'; i++) {
        vidptr[index] = message[i];
        vidptr[index + 1] = (vidptr[index + 1] & 0xF0) | 0x0F; 
        index += 2;
    }
}

void draw_ui() {
    is_green_mode = 0;
    unsigned char colors[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF};
    for (int row = 0; row < 25; row++) {
        unsigned char bg = colors[row % 13] << 4;
        for (int col = 0; col < 80; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';
            vidptr[index + 1] = bg;
        }
    }
    for (int row = 14; row < 24; row++) {
        for (int col = 4; col < 76; col++) {
            vidptr[(row * 80 + col) * 2 + 1] = 0x00; 
        }
    }
    print_at("DCON v1.1", 5, 35);
}

/* 4. COMMAND BRAIN */
void run_command() {
    cmd_buffer[cmd_idx] = '\0';

    if (strcmp(cmd_buffer, "cls") == 0) {
        draw_ui();
        cursor = 14 * 80 + 5;
    } 
    else if (strcmp(cmd_buffer, "dean") == 0) {
        is_green_mode = 1;
        for(int i = 0; i < 2000; i++) {
            vidptr[i*2] = ' ';
            vidptr[i*2+1] = 0x2F; 
        }
        print_at("DEAN GREEN SCREEN ACTIVATED", 12, 25);
        print_at("PRESS ESC TO EXIT", 14, 30);
    } 
    else if (strcmp(cmd_buffer, "help") == 0) {
        print_at("Available: cls, dean, help", 15, 5);
        cursor = 16 * 80 + 5;
    }

    for(int i = 0; i < 81; i++) cmd_buffer[i] = 0;
    cmd_idx = 0;
}

/* 5. KEYBOARD LOGIC */
void check_keyboard() {
    if (inb(0x64) & 0x01) {
        unsigned char scancode = inb(0x60);
        
        if (scancode == 0x01) { // ESC Key
            draw_ui();
            cursor = 14 * 80 + 5;
        }
        else if (scancode == 0x1C) { // ENTER
            run_command();
        } 
        else if (scancode == 0x0E) { // BACKSPACE
            if (cmd_idx > 0) {
                cmd_idx--;
                cmd_buffer[cmd_idx] = 0;
                cursor--;
                vidptr[cursor * 2] = ' ';
                vidptr[cursor * 2 + 1] = 0x0F;
            }
        }
        else if (scancode < 0x80) { // REGULAR KEYS
            char letter = kbd_map[scancode];
            if (letter && cmd_idx < 70) {
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
    while(1) { check_keyboard(); }
}

