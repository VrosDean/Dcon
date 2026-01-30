void check_keyboard() {
    if (inb(0x64) & 0x01) {
        unsigned char scancode = inb(0x60);

        if (scancode == 0x0E) { /* Backspace */
            if (cursor > 0) {
                cursor--;
                vidptr[cursor * 2] = ' ';
            }
        } 
        else if (scancode == 0x1C) { /* ENTER KEY - No more circles! */
            // Move cursor to the start of the next line (each line is 80 chars)
            cursor = ((cursor / 80) + 1) * 80;
        }
        else if (scancode < 128) { /* Normal Keys */
            char letter = kbd_map[scancode];
            if (letter != 0) {
                vidptr[cursor * 2] = letter;
                // Keep the rainbow background, just add white text
                vidptr[cursor * 2 + 1] = (vidptr[cursor * 2 + 1] & 0xF0) | 0x0F;
                cursor++;

                /* SECRET DEAN COMMAND CHECK */
                // If you type a '!' (just as a test), turn the screen blue!
                if (letter == '!') {
                    for(int i = 0; i < 80*25*2; i += 2) {
                        vidptr[i] = ' '; 
                        vidptr[i+1] = 0x1F; // Blue background, White text
                    }
                    print_at("DCON: DEAN BLUESCREEN ACTIVATED", 10, 25);
                }
            }
        }
    }
}

