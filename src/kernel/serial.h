#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

#define BUFF_SIZE 16

// COM ports
#define COM1 0x3F8 // irq4

// IO port offsets
#define COM1 0x3F8 // irq4
#define COM2 0x2F8 // irq3
#define COM3 0x3E8 // irq4
#define COM4 0x2E8 // irq3
#define COM5 0x5F8
#define COM6 0x4F8
#define COM7 0x5E8
#define COM8 0x4E8
// io port offset
#define COM_RECIEVE_OFFSET 0      // receive buffer, dlab 0
#define COM_TRANSMIT_OFFSET 0     // transmit buffer, dlab 0
#define COM_INT_ENABLE_REG_OFFSET 1 // interrupt enable register, dlab 0
#define COM_DIV_LATCH_LOW_OFFSET 0  // divisor latch low byte, dlab 1
#define COM_DIV_LATCH_HIGH_OFFSET 1 // divisor latch high byte, dlab 1
#define COM_INT_IDENT_REG_OFFSET 2  // interrupt identification register
#define COM_FIFO_CTL_REG_OFFSET 2   // fifo control register
#define COM_LINE_CTL_REG_OFFSET 3   // line control register
#define COM_MODEM_CTL_REG_OFFSET 4  // modem control register
#define COM_LINE_STATUS_OFFSET 5    // line status register
#define COM_MODEM_STATUS_OFFSET 6   // modem status register
#define COM_SCRATCH_REG_OFFSET 7    // scratch register

// interrupt enable register bits
#define COM_INT_ENABLE_RECEIVED_DATA 0x01        // enable received data available interrupt
#define COM_INT_ENABLE_TRANSMITTER_EMPTY 0x02    // enable transmitter holding register empty interrupt
#define COM_INT_ENABLE_LINE_STATUS 0x04          // enable receiver line status interrupt
#define COM_INT_ENABLE_MODEM_STATUS 0x08         // enable modem status interrupt

// interrupt identification register bits
#define COM_INT_IDENT_PENDING 0x01               // 0 = interrupt pending
#define COM_INT_IDENT_MASK 0x0E                  // mask for interrupt source bits
#define COM_INT_IDENT_MODEM_STATUS 0x00          // modem status interrupt
#define COM_INT_IDENT_TRANSMITTER_EMPTY 0x02     // transmitter holding register empty
#define COM_INT_IDENT_RECEIVED_DATA 0x04         // received data available
#define COM_INT_IDENT_LINE_STATUS 0x06           // line status interrupt
#define COM_INT_IDENT_CHAR_TIMEOUT 0x0C          // character timeout (fifo only)
#define COM_INT_IDENT_FIFO_ENABLED 0xC0          // fifos enabled (16550 only)

// fifo control register bits
#define COM_FIFO_CTL_ENABLE 0x01                 // enable fifos
#define COM_FIFO_CTL_CLEAR_RECEIVE 0x02          // clear receive fifo
#define COM_FIFO_CTL_CLEAR_TRANSMIT 0x04         // clear transmit fifo
#define COM_FIFO_CTL_DMA_MODE 0x08               // enable dma mode
#define COM_FIFO_CTL_ENABLE_64 0x20              // enable 64 byte fifo (16750 only)
#define COM_FIFO_CTL_TRIGGER_LEVEL_1 0x00        // trigger level 1 byte
#define COM_FIFO_CTL_TRIGGER_LEVEL_4 0x40        // trigger level 4 bytes
#define COM_FIFO_CTL_TRIGGER_LEVEL_8 0x80        // trigger level 8 bytes
#define COM_FIFO_CTL_TRIGGER_LEVEL_14 0xC0       // trigger level 14 bytes

// line control register bits
#define COM_LINE_CTL_DATA_BITS_5 0x00            // 5 data bits
#define COM_LINE_CTL_DATA_BITS_6 0x01            // 6 data bits
#define COM_LINE_CTL_DATA_BITS_7 0x02            // 7 data bits
#define COM_LINE_CTL_DATA_BITS_8 0x03            // 8 data bits
#define COM_LINE_CTL_STOP_BITS_1 0x00            // 1 stop bit
#define COM_LINE_CTL_STOP_BITS_2 0x04            // 1.5 or 2 stop bits
#define COM_LINE_CTL_PARITY_NONE 0x00            // no parity
#define COM_LINE_CTL_PARITY_ODD 0x08             // odd parity
#define COM_LINE_CTL_PARITY_EVEN 0x18            // even parity
#define COM_LINE_CTL_PARITY_MARK 0x28            // mark parity
#define COM_LINE_CTL_PARITY_SPACE 0x38           // space parity
#define COM_LINE_CTL_BREAK_ENABLE 0x40           // enable break transmission
#define COM_LINE_CTL_DLAB 0x80                   // divisor latch access bit

// modem control register bits
#define COM_MODEM_CTL_DTR 0x01                   // data terminal ready
#define COM_MODEM_CTL_RTS 0x02                   // request to send
#define COM_MODEM_CTL_AUX_OUT1 0x04              // auxiliary output 1
#define COM_MODEM_CTL_AUX_OUT2 0x08              // auxiliary output 2 (used for interrupt enable)
#define COM_MODEM_CTL_LOOPBACK 0x10              // loopback mode

// line status register bits
#define COM_LINE_STATUS_DATA_READY 0x01          // received data ready
#define COM_LINE_STATUS_OVERRUN_ERROR 0x02       // overrun error
#define COM_LINE_STATUS_PARITY_ERROR 0x04        // parity error
#define COM_LINE_STATUS_FRAMING_ERROR 0x08       // framing error
#define COM_LINE_STATUS_BREAK_INDICATOR 0x10     // break indicator
#define COM_LINE_STATUS_THR_EMPTY 0x20           // transmitter holding register empty
#define COM_LINE_STATUS_TRANSMITTER_EMPTY 0x40   // transmitter empty (shift register)
#define COM_LINE_STATUS_FIFO_ERROR 0x80          // error in received fifo

// modem status register bits
#define COM_MODEM_STATUS_DCTS 0x01               // delta clear to send
#define COM_MODEM_STATUS_DDSR 0x02               // delta data set ready
#define COM_MODEM_STATUS_TERI 0x04               // trailing edge ring indicator
#define COM_MODEM_STATUS_DDCD 0x08               // delta data carrier detect
#define COM_MODEM_STATUS_CTS 0x10                // clear to send
#define COM_MODEM_STATUS_DSR 0x20                // data set ready
#define COM_MODEM_STATUS_RI 0x40                 // ring indicator
#define COM_MODEM_STATUS_DCD 0x80                // data carrier detect

// common baud rate divisors
#define COM_BAUD_DIVISOR_115200 1                // 115200 baud
#define COM_BAUD_DIVISOR_57600 2                 // 57600 baud
#define COM_BAUD_DIVISOR_38400 3                 // 38400 baud
#define COM_BAUD_DIVISOR_19200 6                 // 19200 baud
#define COM_BAUD_DIVISOR_9600 12                 // 9600 baud
#define COM_BAUD_DIVISOR_4800 24                 // 4800 baud
#define COM_BAUD_DIVISOR_2400 48                 // 2400 baud
#define COM_BAUD_DIVISOR_1200 96                 // 1200 baud

struct UART_State {
    char buff[BUFF_SIZE];
    char *consumer, *producer;
    int tx_busy, count, initialized;
    uint16_t port_base;
};

extern void SER_init(void);
extern int SER_write(const char *buff, int len);
void serial_interrupt_handler(int irq, int error_code, void* arg);

#endif