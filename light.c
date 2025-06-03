// I/O Registers definitions
#include <mega328p.h>

// Declare your global variables here
volatile int counter = 0;
volatile int inputPeriod = 100;
// Standard Input/Output functions
#include <stdio.h>

// Timer 0 output compare A interrupt service routine
interrupt [TIM0_COMPA] void timer0_compa_isr(void)
{
    // Place your code here
    counter++;                                           
    
    if(counter >= inputPeriod) {
        PORTB ^= (1 << 0); /// toggle
        counter = 0;       
    }
}

void main(void)
{
    DDRB |= (1 << 0);
    
    // Timer/Counter 0 initialization
    // Clock source: System Clock
    // Clock value: 250.000 kHz
    // Mode: CTC top=OCR0A
    // OC0A output: Disconnected
    // OC0B output: Disconnected
    // Timer Period: 1 ms
    TCCR0A=(0<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (1<<WGM01) | (0<<WGM00);
    TCCR0B=(0<<WGM02) | (0<<CS02) | (1<<CS01) | (1<<CS00);
    TCNT0=0x00;
    OCR0A=0xF9;
    OCR0B=0x00;


    // Timer/Counter 0 Interrupt(s) initialization
    TIMSK0=(0<<OCIE0B) | (1<<OCIE0A) | (0<<TOIE0);


    // USART initialization
    // Communication Parameters: 8 Data, 1 Stop, No Parity
    // USART Receiver: On
    // USART Transmitter: On
    // USART Mode: Asynchronous
    // USART Baud Rate: 9600
    UCSR0A=(0<<RXC0) | (0<<TXC0) | (0<<UDRE0) | (0<<FE0) | (0<<DOR0) | (0<<UPE0) | (0<<U2X0) | (0<<MPCM0);
    UCSR0B=(0<<RXCIE0) | (0<<TXCIE0) | (0<<UDRIE0) | (1<<RXEN0) | (1<<TXEN0) | (0<<UCSZ02) | (0<<RXB80) | (0<<TXB80);
    UCSR0C=(0<<UMSEL01) | (0<<UMSEL00) | (0<<UPM01) | (0<<UPM00) | (0<<USBS0) | (1<<UCSZ01) | (1<<UCSZ00) | (0<<UCPOL0);
    UBRR0H=0x00;
    UBRR0L=0x67;

    // Globally enable interrupts
    #asm("sei")         
    
    printf("Enter the number between 1 and 4 for PWM period : "); 
    scanf("%d", &inputPeriod);
    counter = 0;
    
    printf("Your input peroid is %d.", inputPeriod);
    
    while (1) {
               
    }
}