## src/kernel

kernel.c: The main build which everything is run on
keyboard_scancodes.h: Contains all the scancodes for set 2
printk.c: All the methods for printk to operate. Works like printf but for printing in the kernel
drivers.c: Contains the methods for the ps2 controller, keyboard, PIC, and interrupts
string.c: Helper functions for basic string operations
vga.c: Contains the methods to display stuff on the kernel with the VGA card