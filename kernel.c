volatile char* vidptr = (volatile char*)0xb8000;

void print_at(const char* message, int row, int col) {
    int index = (row * 80 + col) * 2;
    for (int i = 0; message[i] != '\0'; i++) {
        vidptr[index] = message[i];
        // Keep the background color that's already there, just make text white (0x0F)
        vidptr[index + 1] = (vidptr[index + 1] & 0xF0) | 0x0F; 
        index += 2;
    }
}

void draw_ui() {
    // These are the VGA background color codes
    unsigned char colors[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF};
    
    for (int row = 0; row < 25; row++) {
        // Shift left by 4 to set the BACKGROUND color
        unsigned char bg = colors[row % 13] << 4; 
        
        for (int col = 0; col < 80; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';     // Clear character
            vidptr[index + 1] = bg;  // Set the stripe color
        }
    }
    
    // Draw the "Input Box" (Black rectangle at the bottom)
    for (int row = 14; row < 24; row++) {
        for (int col = 4; col < 76; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index + 1] = 0x00; // Black background
        }
    }

    print_at("DCON v1.1", 5, 35);
}

void kernel_main(unsigned int magic, unsigned int addr) {
    draw_ui();
    while(1); // Stay alive
}

