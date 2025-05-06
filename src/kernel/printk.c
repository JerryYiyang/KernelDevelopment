#include "printk.h"
#include "vga.h"
#include "string.h"
#include "serial.h"

#define PRINT_BUF_LEN 65
static char print_buf[PRINT_BUF_LEN];
static int int_len;

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
    unsigned long long unsigned_value;
    if (value < 0 && base == 10) {
        negative = 1;
        unsigned_value = -value;
    } else {
        unsigned_value = (unsigned long long)value;
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

    int_len = total_length;

    return print_buf;
}

static char *uint_to_string(unsigned long long value, int base) {
    // Handle zero case
    if (value == 0) {
        print_buf[0] = '0';
        print_buf[1] = '\0';
        int_len = 1;
        return print_buf;
    }
    
    int num_digits = 0;
    unsigned long long temp = value;
    while (temp > 0) {
        num_digits++;
        temp /= base;
    }

    if (num_digits >= PRINT_BUF_LEN) {
        num_digits = PRINT_BUF_LEN - 1;
    }

    int_len = num_digits;
    
    print_buf[num_digits] = '\0';
    
    int pos = num_digits - 1;
    while (value > 0 && pos >= 0) {
        unsigned digit = value % base;
        print_buf[pos--] = digit < 10 ? '0' + digit : 'a' + digit - 10;
        value /= base;
    }
    
    return print_buf;
}

void print_char(char c) {
    VGA_display_char(c);
    ser_print_char(c);
}

void print_str(const char *s) {
    if (s == NULL) {
        print_str("(null)");
        return;
    }
    VGA_display_str(s);
    ser_print_str(s);
}

static void print_int(int n) {
    print_str(int_to_string(n, 10));
}

static void print_uint(unsigned int n) {
    print_str(int_to_string(n, 10));
}

static void print_hex(unsigned int n) {
    print_str(int_to_string(n, 16));
}

static void print_short(short n) {
    print_str(int_to_string(n, 10));
}

static void print_ushort(unsigned short n) {
    print_str(int_to_string(n, 10));
}

static void print_short_hex(unsigned short n) {
    print_str(int_to_string(n, 16));
}

static void print_long(long n) {
    print_str(int_to_string(n, 10));
}

static void print_ulong(unsigned long n) {
    print_str(uint_to_string((unsigned long long)n, 10));
}

static void print_ulonglong(unsigned long long n) {
    print_str(uint_to_string(n, 10));
}

static void print_long_hex(unsigned long n) {
    print_str(int_to_string(n, 16));
}

static void print_pointer(const void *p) { // use char * ??
    print_str("0x");
    print_str(int_to_string((uintptr_t)p, 16)); // casting here for saftey
}

int printk(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int num_printed = 0;

    while (*fmt) {
        if (*fmt != '%') {
            print_char(*fmt);
            num_printed++;
        }
        else {
            fmt++;
            // for %h_ %l_ %q_
            int len_mod = 0;
            if (*fmt == 'h') {
                len_mod = 1;
                fmt++;
            } else if (*fmt == 'l') {
                len_mod = 2;
                fmt++;
            } else if (*fmt == 'q') {
                len_mod = 3;
                fmt++;
            }

            switch (*fmt) {
                case '%':
                    print_char('%');
                    num_printed++;
                    break;
                case 'd':
                    if (len_mod == 1) {
                        print_short(va_arg(args, int)); // short prmoted
                        num_printed += int_len;
                    } else if (len_mod == 2) {
                        print_long(va_arg(args, long));
                        num_printed += int_len;
                    } else if (len_mod == 3) {
                        print_long(va_arg(args, long long));
                        num_printed += int_len;
                    } else {
                        print_int(va_arg(args, int));
                        num_printed += int_len;
                    }
                    break;
                    case 'u':
                        if (len_mod == 1) {
                            print_ushort(va_arg(args, unsigned int));
                            num_printed += int_len;
                        } else if (len_mod == 2) {
                            print_ulong(va_arg(args, unsigned long));
                            num_printed += int_len;
                        } else if (len_mod == 3) {
                            print_ulonglong(va_arg(args, unsigned long long));
                            num_printed += int_len;
                        } else {
                            print_uint(va_arg(args, unsigned int));
                            num_printed += int_len;
                        }
                        break;
                case 'x':
                    if (len_mod == 1) {
                        print_short_hex(va_arg(args, unsigned int)); // short promoted
                        num_printed += int_len;
                    } else if (len_mod == 2) {
                        print_long_hex(va_arg(args, unsigned long long));
                        num_printed += int_len;
                    } else if (len_mod == 3) {
                        print_long_hex(va_arg(args, unsigned long long));
                        num_printed += int_len;
                    } else {
                        print_hex(va_arg(args, unsigned int));
                        num_printed += int_len;
                    }
                    break;
                case 'c':
                    print_char(va_arg(args, int)); // char promoted
                    num_printed++;
                    break;
                case 'p':
                    print_pointer(va_arg(args, const void *));
                    num_printed = num_printed + 2 + int_len;
                    break;
                case 's':
                    {
                    const char *s = va_arg(args, const char *);
                    print_str(s);
                    num_printed += strlen(s);
                    }
                    break;
            }
        }
        fmt++;
    }
    va_end(args);
    return num_printed;
}

void ser_print_char(char c) {
    char buff[1] = {c};
    SER_write(buff, 1);
}

void ser_print_str(const char *s) {
    if (s == NULL) {
        ser_print_str("(null)");
        return;
    }
    SER_write(s, strlen(s));
}