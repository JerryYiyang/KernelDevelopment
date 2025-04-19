#include "ps2.h"
#include "printk.h"
#include <stdint.h>
#include "keyboard_scancodes.h"

// contains ps2 and keyboard drivers

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0"
                    : "=a"(ret)
                    : "Nd"(port) );
    return ret;
}

static char ps2_poll_read(void) {
    char status = inb(PS2_STATUS);
    while (!(status & PS2_STATUS_OUTPUT)) status = inb(PS2_STATUS);
    return inb(PS2_DATA);
}

static void ps2_wait_write(void) {
    char status = inb(PS2_STATUS);
    while (status & PS2_STATUS_INPUT) status = inb(PS2_STATUS);
}

static void ps2_wait_read(void) {
    char status = inb(PS2_STATUS);
    while (!(status & PS2_STATUS_OUTPUT)) status = inb(PS2_STATUS);
}

static char ps2_read_data(void) {
    ps2_wait_read();
    return inb(PS2_DATA);
}

static void ps2_write_data(char cmd) {
    ps2_wait_write();
    outb(PS2_DATA, cmd);
}

static void ps2_write_command(char cmd) {
    ps2_wait_write();
    outb(PS2_CMD, cmd);
}

void ps2_init(void) {
    char config;

    ps2_write_command(PS2_DISABLE_PORT1);
    ps2_write_command(PS2_DISABLE_PORT2);
    config = ps2_read_data();
    config |= 0x01;  // enable interrupt 1
    config &= ~0x02; // disable interrupt 2
    config &= ~0x08; // enable clock 1
    config |= 0x10;  // disable clock 2
    ps2_write_command(PS2_WRITE_CONFIG);
    ps2_write_data(config);
}

int kb_init(void) {
    ps2_write_command(KB_RESET);
    unsigned char response = ps2_read_data();
    if (response == KB_TEST_FAIL1 || response == KB_TEST_FAIL2) {
        printk("Keyboard self test failed with code 0x%x\n", response);
        ps2_write_command(KB_RESET);
        response = ps2_read_data();
        if (response == KB_TEST_FAIL1 || response == KB_TEST_FAIL2) {
            printk("Keyboard failure detected\n");
            return -1;
        }
    }
    if (response == KB_TEST_PASS) {
        ps2_write_data(KB_SCAN);
        ps2_write_data(2);
        ps2_write_data(KB_ENABLE);
        ps2_write_data(KB_REPEAT_RATE);
        ps2_read_data(); // Read acknowledgement and clears data buf
        ps2_write_data(0x00);
        ps2_read_data();
        return 0;
    }
    printk("Unexpected keyboard response with code 0x%x\n", response);
    return -1;
}

char scancode_to_ascii(unsigned char scancode, int extended, int capslock, int shift) {
    // extended
    if (extended) {
        switch (scancode) {
            case 0x75: return '^';
            case 0x72: return 'v';
            case 0x6B: return '<';
            case 0x74: return '>';
            
            // Keypad keys (with numlock off)
            case 0x70: return '0';  // Insert/0
            case 0x71: return '.';  // Delete/.
            case 0x6C: return '7';  // Home/7
            case 0x69: return '1';  // End/1
            case 0x7D: return '9';  // PgUp/9
            case 0x7A: return '3';  // PgDn/3
            case 0x4A: return '/';  // Keypad /
            case 0x5A: return '\n'; // Keypad Enter
            default: return 0;
        }
    }
    // capslock and/or shift
    if (scancode >= KB_SC_A && scancode <= KB_SC_Z) {
        // if either (shift is on and caps lock is off) or (shift is off and caps lock is on) then uppercase, otherwise lowercase
        if ((shift && !capslock) || (!shift && capslock)) {
            switch (scancode) {
                case KB_SC_A: return 'A';
                case KB_SC_B: return 'B';
                case KB_SC_C: return 'C';
                case KB_SC_D: return 'D';
                case KB_SC_E: return 'E';
                case KB_SC_F: return 'F';
                case KB_SC_G: return 'G';
                case KB_SC_H: return 'H';
                case KB_SC_I: return 'I';
                case KB_SC_J: return 'J';
                case KB_SC_K: return 'K';
                case KB_SC_L: return 'L';
                case KB_SC_M: return 'M';
                case KB_SC_N: return 'N';
                case KB_SC_O: return 'O';
                case KB_SC_P: return 'P';
                case KB_SC_Q: return 'Q';
                case KB_SC_R: return 'R';
                case KB_SC_S: return 'S';
                case KB_SC_T: return 'T';
                case KB_SC_U: return 'U';
                case KB_SC_V: return 'V';
                case KB_SC_W: return 'W';
                case KB_SC_X: return 'X';
                case KB_SC_Y: return 'Y';
                case KB_SC_Z: return 'Z';
            }
        } else {
            switch (scancode) {
                case KB_SC_A: return 'a';
                case KB_SC_B: return 'b';
                case KB_SC_C: return 'c';
                case KB_SC_D: return 'd'; 
                case KB_SC_E: return 'e';
                case KB_SC_F: return 'f';
                case KB_SC_G: return 'g';
                case KB_SC_H: return 'h';
                case KB_SC_I: return 'i';
                case KB_SC_J: return 'j';
                case KB_SC_K: return 'k';
                case KB_SC_L: return 'l';
                case KB_SC_M: return 'm';
                case KB_SC_N: return 'n';
                case KB_SC_O: return 'o';
                case KB_SC_P: return 'p';
                case KB_SC_Q: return 'q';
                case KB_SC_R: return 'r';
                case KB_SC_S: return 's';
                case KB_SC_T: return 't';
                case KB_SC_U: return 'u';
                case KB_SC_V: return 'v';
                case KB_SC_W: return 'w';
                case KB_SC_X: return 'x';
                case KB_SC_Y: return 'y';
                case KB_SC_Z: return 'z';
            }
        }
    }
    
    // shift
    if (shift) {
        switch (scancode) {
            case KB_SC_1: return '!';
            case KB_SC_2: return '@';
            case KB_SC_3: return '#';
            case KB_SC_4: return '$';
            case KB_SC_5: return '%';
            case KB_SC_6: return '^';
            case KB_SC_7: return '&';
            case KB_SC_8: return '*';
            case KB_SC_9: return '(';
            case KB_SC_0: return ')';
            
            case KB_SC_BACKTICK: return '~';
            case KB_SC_MINUS: return '_';
            case KB_SC_EQUALS: return '+';
            case KB_SC_LBRACKET: return '{';
            case KB_SC_RBRACKET: return '}';
            case KB_SC_BACKSLASH: return '|';
            case KB_SC_SEMICOLON: return ':';
            case KB_SC_QUOTE: return '"';
            case KB_SC_COMMA: return '<';
            case KB_SC_PERIOD: return '>';
            case KB_SC_SLASH: return '?';
            
            default: return 0;
        }
    } else {
        // no shift
        switch (scancode) {
            case KB_SC_1: return '1';
            case KB_SC_2: return '2';
            case KB_SC_3: return '3';
            case KB_SC_4: return '4';
            case KB_SC_5: return '5';
            case KB_SC_6: return '6';
            case KB_SC_7: return '7';
            case KB_SC_8: return '8';
            case KB_SC_9: return '9';
            case KB_SC_0: return '0';
            
            case KB_SC_BACKTICK: return '`';
            case KB_SC_MINUS: return '-';
            case KB_SC_EQUALS: return '=';
            case KB_SC_LBRACKET: return '[';
            case KB_SC_RBRACKET: return ']';
            case KB_SC_BACKSLASH: return '\\';
            case KB_SC_SEMICOLON: return ';';
            case KB_SC_QUOTE: return '\'';
            case KB_SC_COMMA: return ',';
            case KB_SC_PERIOD: return '.';
            case KB_SC_SLASH: return '/';
            
            case KB_SC_SPACE: return ' ';
            case KB_SC_ENTER: return '\n';
            case KB_SC_BACKSPACE: return '\b';
            case KB_SC_TAB: return '\t';
            case KB_SC_ESC: return 27;  // ASCII ESC
            
            // Keypad (numlock on)
            case KB_SC_KP_0: return '0';
            case KB_SC_KP_1: return '1';
            case KB_SC_KP_2: return '2';
            case KB_SC_KP_3: return '3';
            case KB_SC_KP_4: return '4';
            case KB_SC_KP_5: return '5';
            case KB_SC_KP_6: return '6';
            case KB_SC_KP_7: return '7';
            case KB_SC_KP_8: return '8';
            case KB_SC_KP_9: return '9';
            case KB_SC_KP_STAR: return '*';
            case KB_SC_KP_MINUS: return '-';
            case KB_SC_KP_PLUS: return '+';
            case KB_SC_KP_DOT: return '.';
            
            default: return 0;
        }
    }
    return 0;
}

void kb_polling(void) {
    unsigned char code;
    int extended, capslock, shift = 0;
    char c;

    while(1) {
        code = ps2_poll_read();
        if (scancode == KB_EXTENDED) extended = 1;
        else if (scancode == KB_SC_CAPSLOCK) capslock = 1;
        else if (scancode == KB_SC_LSHIFT || scancode == KB_SC_RSHIFT) shift = 1;
        if ((extended + capslock + shift) > 0) code = ps2_poll_read();
        c = scancode_to_ascii(scancode, extended, capslock, shift);
        printk(c);
    }
}