#include "ps2.h"
#include "printk.h"

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

// are keyboard commands sent to cmd or data?
int kb_init(void) {
    ps2_write_command(KB_RESET);
    char response = ps2_read_data();
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
        ps2_write_command(KB_SCAN);
        ps2_write_data(3); // set scan code 3 ??
        ps2_write_command(KB_ENABLE);
        return 0;
    }
    printk("Unexpected keyboard response with code 0x%x\n");
}