// I/O Registers definitions
#include <mega328p.h>
#include <stdio.h>

// Global variables
volatile int counter = 0;
volatile int inputPeriod = 200; // 200ms total period
volatile int highTime = 125;     // 25% of 200ms = 50ms
volatile int inputDutyCycle;
volatile int inputPeriod;

// Timer 0 output compare A interrupt service routine
interrupt [TIM0_COMPA] void timer0_compa_isr(void)
{
    counter++;                                           
    
    if(counter == highTime) {
        PORTB &= ~(1 << 0); // Turn OFF (LOW) after 50ms
    } 
    else if(counter >= inputPeriod) {
        PORTB |= (1 << 0);  // Turn ON (HIGH)
        counter = 0;
    }
}

void main(void)
{
    DDRB |= (1 << 0);     // Set PB0 as output
    PORTB |= (1 << 0);    // Start with PB0 HIGH (pulse ON)

    // Timer/Counter 0 initialization
    // Mode: CTC, Clock: clk/64 = 250kHz, Period: 1ms (OCR0A = 0xF9)
    TCCR0A = (1<<WGM01);
    TCCR0B = (1<<CS01) | (1<<CS00); // clk/64
    TCNT0 = 0x00;
    OCR0A = 0xF9;

    // Enable Timer0 Compare A interrupt
    TIMSK0 = (1<<OCIE0A);    
    
    // USART initialization
    // Communication Parameters: 8 Data, 1 Stop, No Parity
    // USART Receiver: Off
    // USART Transmitter: On
    // USART Mode: Asynchronous
    // USART Baud Rate: 9600
    UCSR0A=(0<<RXC0) | (0<<TXC0) | (0<<UDRE0) | (0<<FE0) | (0<<DOR0) | (0<<UPE0) | (0<<U2X0) | (0<<MPCM0);
    UCSR0B=(0<<RXCIE0) | (0<<TXCIE0) | (0<<UDRIE0) | (0<<RXEN0) | (1<<TXEN0) | (0<<UCSZ02) | (0<<RXB80) | (0<<TXB80);
    UCSR0C=(0<<UMSEL01) | (0<<UMSEL00) | (0<<UPM01) | (0<<UPM00) | (0<<USBS0) | (1<<UCSZ01) | (1<<UCSZ00) | (0<<UCPOL0);
    UBRR0H=0x00;
    UBRR0L=0x67;

    // Global interrupt enable
    #asm("sei")
    
    printf("Enter the duty cycle : ");
    scanf("%d", &inputDutyCycle);
    printf("Enter the Period of pulse : ");
    scanf("%d", &inputPeriod);

    while (1) {
        // Main loop does nothing, all handled in ISR
    }
}
