#include <mega328p.h>
#include <delay.h>
#include <stdio.h>

// ADC Voltage Reference: AVCC pin
#define ADC_VREF_TYPE ((1<<REFS1) | (1<<REFS0) | (0<<ADLAR))

// Read the AD conversion result
// Read Voltage=read_adc*(Vref/1024.0)
unsigned int read_adc(unsigned char adc_input) {
  ADMUX=adc_input | ADC_VREF_TYPE;
  // Delay needed for the stabilization of the ADC input voltage
  delay_us(10);
  // Start the AD conversion
  ADCSRA|=(1<<ADSC);
  // Wait for the AD conversion to complete
  while ((ADCSRA & (1<<ADIF))==0);
  ADCSRA|=(1<<ADIF);
  return ADCW;
}

void EEPROM_write(unsigned int uiAddress, unsigned char ucData) {
  /* Wait for completion of previous write */
  while(EECR & (1<<EEPE))
  ;
  /* Set up address and Data Registers */
  EEAR = uiAddress;
  EEDR = ucData;
  /* Write logical one to EEMPE */
  EECR |= (1<<EEMPE);
  /* Start eeprom write by setting EEPE */
  EECR |= (1<<EEPE);
}

unsigned char EEPROM_read(unsigned int uiAddress)
{
  /* Wait for completion of previous write */
  while(EECR & (1<<EEPE))
  ;
  /* Set up address register */
  EEAR = uiAddress;
  /* Start eeprom read by writing EERE */
  EECR |= (1<<EERE);
  /* Return data from Data Register */
  return EEDR;
}

// Declare your global variables here

void main(void) {
  // Declare your local variables here
  int i;
  int input_PIND, k_adc;
  float adc_voltage;
  int currentTemperature;
  int resTemperature;
  int p;                    
  float x = 1.28;
  float y = 83. / 65;
  int add = 33;

  // Clock Oscillator division factor: 1
  #pragma optsize-
  CLKPR=(1<<CLKPCE);
  CLKPR=(0<<CLKPCE) | (0<<CLKPS3) | (0<<CLKPS2) | (0<<CLKPS1) | (0<<CLKPS0);
  #ifdef _OPTIMIZE_SIZE_
  #pragma optsize+
  #endif

  // Port D initialization
  // Function: Bit7=In Bit6=In Bit5=Out Bit4=In Bit3=In Bit2=In Bit1=In Bit0=In 
  DDRD = (1<<DDD5) | (0<<DDD3);
  // State: Bit7=T Bit6=T Bit5=0 Bit4=T Bit3=T Bit2=T Bit1=T Bit0=T 
  PORTD = (0<<PORTD5);


  //8data, 1stop, 9600(占쏙옙占?占쏙옙占?
  UCSR0A=(0<<RXC0) | (0<<TXC0) | (0<<UDRE0) | (0<<FE0) | (0<<DOR0) | (0<<UPE0) | (0<<U2X0) | (0<<MPCM0);
  UCSR0B=(0<<RXCIE0) | (0<<TXCIE0) | (0<<UDRIE0) | (1<<RXEN0) | (1<<TXEN0) | (0<<UCSZ02) | (0<<RXB80) | (0<<TXB80);
  UCSR0C=(0<<UMSEL01) | (0<<UMSEL00) | (0<<UPM01) | (0<<UPM00) | (0<<USBS0) | (1<<UCSZ01) | (1<<UCSZ00) | (0<<UCPOL0);
  UBRR0H=0x00;
  UBRR0L=0x67;

  // Analog Comparator initialization
  // Analog Comparator: On
  // The Analog Comparator's positive input is
  // connected to the AIN0 pin
  // The Analog Comparator's negative input is
  // connected to the AIN1 pin
  // Analog Comparator Input Capture by Timer/Counter 1: Off
  ACSR=(0<<ACD) | (0<<ACBG) | (0<<ACO) | (0<<ACI) | (0<<ACIE) | (0<<ACIC) | (0<<ACIS1) | (0<<ACIS0);
  // Digital input buffer on AIN0: On
  // Digital input buffer on AIN1: On
  DIDR1=(0<<AIN0D) | (0<<AIN1D);

  // ADC initialization
  // ADC Clock frequency: 1000.000 kHz
  // ADC Auto Trigger Source: Software
  ADCSRA=(1<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIF) | (0<<ADIE) | (1<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);
  ADCSRB=(0<<ADTS2) | (0<<ADTS1) | (0<<ADTS0);
  // Digital input buffers on ADC0: On, ADC1: On, ADC2: On, ADC3: On
  // ADC4: On, ADC5: On
  DIDR0=(0<<ADC5D) | (0<<ADC4D) | (0<<ADC3D) | (0<<ADC2D) | (0<<ADC1D) | (0<<ADC0D);

  // SPI initialization
  // SPI disabled
  SPCR=(0<<SPIE) | (0<<SPE) | (0<<DORD) | (0<<MSTR) | (0<<CPOL) | (0<<CPHA) | (0<<SPR1) | (0<<SPR0);

  // TWI initialization
  // TWI disabled
  TWCR=(0<<TWEA) | (0<<TWSTA) | (0<<TWSTO) | (0<<TWEN) | (0<<TWIE);

  
   while (1) {
     // Place your code here


    input_PIND = PIND;
    printf("%d is the input of PORTD\r\n", PIND);

     for(i = 0; i < 5; i++) {
       if (PIND & (1 << 3)) { /// if input is 5V, then blink LED per 100ms.
         PORTD = 0; // LED OFF
         delay_ms(100);
         PORTD = 1 << 5; // LED ON
         delay_ms(100);
       } else { // if input is not 5V, then blink LED per 500ms.
         PORTD = 0; // LED OFF
         delay_ms(500);
         PORTD = 1 << 5; // LED ON
         delay_ms(500);
       }
     }

     k_adc = read_adc(8);
     
     currentTemperature = (int)((k_adc/1024.0) * 1.1 * 1000); 
     
//     if(currentTemperature < 314) {                              
//       p = 314 - 242 
//     } else if(currentTemperature < 380) {
//     } else {
//     }
     
     printf("Voltage is %d\n\r", currentTemperature);
     currentTemperature = currentTemperature - 30;
     
     
     if(currentTemperature > 355) { 
        resTemperature = (int)(25. + (currentTemperature - 352.) * (1.0 / x));
        printf("current temp is %d\n\r", resTemperature);
     } else {
        resTemperature = (int)(-40. + (currentTemperature - 269.) * (1.0 / y));   
        printf("current temp is %d\n\r", resTemperature);
     }
     
     while(resTemperature > 1) {
      EEPROM_write(add++, resTemperature % 10 + '0');
      resTemperature = resTemperature / 10;
     }

      for(int i = 0; i <= 1; i--) {
        printf("EEPROM : %c", EEPROM_read(34 - i));
      }
     
//     void EEPROM_write(unsigned int uiAddress, unsigned char ucData)   
//     void EEPROM_write(unsigned int uiAddress, unsigned char ucData)
   }
  
  return;
}