#include "printk.h"
#include "vga.h"
#include "string.h"

#define PRINT_BUF_LEN 65
static char print_buf[PRINT_BUF_LEN];

// int conversion starting at most siginificant digit

static int count_digits(unsigned long long value, int base) {
    if (value == 0) {
        return 1;
    }
    int count = 0;
    while (value > 0) {
        count++;
        value /= base;
    }
    return count;
}

static char *int_to_string(long long value, int base) {
    // negative numbers for base 10
    int negative = 0;
    unsigned long long unsigned_value = value;
    if (value < 0 && base == 10) {
        negative = 1;
        unsigned_value = -value;
    }
    
    // special case of zero
    if (unsigned_value == 0) {
        print_buf[0] = '0';
        print_buf[1] = '\0';
        return print_buf;
    }
    
    int num_digits = count_digits(unsigned_value, base);
    int total_length = num_digits + negative;
    
    // buffer overflow handling
    if (total_length >= PRINT_BUF_LEN) {
        // truncate to fit
        total_length = PRINT_BUF_LEN - 1;
    }
    
    // null terminator
    print_buf[total_length] = '\0';
    
    // if negative, add minus sign
    if (negative) {
        print_buf[0] = '-';
    }
    
    // calculate place value (largest power of base that fits in the number)
    unsigned long long place_value = 1;
    while (place_value <= unsigned_value / base) {
        place_value *= base;
    }
    
    // digits from most to least significant
    int pos = negative;  // start after negative sign if present
    while (place_value > 0) {
        int digit = unsigned_value / place_value;
        print_buf[pos++] = digit < 10 ? '0' + digit : 'a' + digit - 10;
        unsigned_value %= place_value;
        place_value /= base;
    }
    
    return print_buf;
}

void print_char(char c) {
    VGA_display_char(c);
}

void print_str(const char *s) {
    if (s == NULL) {
        print_str("(null)");
        return;
    }
    VGA_display_str(s);
}

void print_int(int n) {
    print_str(int_to_string(n, 10));
}

void print_uint(unsigned int n) {
    print_str(int_to_string(n, 10));
}

void print_hex(unsigned int n) {
    print_str(int_to_string(n, 16));
}

void print_short(short n) {
    print_str(int_to_string(n, 10));
}

void print_ushort(unsigned short n) {
    print_str(int_to_string(n, 10));
}

void print_short_hex(unsigned short n) {
    print_str(int_to_string(n, 16));
}

void print_long(long n) {
    print_str(int_to_string(n, 10));
}

void print_ulong(unsigned long n) {
    print_str(int_to_string(n, 10));
}

void print_long_hex(unsigned long n) {
    print_str(int_to_string(n, 16));
}

void print_pointer(unsigned int p) {
    print_str("0x");
    print_str(int_to_string(p, 16));
}

int printk(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt != '%') {
            print_char(*fmt);
        }
        else {
            // for %h_ %l_ %q_
            int length_modifier = 0;
            if (*fmt == 'h') {
                length_modifier = 1;
                fmt++;
            } else if (*fmt == 'l') {
                length_modifier = 2;
                fmt++;
            } else if (*fmt == 'q') {
                length_modifier = 3;
                fmt++;
            }
            
        }
    }

    va_end(args);
}