/* Updated print_at: Now keeps the background color perfectly! */
void print_at(const char* message, int row, int col) {
    int index = (row * 80 + col) * 2;
    for (int i = 0; message[i] != '\0'; i++) {
        vidptr[index] = message[i];
        /* 0xF0 keeps the background, 0x0F makes the text white */
        vidptr[index + 1] = (vidptr[index + 1] & 0xF0) | 0x0F; 
        index += 2;
    }
}

void draw_ui() {
    unsigned char colors[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF};
    int color_count = 13;

    /* 1. Rainbow Background */
    for (int row = 0; row < 25; row++) {
        unsigned char bg = colors[row % color_count] << 4;
        for (int col = 0; col < 80; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';
            vidptr[index + 1] = bg;
        }
    }

    /* 2. DCON Title (Now has NO box around it!) */
    print_at("DCON v1.0", 5, 35);
    print_at("DEAN CONSOLE", 6, 34);

    /* 3. The Command Box */
    for (int row = 14; row < 24; row++) {
        for (int col = 4; col < 76; col++) {
            int index = (row * 80 + col) * 2;
            vidptr[index] = ' ';
            vidptr[index + 1] = 0x00; /* Solid Black */
        }
    }
}

