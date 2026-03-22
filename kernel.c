/* 1. HARDWARE PORTS */
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

/* 2. GLOBAL VARIABLES */
volatile char* vidptr = (volatile char*)0xb8000;
int cursor = 0;
char cmd_buffer[81]; 
int cmd_idx = 0;
int is_green_screen = 0;

// Added missing keys and made it more robust
unsigned char kbd_map[] = { 
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b', 
    '\t', 'q','w','e','r','t','y','u','i','o','p','[',']','\n', 
    0, 'a','s','d','f','g','h','j','k','l',';','\'','`', 
    0, '\\','z','x','c','v','b','n','m',',','.','/', 0, '*', 0, ' ' 
};

/* 3. SCREEN HELPERS */
void print_at(const char* message, int row, int col) {
    int index = (row * 80 + col);
    if (index > 2000) return; // Safety check
    
    for (int i = 0; message[i] != '\0'; i++) {
        vidptr[index * 2] = message[i];
        vidptr[index * 2 + 1] = 0x0F; // White on Black
        index++;
    }
}

void draw_ui() {
    // Fill background
    for (int i = 0; i < 2000; i++) {
        vidptr[i * 2] = ' ';
        vidptr[i * 2 + 1] = 0x1F; // Blue background
    }
    print_at("DCON v1.1", 5, 35);
    is_green_screen = 0;
}

/* 4. COMMAND BRAIN */
void run_command() {
    cmd_buffer[cmd_idx] = '\0'; // Properly terminate the string

    // Simple "dean" check (lowercase check is easier)
    if ((cmd_buffer[0] == 'd' || cmd_buffer[0] == 'D') &&
        (cmd_buffer[1] == 'e' || cmd_buffer[1] == 'E') &&
        (cmd_buffer[2] == 'a' || cmd_buffer[2] == 'A') &&
        (cmd_buffer[3] == 'n' || cmd_buffer[3] == 'N')) {
        for(int i = 0; i < 4000; i += 2) { vidptr[i] = ' '; vidptr[i+1] = 0x2F; }
        print_at("DEAN GREEN SCREEN ACTIVATED", 12, 25);
        is_green_screen = 1;
    } 
    else if (cmd_buffer[0] == 'c' && cmd_buffer[1] == 'l' && cmd_buffer[2] == 's') {
        draw_ui();
        cursor = 14 * 80 + 5;
    }

    // Reset buffer
    for(int i=0; i<81; i++) cmd_buffer[i] = 0;
    cmd_idx = 0;
}

/* 5. KEYBOARD & MAIN */
void check_keyboard() {
    // Check if a key is pressed (Status Port 0x64, bit 0)
    if (inb(0x64) & 0x01) {
        unsigned char scancode = inb(0x60);
        
        // Only handle "Key Down" events (scancodes < 0x80)
        if (scancode < 0x80) {
            if (scancode == 0x1C) { // Enter Key
                run_command(); 
                if(!is_green_screen) cursor = 21 * 80 + 5; 
            } else {
                char letter = kbd_map[scancode];
                if (letter && cmd_idx < 80) { 
                    vidptr[cursor * 2] = letter;
                    vidptr[cursor * 2 + 1] = 0x0F;
                    cmd_buffer[cmd_idx++] = letter; 
                    cursor++; 
                }
            }
        }
    }
}

void kernel_main() {
    draw_ui();
    cursor = 14 * 80 + 5;
    while(1) { check_keyboard(); }
}

