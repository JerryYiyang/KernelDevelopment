#include "serial.h"
#include "interrupts.h"
#include "printk.h"

static struct UART_State state;

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0"
                    : "=a"(ret)
                    : "Nd"(port) );
    return ret;
}

static inline int are_interrupts_enabled() {
    unsigned long flags;
    __asm__ volatile("pushf\n\t"
                     "pop %0"
                     : "=r"(flags));
    return flags & 0x200; // Check IF (Interrupt Flag) bit
}

void SER_init(void) {
    state.consumer = state.buff;
    state.producer = state.buff;
    state.tx_busy = 0;
    state.port_base = COM1;
    state.initialized = 0;
    
    // Disable all interrupts first
    outb(state.port_base + COM_INT_ENABLE_REG_OFFSET, 0x00);
    
    // Enable DLAB to set baud rate
    outb(state.port_base + COM_LINE_CTL_REG_OFFSET, COM_LINE_CTL_DLAB);
    
    // Set divisor for 115200 baud
    outb(state.port_base + COM_DIV_LATCH_LOW_OFFSET, COM_BAUD_DIVISOR_115200 & 0xFF); // low byte
    outb(state.port_base + COM_DIV_LATCH_HIGH_OFFSET, (COM_BAUD_DIVISOR_115200 >> 8) & 0xFF); // high byte
    
    // 8 bits, no parity, one stop bit and disable DLAB
    outb(state.port_base + COM_LINE_CTL_REG_OFFSET, 
         COM_LINE_CTL_DATA_BITS_8 | COM_LINE_CTL_STOP_BITS_1 | COM_LINE_CTL_PARITY_NONE);
    
    // Enable FIFO, clear them, with 14-byte threshold
    outb(state.port_base + COM_FIFO_CTL_REG_OFFSET, 
         COM_FIFO_CTL_ENABLE | COM_FIFO_CTL_CLEAR_TRANSMIT | COM_FIFO_CTL_TRIGGER_LEVEL_14);
    
    // Enable IRQs, RTS/DSR set
    outb(state.port_base + COM_MODEM_CTL_REG_OFFSET, 
         COM_MODEM_CTL_DTR | COM_MODEM_CTL_RTS | COM_MODEM_CTL_AUX_OUT2);
    
    // Set in loopback mode to test the chip
    outb(state.port_base + COM_MODEM_CTL_REG_OFFSET, 
         COM_MODEM_CTL_DTR | COM_MODEM_CTL_RTS | COM_MODEM_CTL_AUX_OUT2 | COM_MODEM_CTL_LOOPBACK);
    
    // Test serial chip (send byte 0xAE and check if serial returns same byte)
    outb(state.port_base + COM_TRANSMIT_OFFSET, 0xAE);
    
    // Check if serial is faulty (i.e., not same byte as sent)
    if (inb(state.port_base + COM_RECIEVE_OFFSET) != 0xAE) {
        // serial is faulty - handle error
        printk("Serial port at 0x%x is faulty\n", state.port_base);
        return;
    }
    
    // If serial is not faulty, set it in normal operation mode
    outb(state.port_base + COM_MODEM_CTL_REG_OFFSET, 
         COM_MODEM_CTL_DTR | COM_MODEM_CTL_RTS | COM_MODEM_CTL_AUX_OUT2);
    
    // enable transmitter empty and line status interrupts
    outb(state.port_base + COM_INT_ENABLE_REG_OFFSET, COM_INT_ENABLE_TRANSMITTER_EMPTY | COM_INT_ENABLE_LINE_STATUS);
    IRQ_clear_mask(4);
    state.initialized = 1;
}

static void hardware_write(void) {
    if (state.tx_busy == 0 && state.count > 0) {
        state.tx_busy = 1;
        char next_byte = *state.consumer;
        state.consumer++;
        if (state.consumer >= &state.buff[BUFF_SIZE]) 
            state.consumer = &state.buff[0];
        state.count--;
        outb(state.port_base + COM_TRANSMIT_OFFSET, next_byte);
    }
}

int SER_write(const char *buff, int len) {
    int enable_ints = 0;
    
    if (are_interrupts_enabled()) {
        enable_ints = 1;
        __asm__ volatile("cli");
    }

    int bytes_free = BUFF_SIZE - state.count;
    int bytes_to_copy = (bytes_free >= len) ? len : bytes_free;

    // adding to buff
    for (int i = 0; i < bytes_to_copy; i++) {
        *state.producer = buff[i];
        state.producer++;
        if (state.producer >= &state.buff[BUFF_SIZE]) state.producer = &state.buff[0];
        state.count++;
    }
    hardware_write();

    if (enable_ints) {
        __asm__ volatile("sti");
    }
    
    return bytes_to_copy;
}

void serial_interrupt_handler(int irq, int error_code, void* arg) {
    (void)irq;
    (void)error_code;
    (void)arg;
    
    // read the interrupt identification register
    uint8_t iir = inb(state.port_base + COM_INT_IDENT_REG_OFFSET);
    
    // check if an interrupt is pending (bit 0 is 0 when an interrupt is pending)
    if (iir & COM_INT_IDENT_PENDING) {
        return; // no interrupt pending
    }
    
    // extract the interrupt source from bits 1-3
    uint8_t interrupt_source = iir & COM_INT_IDENT_MASK;
    
    switch (interrupt_source) {
        case COM_INT_IDENT_LINE_STATUS: {
            // handle line status interrupt (read LSR to clear the interrupt
            uint8_t lsr = inb(state.port_base + COM_LINE_STATUS_OFFSET);
            
            // check errors
            if (lsr & (COM_LINE_STATUS_OVERRUN_ERROR | 
                      COM_LINE_STATUS_PARITY_ERROR | 
                      COM_LINE_STATUS_FRAMING_ERROR | 
                      COM_LINE_STATUS_BREAK_INDICATOR)) {
                printk("Serial line error: 0x%x\n", lsr);
            }
            break;
        }
        case COM_INT_IDENT_TRANSMITTER_EMPTY: {
            state.tx_busy = 0;
            hardware_write();
            break;
        }
        default:
            // clear non-TX interrupts
            inb(state.port_base + COM_RECIEVE_OFFSET);
            break;
    }
    if (interrupt_source == COM_INT_IDENT_TRANSMITTER_EMPTY) {
        state.tx_busy = 0;
        hardware_write();
    }
}