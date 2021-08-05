#include <xc.h>
#include <stdint.h>
#include <string.h>
#include "can.h"

can_msg_t rx_msg;
can_msg_t txq[CAN_TXQ_LEN];
volatile uint8_t txq_head, txq_tail;

//setup CAN peripheral of MCU
void can_init() {
    //RC6 is CANRX
    ANSELCbits.ANSELC6 = 0; //disable analog
    CANRXPPS = 0b010110; //connect RC6 to CANRX signal


    //RC7 is CANTX
    ANSELCbits.ANSELC7 = 0; //disable analog
    TRISCbits.TRISC7 = 0; //CANTX is an output pin
    LATCbits.LATC7 = 1; //default TX to HIGH when CAN peripheral is not using it
    //this corresponds to sending a recessive bit on the bus
    RC7PPS = 0b110011; //connect RC7 to CANTX signal
    
    //CAN peripheral configuration
    CANCONbits.REQOP = 0b100; //request to enter configuration mode
    while(CANSTATbits.OPMODE != 0b100); //wait for config mode
    //use legacy mode, mode 0 (the default) - it's simpler and still works
    //modes 1 and 2 provide more ID filters and message buffers
    //but we don't need them
    
    BRGCON3bits.WAKDIS = 1; //disable wakeup on CAN activity
    
    //CAN bit timing stuff
    //1Mb/s -> bit time is 1us
    //8x 0.125us time quanta (TQ) per bit time
    BRGCON1bits.BRP = 0b000011; //(3) time quanta TQ = 2*(3+1)/64MHz = 1/8 us
    
    //fixed default sync segment = 1 TQ
    //default propagation segment = 1 TQ
    BRGCON2bits.SEG1PH = 0b011; //Phase Seg 1 = 4 TQ
    BRGCON2bits.SEG2PHTS = 1; //allow programming Phase Seg 2
    BRGCON3bits.SEG2PH = 0b001; //Phase Seg 2 = 2 TQ
    
    //configure receive buffer 0 to ignore filters and accept all messages
    RXB0CONbits.RXM0 = 1; 
    RXB0CONbits.RXM1 = 1;
    //RXB1 will never receive a message - filters block them all by default
//    RXB0CONbits.RB0DBEN = 1; //if we receive a message and RXB0 is full, 
                    //put it in RXB1. This ignores RXB1 filters by default
    
    CANCONbits.REQOP = 0b000; //request Normal Mode
    while(CANSTATbits.OPMODE != 0b000); //wait mode change
    
    //enable interrupts for message rx and tx completion
    PIE5bits.RXB0IE=1;
    PIE5bits.TXB0IE=1;
}

uint8_t can_txq_push(const uint16_t id, const uint8_t len, const uint8_t *data) {
    PIE5bits.TXB0IE=0; //disable TX interrupt to avoid race conditions during
                        //head,tail,buffer access/modify
    if(txq_head == txq_tail - 1) {
        return 0; //failed to queue transmit - buffer was full
    }
    txq[txq_head].id=id; //populate struct at head of queue with msg to send
    txq[txq_head].len=len;
    memcpy(txq[txq_head].data, data, len);
    //increment head, wrapping back to 0 at array end
    txq_head++;
    if(txq_head >= CAN_TXQ_LEN) {
        txq_head = 0;
    }
    PIE5bits.TXB0IE=1; //re-enable interrupt
    
    //TXREQ is set at the time a message transmit is requested
    //and cleared at the time when the transmit is completed.
    //At transmit complete, the TXB0IF fires the interrupt.
    //initially, when there's no transmission started, the interrupt never fires
    if(PIR5bits.TXB0IF == 0 && TXB0CONbits.TXREQ == 0) {
        PIR5bits.TXB0IF = 1;
        //interrupt fires here
    }
    
    return 1; //succesfully transmitted message
}

void can_tx(const can_msg_t *msg) {
//    //if TXB0 already has a message, don't try to send
//    if (TXB0CONbits.TXREQ != 0) {
//        return;
//    }

    //copy ID, length, and data bytes from msg struct to transmit buffer registers.
    TXB0SIDH = (uint8_t)((msg->id) >> 3);
    TXB0SIDL = (uint8_t)(((msg->id) & 0b111) << 5);
    TXB0SIDLbits.EXIDE = 0; //send as standard (11-bit) ID frame

    // not an RTR message, we don't support those
    TXB0DLCbits.TXRTR = 0;
    // set message length
    TXB0DLCbits.DLC = msg->len;
    //copy data bytes
    memcpy((void*) &TXB0D0, (const void*)msg->data, msg->len);

    // Mark transmit buffer 0 ready to transmit and the hardware does the rest
    TXB0CONbits.TXREQ = 1;
}

//run when message rxed and transferred to RXB0
void __interrupt(irq(IRQ_RXB0IF)) RXB0_int() {
    PIR5bits.RXB0IF = 0; //clear flag immediately
    //RXB0SIDH contains the 8 MSbs of the SID
    //RXB0SIDL contains the 3 LSbs of the SID in its 3 MSbs
    //combine them to give a right-justified 11-bit SID
    rx_msg.id = (((uint16_t)RXB0SIDH) << 3) | (RXB0SIDL >> 5);
    rx_msg.len = RXB0DLCbits.DLC;
    //simple mem copy works because each data array is in a contiguous memory space
    memcpy((void *)rx_msg.data, (const void *)&RXB0D0, rx_msg.len);

    // call application code callback
    if (can_rx_callback != NULL) {
        can_rx_callback(&rx_msg);
    }

    RXB0CONbits.RXFUL = 0; //clear RX buffer full flag, allowing new message reception
}
//
////ditto for RXB1
//void __interrupt(irq(IRQ_RXB1IF)) RXB1_int() {
//    
//}

void __interrupt(irq(IRQ_TXB0IF)) TXB0_int() {
    PIR5bits.TXB0IF = 0; //clear interrupt flag
    if(txq_tail == txq_head) {
        //empty buffer
        return;
    }
    can_tx(&txq[txq_tail]); //request transmission of message at index tail
    //increment tail, wrapping at array end
    txq_tail++;
    if(txq_tail >= CAN_TXQ_LEN) {
        txq_tail = 0;
    }
}