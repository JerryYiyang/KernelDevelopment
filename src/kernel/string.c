#include "string.h"

void *memset(void *dst, int c, size_t n) {
    unsigned char *p = dst;
    while (n > 0){
        *p = (unsigned char) c;
        p++;
        n--;
    }
    return dst;
}

void *memcpy(void *dest, const void *src, size_t n) {
    if (dest == NULL || src == NULL) return NULL;
    unsigned char *dest_arr = dest;
    const unsigned char *src_arr = src;
    size_t i = 0;
    while (i < n){
        dest_arr[i] = src_arr[i];
        i++;
    }
    return dest;
}

size_t strlen(const char *s) {
    const char *s_arr = s;
    size_t i = 0;
    while (s_arr[i] != '\0') i++;
    return i;
}

char *strcpy(char *dest, const char *src) {
    char *og_dest = dest;
    while (*src != '\0'){
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
    return og_dest;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

const char *strchr(const char *s, int c) {
    const unsigned char *p = (const unsigned char *)s;
    unsigned char ch = (unsigned char)c;
    while (*p != '\0') {
        if (*p == ch) 
            return (const char *)p;
        p++;
    }
    if (ch == '\0')
        return (const char *)p;

    // c not found in string
    return NULL;
}

// char *strdup(const char *s) {

// }