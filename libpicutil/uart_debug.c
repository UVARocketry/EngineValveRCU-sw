#include "uart_debug.h"
#include <xc.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

uint8_t tx_ring_buf[UART_TXBUF_SIZE]; //memory for transmit ring buffer
volatile uint8_t head = 0; //index of the last byte added to buffer for transmit
volatile uint8_t tail = 0; //index of byte about to be transmitted

void uart_init() {
    U1CON0bits.TXEN = 1; //UART transmit enable
    
    //9600b/s baud rate.
    //Fosc (64MHz) / 16 / (x + 1)
    //where x is the value in the baud rate registers
    //gives x=416 for 9600b/s
    //31 for 125kb/s
    
    //125000b/s -> x=32
    U1BRGH = 0;
    U1BRGL = 32;
    
    //RC0 is the UART TX pin
    //RX is not implemented since this is for debug output
    RC0PPS = 0x13; //connect RC0 to UART1TX signal
    //configures pin when peripheral not using it
    TRISCbits.TRISC0 = 0; //enable push-pull output
    LATCbits.LATC0 = 1; //outputs HIGH (resting state of UART)
    
    U1CON1bits.ON = 1; //start the peripheral
}

//transfers the array bytes of length n_bytes to tx_ring_buf
//if the buffer becomes full, the method just returns without sending everything
//this will cause only the first 64 bytes of a long message to transmit, for example
//it will also cause truncated packets to be transmitted if the frequency of packets is too high

void uart_tx(uint8_t *bytes, const size_t n_bytes) {
    PIE3bits.U1TXIE = 0; //disable UART interrupts to prevent race conditions
    for (uint8_t i = 0; i < n_bytes; i++) {
        if (head == tail - 1) { //ring buffer is full (head caught up to tail)
            break; //(still set the interrupt enable after the loop)
        }
        tx_ring_buf[head] = bytes[i];
        head++;
        if (head >= UART_TXBUF_SIZE) { //wrap head index back to buffer start
            head = 0;
        }
    }
    PIE3bits.U1TXIE = 1; //enabling the UART TX interrupt while the tx shift register
    //is empty will cause the interrupt flag to be set immediately, starting the tx process
}

//UART1 transmit interrupt 
//U1TXIF bit is set whenever the U1TXB transmit register is empty because that byte
//has been transferred to the shift registered for transmission
//you can't clear the bit directly - you have to put a byte in U1TXB to do that.
//when there are no more bytes to transmit, disable the interrupt to clear U1TXIF

void __interrupt(irq(IRQ_U1TX)) U1TX_int(void) {
    if (tail == head) { //buffer empty
        PIE3bits.U1TXIE = 0; //clear the interrupt flag, or the interrupt will fire forever.
    } else {
        U1TXB = tx_ring_buf[tail]; //transmit the next byte in the buffer
        tail++;
        if (tail >= UART_TXBUF_SIZE) { //wrap tail index back to buffer start
            tail = 0;
        }
    }
}