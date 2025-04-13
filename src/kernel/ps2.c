#include "ps2.h"

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
    outb(PS2_CMD, cmd)
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

