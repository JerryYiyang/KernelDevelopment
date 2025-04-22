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

    // flush buf in case there's bad data
    while (inb(PS2_STATUS) & PS2_STATUS_OUTPUT) {
        inb(PS2_DATA);
    }

    ps2_write_command(PS2_READ_CONFIG);
    config = ps2_read_data();
    config &= ~0x10;  // Enable clock 1
    config |= 0x01;   // Enable interrupt 1
    config |= 0x20;   // Disable clock 2
    config &= ~0x02;  // Disable interrupt 2
    config &= ~0x40;  // Disable translation
    ps2_write_command(PS2_WRITE_CONFIG);
    ps2_write_data(config);
    ps2_write_command(PS2_ENABLE_PORT1);
}

int kb_init(void) {
    ps2_write_data(KB_RESET);
    
    unsigned char response = ps2_read_data();

    while ((response = ps2_read_data()) != KB_TEST_PASS) {
        if (response == KB_TEST_FAIL1 || response == KB_TEST_FAIL2) {
            printk("Keyboard self test failed with code 0x%x\n", response);
            return -1;
        }
    }

    ps2_write_data(KB_SCAN);
    ps2_read_data();
    ps2_write_data(2);
    ps2_read_data();
    ps2_write_data(KB_ENABLE);
    ps2_read_data();
    ps2_write_data(KB_REPEAT_RATE);
    ps2_read_data();
    ps2_write_data(0x00);
    ps2_read_data();
    return 0;
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
    // if either (shift is on and caps lock is off) or (shift is off and caps lock is on) then uppercase, otherwise lowercase
    char lower_case = 0;
    char upper_case = 0;
    switch (scancode) {
        // Letters
        case KB_SC_A: lower_case = 'a'; upper_case = 'A'; break;
        case KB_SC_B: lower_case = 'b'; upper_case = 'B'; break;
        case KB_SC_C: lower_case = 'c'; upper_case = 'C'; break;
        case KB_SC_D: lower_case = 'd'; upper_case = 'D'; break;
        case KB_SC_E: lower_case = 'e'; upper_case = 'E'; break;
        case KB_SC_F: lower_case = 'f'; upper_case = 'F'; break;
        case KB_SC_G: lower_case = 'g'; upper_case = 'G'; break;
        case KB_SC_H: lower_case = 'h'; upper_case = 'H'; break;
        case KB_SC_I: lower_case = 'i'; upper_case = 'I'; break;
        case KB_SC_J: lower_case = 'j'; upper_case = 'J'; break;
        case KB_SC_K: lower_case = 'k'; upper_case = 'K'; break;
        case KB_SC_L: lower_case = 'l'; upper_case = 'L'; break;
        case KB_SC_M: lower_case = 'm'; upper_case = 'M'; break;
        case KB_SC_N: lower_case = 'n'; upper_case = 'N'; break;
        case KB_SC_O: lower_case = 'o'; upper_case = 'O'; break;
        case KB_SC_P: lower_case = 'p'; upper_case = 'P'; break;
        case KB_SC_Q: lower_case = 'q'; upper_case = 'Q'; break;
        case KB_SC_R: lower_case = 'r'; upper_case = 'R'; break;
        case KB_SC_S: lower_case = 's'; upper_case = 'S'; break;
        case KB_SC_T: lower_case = 't'; upper_case = 'T'; break;
        case KB_SC_U: lower_case = 'u'; upper_case = 'U'; break;
        case KB_SC_V: lower_case = 'v'; upper_case = 'V'; break;
        case KB_SC_W: lower_case = 'w'; upper_case = 'W'; break;
        case KB_SC_X: lower_case = 'x'; upper_case = 'X'; break;
        case KB_SC_Y: lower_case = 'y'; upper_case = 'Y'; break;
        case KB_SC_Z: lower_case = 'z'; upper_case = 'Z'; break;
    }
    if (lower_case != 0) {
        if ((shift && !capslock) || (!shift && capslock)) {
            return upper_case;
        } else {
            return lower_case;
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

int is_modifier (unsigned char scancode) {
    if (scancode == KB_SC_LSHIFT || scancode == KB_SC_RSHIFT ||
        scancode == KB_SC_LCTRL || scancode == KB_SC_CAPSLOCK ||
        scancode == KB_SC_LALT) return 1;
    return 0;
}

void kb_polling(void) {
    unsigned char scancode;
    int extended = 0, capslock = 0, shift = 0, ctrl = 0, alt = 0;
    char c;

    while(1) {
        scancode = ps2_read_data();

        // handling prefixes
        if (scancode == KB_EXTENDED) {
            extended = 1;
            scancode = ps2_read_data();
        }
        if (scancode == KB_KEY_RELEASE) {
            scancode = ps2_read_data();
            if (scancode == KB_SC_LSHIFT || scancode == KB_SC_RSHIFT) shift = 0;
            else if (scancode == KB_SC_LCTRL) ctrl = 0;
            else if (scancode == KB_SC_LALT) alt = 0;
            continue;
        }

        // handling modifiers
        if (scancode == KB_SC_CAPSLOCK) capslock ^= 1;
        else if (scancode == KB_SC_LSHIFT || scancode == KB_SC_RSHIFT) {
            shift = 1;
            continue;
        } else if (scancode == KB_SC_LCTRL) {
            ctrl = 1;
            continue;
        } else if (scancode == KB_SC_LALT) {
            alt = 1;
            continue;
        }
    
        if (ctrl && !extended) {
            c = scancode_to_ascii(scancode, extended, capslock, shift);
            if (c >= 'a' && c <= 'z') {
                c = c - 'a' + 1;
            } else if (c >= 'A' && c <= 'Z') {
                c = c - 'A' + 1;
            }
            if (c) printk("Ctrl+%c", c + 'a' - 1);
            continue;
        }

        if (alt && !extended) {
            c = scancode_to_ascii(scancode, extended, capslock, shift);
            if (c) printk("Alt+%c", c);
            continue;
        }

        if (!is_modifier(scancode)) {
            c = scancode_to_ascii(scancode, extended, capslock, shift);
            printk("%c", c);
        }
        extended = 0;
    }
}