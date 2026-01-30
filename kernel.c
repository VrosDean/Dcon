void run_command() {
    /* 1. DEAN / dean - Green Screen */
    if (cmd_idx == 4 && 
       (cmd_buffer[0] == 'D' || cmd_buffer[0] == 'd') && 
       (cmd_buffer[1] == 'E' || cmd_buffer[1] == 'e') && 
       (cmd_buffer[2] == 'A' || cmd_buffer[2] == 'a') && 
       (cmd_buffer[3] == 'N' || cmd_buffer[3] == 'n')) {
        
        for(int i = 0; i < 4000; i += 2) { vidptr[i] = ' '; vidptr[i+1] = 0x2F; }
        print_at("DEAN GREEN SCREEN ACTIVATED", 12, 25);
        is_green_screen = 1;
    }
    /* 2. PACKON / packon */
    else if (cmd_idx == 6 && 
            (cmd_buffer[0] == 'P' || cmd_buffer[0] == 'p') && 
            (cmd_buffer[1] == 'A' || cmd_buffer[1] == 'a')) {
        print_at("PackON: INSTALLING...", 15, 5);
    }
    /* 3. CLS / cls */
    else if (cmd_idx == 3 && 
            (cmd_buffer[0] == 'C' || cmd_buffer[0] == 'c') && 
            (cmd_buffer[1] == 'L' || cmd_buffer[1] == 'l')) {
        draw_ui();
        cursor = 14 * 80 + 5;
    }

    for(int i = 0; i < 80; i++) cmd_buffer[i] = 0;
    cmd_idx = 0;
}

