#include "vga.h"
#include "string.h"

// constants for VGA text mode
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
static volatile uint16_t* const VGA_MEMORY = (volatile uint16_t*)0xb8000; // volatile because it may change at any time

// color declarations
#define VGA_BLACK 0
#define VGA_BLUE 1
#define VGA_GREEN 2
#define VGA_CYAN 3
#define VGA_RED 4
#define VGA_MAGENTA 5
#define VGA_BROWN 6
#define VGA_WHITE 7
#define VGA_GRAY 8
#define VGA_LIGHT_BLUE 9
#define VGA_LIGHT_GREEN 10
#define VGA_LIGHT_CYAN 11
#define VGA_LIGHT_RED 12
#define VGA_LIGHT_MAGENTA 13
#define VGA_YELLOW 14
#define VGA_BRIGHT_WHITE 15

// func to define color byte - attribute part of the word
#define VGA_COLOR(fg, bg) ((bg << 4) | fg)

// set default color
#define VGA_DEFAULT_COLOR VGA_COLOR(VGA_BRIGHT_WHITE, VGA_BLACK)

// variables to track cursor position
static int cursor_x = 0;
static int cursor_y = 0;

// helper func to determine index in buffer
static inline int vga_index(int x, int y) {
    return y * VGA_WIDTH + x;
}

static void vga_scroll() {
    // moves lines up by one
    for (int y = 0; y < VGA_HEIGHT - 1; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            VGA_MEMORY[vga_index(x, y)] = VGA_MEMORY[vga_index(x, y + 1)];
        }
    }
    
    // clear last line
    for (int x = 0; x < VGA_WIDTH; x++) {
        VGA_MEMORY[vga_index(x, VGA_HEIGHT - 1)] = VGA_DEFAULT_COLOR << 8 | ' ';
    }
}

// VGA_DEFAULT_COLOR << 8: sets color at high bits
// | __ : sets char at low bits

void VGA_clear(void) {
    int x;
    int y;
    for (y = 0; y < VGA_HEIGHT; y++){
        for (x = 0; x < VGA_WIDTH; x++){ 
            VGA_MEMORY[vga_index(x, y)] = VGA_DEFAULT_COLOR << 8 | ' '; // sets default/clear char as the space char
        }
    }
    cursor_x = 0;
    cursor_y = 0;
}

void VGA_display_char(char c) {
    switch (c) {
        case '\n':
            // new line
            cursor_x = 0;
            cursor_y++;
            break;
        
        case '\r':
            // Carriage return
            cursor_x = 0;
            break;
            
        case '\t':
            // tab
            cursor_x = (cursor_x + 8) & ~7;
            break;
            
        default:
            // display the character at cursor location
            VGA_MEMORY[vga_index(cursor_x, cursor_y)] = VGA_DEFAULT_COLOR << 8 | c;
            cursor_x++;
            break;
    }
    
    // line wrap
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }

    // scrolling
    if (cursor_y >= VGA_HEIGHT) {
        vga_scroll();
        cursor_y = VGA_HEIGHT - 1;
    }
}

void VGA_display_str(const char *str) {
    while (*str) {
        VGA_display_char(*str);
        str++;
    }
}
