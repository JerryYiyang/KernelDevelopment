#ifndef KEYBOARD_SCANCODES_H
#define KEYBOARD_SCANCODES_H

#define KB_SC_ESC       0x76    // Escape key
#define KB_SC_F1        0x05    // F1
#define KB_SC_F2        0x06    // F2
#define KB_SC_F3        0x04    // F3
#define KB_SC_F4        0x0C    // F4
#define KB_SC_F5        0x03    // F5
#define KB_SC_F6        0x0B    // F6
#define KB_SC_F7        0x83    // F7
#define KB_SC_F8        0x0A    // F8
#define KB_SC_F9        0x01    // F9
#define KB_SC_F10       0x09    // F10
#define KB_SC_F11       0x78    // F11
#define KB_SC_F12       0x07    // F12

#define KB_SC_BACKTICK  0x0E    // ` (backtick)
#define KB_SC_1         0x16    // 1
#define KB_SC_2         0x1E    // 2
#define KB_SC_3         0x26    // 3
#define KB_SC_4         0x25    // 4
#define KB_SC_5         0x2E    // 5
#define KB_SC_6         0x36    // 6
#define KB_SC_7         0x3D    // 7
#define KB_SC_8         0x3E    // 8
#define KB_SC_9         0x46    // 9
#define KB_SC_0         0x45    // 0
#define KB_SC_MINUS     0x4E    // - (minus)
#define KB_SC_EQUALS    0x55    // = (equals)
#define KB_SC_BACKSPACE 0x66    // Backspace

#define KB_SC_TAB       0x0D    // Tab
#define KB_SC_Q         0x15    // Q
#define KB_SC_W         0x1D    // W
#define KB_SC_E         0x24    // E
#define KB_SC_R         0x2D    // R
#define KB_SC_T         0x2C    // T
#define KB_SC_Y         0x35    // Y
#define KB_SC_U         0x3C    // U
#define KB_SC_I         0x43    // I
#define KB_SC_O         0x44    // O
#define KB_SC_P         0x4D    // P
#define KB_SC_LBRACKET  0x54    // [ (left bracket)
#define KB_SC_RBRACKET  0x5B    // ] (right bracket)
#define KB_SC_BACKSLASH 0x5D    // \ (backslash)

#define KB_SC_CAPS_LOCK 0x58    // Caps Lock
#define KB_SC_A         0x1C    // A
#define KB_SC_S         0x1B    // S
#define KB_SC_D         0x23    // D
#define KB_SC_F         0x2B    // F
#define KB_SC_G         0x34    // G
#define KB_SC_H         0x33    // H
#define KB_SC_J         0x3B    // J
#define KB_SC_K         0x42    // K
#define KB_SC_L         0x4B    // L
#define KB_SC_SEMICOLON 0x4C    // ; (semicolon)
#define KB_SC_QUOTE     0x52    // ' (quote)
#define KB_SC_ENTER     0x5A    // Enter

#define KB_SC_LSHIFT    0x12    // Left Shift
#define KB_SC_Z         0x1A    // Z
#define KB_SC_X         0x22    // X
#define KB_SC_C         0x21    // C
#define KB_SC_V         0x2A    // V
#define KB_SC_B         0x32    // B
#define KB_SC_N         0x31    // N
#define KB_SC_M         0x3A    // M
#define KB_SC_COMMA     0x41    // , (comma)
#define KB_SC_PERIOD    0x49    // . (period)
#define KB_SC_SLASH     0x4A    // / (slash)
#define KB_SC_RSHIFT    0x59    // Right Shift

#define KB_SC_LCTRL     0x14        // Left Control
#define KB_SC_LALT      0x11        // Left Alt
#define KB_SC_SPACE     0x29        // Space
#define KB_SC_RALT      0xE0, 0x11  // Right Alt (extended key)
#define KB_SC_RCTRL     0xE0, 0x14  // Right Control (extended key)

#define KB_SC_UP        0xE0, 0x75  // Up arrow
#define KB_SC_LEFT      0xE0, 0x6B  // Left arrow
#define KB_SC_RIGHT     0xE0, 0x74  // Right arrow
#define KB_SC_DOWN      0xE0, 0x72  // Down arrow

#define KB_SC_INSERT    0xE0, 0x70  // Insert
#define KB_SC_DELETE    0xE0, 0x71  // Delete
#define KB_SC_HOME      0xE0, 0x6C  // Home
#define KB_SC_END       0xE0, 0x69  // End
#define KB_SC_PGUP      0xE0, 0x7D  // Page Up
#define KB_SC_PGDN      0xE0, 0x7A  // Page Down

#define KB_SC_NUM_LOCK  0x77        // Num Lock
#define KB_SC_KP_SLASH  0xE0, 0x4A  // Keypad / (extended key)
#define KB_SC_KP_STAR   0x7C        // Keypad *
#define KB_SC_KP_MINUS  0x7B        // Keypad -
#define KB_SC_KP_PLUS   0x79        // Keypad +
#define KB_SC_KP_ENTER  0xE0, 0x5A  // Keypad Enter (extended key)
#define KB_SC_KP_DOT    0x71        // Keypad .
#define KB_SC_KP_0      0x70        // Keypad 0
#define KB_SC_KP_1      0x69        // Keypad 1
#define KB_SC_KP_2      0x72        // Keypad 2
#define KB_SC_KP_3      0x7A        // Keypad 3
#define KB_SC_KP_4      0x6B        // Keypad 4
#define KB_SC_KP_5      0x73        // Keypad 5
#define KB_SC_KP_6      0x74        // Keypad 6
#define KB_SC_KP_7      0x6C        // Keypad 7
#define KB_SC_KP_8      0x75        // Keypad 8
#define KB_SC_KP_9      0x7D        // Keypad 9

#define KB_SC_PRINT_SCREEN  0xE0, 0x7C  // Print Screen (extended key)
#define KB_SC_SCROLL_LOCK   0x7E    // Scroll Lock
#define KB_SC_PAUSE         0xE1, 0x14, 0x77, 0xE1, 0xF0, 0x14, 0xF0, 0x77  // Pause/Break (special sequence)

#define KB_SC_LWIN      0xE0, 0x1F  // Left Windows key (extended key)
#define KB_SC_RWIN      0xE0, 0x27  // Right Windows key (extended key)
#define KB_SC_MENU      0xE0, 0x2F  // Menu key (extended key)

#define KB_KEY_RELEASE  0xF0    // Key release prefix
#define KB_EXTENDED     0xE0    // Extended key prefix
#define KB_SPECIAL      0xE1    // Special key prefix (used by Pause)

#endif