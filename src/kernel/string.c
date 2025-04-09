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
    int i = 0;
    while (i < n){
        dest_arr[i] = src_arr[i];
        i++;
    }
    return dest;
}

size_t strlen(const char *s) {
    const unsigned char *s_arr = s;
    size_t i = 0;
    while (s_arr[i] != '\0') i++;
    return i;
}

char *strcpy(char *dest, const char *src){

}

int strcmp(const char *s1, const char *s2){

}

const char *strchr(const char *s, int c){

}