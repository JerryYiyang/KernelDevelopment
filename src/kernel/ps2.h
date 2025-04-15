#ifndef PS2_H
#define PS2_H

// ports
#define PS2_DATA 0x60
#define PS2_CMD 0x64
#define PS2_STATUS PS2_CMD
#define PS2_STATUS_OUTPUT 1
#define PS2_STATUS_INPUT (1 << 1)

// commands
#define PS2_READ_CONFIG 0x20
#define PS2_WRITE_CONFIG PS2_DATA
#define PS2_DISABLE_PORT1 0xAD
#define PS2_DISABLE_PORT2 0xA7

// keyboard commands
#define KB_LED 0xED                             // (bit) 0: scroll lock, 1: num lock, 2: caps lock
#define KB_ECHO 0xEE                            // sends keyboard an echo
#define KB_SCAN 0xF0                            // (value) 0: get curr code, 1: set code 1, 2: set code 2, 3: set code 3
#define KB_REPEAT_RATE 0xF3                     // (bit) 0-4: Repeat rate (00000b = 30 Hz, ..., 11111b = 2 Hz) 
                                                //       5-6: Delay before keys repeat (00b = 250 ms, 01b = 500 ms, 10b = 750 ms, 11b = 1000 ms) 
                                                //       7: must be 0
#define KB_ENABLE 0xF4                          // Enable scanning (keyboard will send scan codes) 
#define KB_DISABLE 0xF5                          // disable scanning
#define KB_SET_DEFAULT 0xF6                     // set default params
#define KB_SET_TYPEM_AUTOR 0xF7                 // Set all keys to typematic/autorepeat only (scancode set 3 only) 
#define KB_SET_MAKE_RELEASE 0xF8                // Set all keys to make/release (scancode set 3 only) 
#define KB_SET_MAKE 0xF9                        // Set all keys to make only (scancode set 3 only) 
#define KB_SET_TYPEM_AUTOR_MAKE_RELEASE 0xFA    // Set all keys to typematic/autorepeat/make/release (scancode set 3 only) 
#define KB_SET_KEY_TYPEM_AUTOR 0xFB             // requires scancode for key, Set specific key to typematic/autorepeat only (scancode set 3 only) 
#define KB_SET_KEY_MAKE_RELEASE 0xFC            // requires scancode for key, Set specific key to make/release (scancode set 3 only) 
#define KB_SET_KEY_MAKE 0xFD                    // requires scancode for key, Set specific key to make only (scancode set 3 only)
#define KB_RESEND_BYTE 0xFE                     // keyboard resends last byte
#define KB_RESET 0xFF                           // keyboard resets and starts self test

// keyboard response
#define KB_ERR1 0x00            // Key detection error or internal buffer overrun
#define KB_ERR2 0xFF
#define KB_TEST_PASS 0xAA  // Self test passed (sent after "0xFF (reset)" command or keyboard power up)
#define KB_ECHO_PASS KB_ECHO    // echo recieved from keyboard
#define KB_ACK 0xFA             // command acknowledgement
#define KB_TEST_FAIL1 0xFC      // self test failed
#define KB_TEST_FAIL2 0xFD
#define KB_RESEND_CMD 0xFE      // keyboard wants the last command to be resent

void ps2_init(void);
int kb_init(void);

// scancodes



#endif