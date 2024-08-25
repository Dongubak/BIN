#include <mega328p.h>
#include <delay.h>
#include <stdio.h>

int interruptFlag = 0;
interrupt [EXT_INT0] void ext_int0_isr(void) {
  interruptFlag = 1;
  // Place your code here
  printf("detect interrupt\n\r");
  
}
/// 온도 계산의 경우
#define ADC_VREF_TYPE_TEMP ((1<<REFS1) | (1<<REFS0) | (0<<ADLAR))

/*
  count : 깜박이는 횟수, outputPort  : 출력 포트
*/

unsigned int mearsureVoltage(unsigned char adc_input) {
  float adc_voltage; 
  
  adc_input &= 0b00000111;
  ADMUX = (1 << REFS0);
  ADMUX = (ADMUX & 0xF8) | adc_input;
  
  delay_us(10);

  // Start the AD conversion
  ADCSRA|=(1<<ADSC);
  // Wait for the AD conversion to complete
  while ((ADCSRA & (1<<ADSC))==0);
  
  adc_voltage = ((unsigned int)ADCW/1024.0) * 5.0 * 100.0;
  printf("Voltage is %d.%d\n\r", (int)adc_voltage / 100, (int)adc_voltage % 100);
  return adc_voltage;
}

void EEPROM_write(unsigned int uiAddress, unsigned char ucData) {
  /* Wait for completion of previous write */
  while(EECR & (1<<EEPE));
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
  /* Return data from Data Register*/
  return EEDR;
}

int measureTemperature(unsigned char adc_input) {
  int currentTemperatureInVoltage, resTemperature;

  ADMUX=adc_input | ADC_VREF_TYPE_TEMP;
  // Delay needed for the stabilization of the ADC input voltage
  delay_us(10);
  // Start the AD conversion
  ADCSRA|=(1<<ADSC);
  // Wait for the AD conversion to complete
  while ((ADCSRA & (1<<ADIF))==0);
  ADCSRA|=(1<<ADIF);
  
  // Analog Comparator initialization
  // Analog Comparator: Off
  // The Analog Comparator's positive input is
  // connected to the AIN0 pin
  // The Analog Comparator's negative input is
  // connected to the AIN1 pin
  ACSR=(1<<ACD) | (0<<ACBG) | (0<<ACO) | (0<<ACI) | (0<<ACIE) | (0<<ACIC) | (0<<ACIS1) | (0<<ACIS0);
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


  resTemperature = 25 - (352 - currentTemperatureInVoltage) / 1.28;
  printf("current temp is %d\n\r", resTemperature);

  return resTemperature;
}


void blinkLED(const int count, const int outputPort) {
  int i;
  if(interruptFlag == 1) {
    interruptFlag = 0;
    return;
  }
  
  for(i = 0; i < count; i++) {
    PORTD = 0x00;
    delay_ms(100);
    PORTD = 1 << outputPort;
    delay_ms(100); 

    if(interruptFlag == 1) {
      interruptFlag = 0;
      return;
    }
  }
}

int inputMode(int* pt) {
  // printf("---------------------------------------------\n\r");
  printf("-----------------start input----------------\n\n\n\r");
  printf("enter the mode, quit(q) : \n\r");

  while(scanf("%d", pt) == 1) {
    if(*pt > 4 || *pt < 1) {
      printf("enter the mode (1 ~ 4) : \n\r");
      continue;
    }
    printf("\n\n\n--------------end input--------------\n\r");
    // printf("---------------------------------------------\n\r");  
    return 1;
  }
  printf("\n\n\n--------------end input--------------\n\r"); 
  // printf("---------------------------------------------\n\r"); 
  return 0;
}

int someBehavior(const int mode) {
  int count, i;
  if(mode == 1) {
    // printf("---------------------------------------------\n\r");
    printf("--------------start mode1--------------\n\n\n\r");
    printf("enter the count for blinking LED, quit(q) : \n\r");
    if(scanf("%d", &count)) {
      blinkLED(count, 7);
    }
    printf("\n\n\n-------------- end mode1 --------------\n\n\n\r");
    // printf("---------------------------------------------\n\r");
  } else if (mode == 2){
    int measuredTemperature, sum = 0;   
    measureTemperature(8);
    // printf("---------------------------------------------\n\r");
    printf("--------------start mode2--------------\n\n\n\r");
    printf("enter the count for measuring temperature, quit(q) : \n\r");
    if(scanf("%d", &count)) {
      int add = 33;
      EEPROM_write(add++, count);

      for(i = 0; i < count; i++) {
        measuredTemperature = measureTemperature(8);
        sum += measuredTemperature;
        EEPROM_write(add++, (unsigned char)(measuredTemperature));
      }

      printf("average of temperatures : %d\n\r", sum / count);

    }
    printf("\n\n\n-------------- end mode2 --------------\n\n\n\r");
    // printf("---------------------------------------------\n\r");
  } else if (mode == 3) {
    int add = 33;
    int count = (int)EEPROM_read(add++);
    for(i = 0; i < count; i++) {
      printf("temp %d : %d\n\r", i + 1, (int)EEPROM_read(add++));
    }
  } else if (mode == 4) {
    mearsureVoltage(4);
  } else {
    return 0;
  }

  return 1;
}


void main(void) {
  int mode; 
  #asm("sei");
  #pragma optsize-
  CLKPR=(1<<CLKPCE);
  CLKPR=(0<<CLKPCE) | (0<<CLKPS3) | (0<<CLKPS2) | (0<<CLKPS1) | (0<<CLKPS0);
  #ifdef _OPTIMIZE_SIZE_
  #pragma optsize+
  #endif

  DDRD = (1<<DDD7);
  PORTD = (0<<PORTD7);
  UCSR0A=(0<<RXC0) | (0<<TXC0) | (0<<UDRE0) | (0<<FE0) | (0<<DOR0) | (0<<UPE0) | (0<<U2X0) | (0<<MPCM0);
  UCSR0B=(0<<RXCIE0) | (0<<TXCIE0) | (0<<UDRIE0) | (1<<RXEN0) | (1<<TXEN0) | (0<<UCSZ02) | (0<<RXB80) | (0<<TXB80);
  UCSR0C=(0<<UMSEL01) | (0<<UMSEL00) | (0<<UPM01) | (0<<UPM00) | (0<<USBS0) | (1<<UCSZ01) | (1<<UCSZ00) | (0<<UCPOL0);
  UBRR0H=0x00;
  UBRR0L=0x67;

  EICRA=(0<<ISC11) | (0<<ISC10) | (0<<ISC01) | (0<<ISC00);
  EIMSK=(0<<INT1) | (1<<INT0);
  EIFR=(1<<INTF1) | (0<<INTF0);
  PCICR=(0<<PCIE2) | (0<<PCIE1) | (0<<PCIE0);

  ADCSRA=(1<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIF) | (0<<ADIE) | (1<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);
  ADCSRB=(0<<ADTS2) | (0<<ADTS1) | (0<<ADTS0); 
  
  ACSR=(0<<ACD) | (0<<ACBG) | (0<<ACO) | (0<<ACI) | (0<<ACIE) | (0<<ACIC) | (0<<ACIS1) | (0<<ACIS0);
  DIDR1=(0<<AIN0D) | (0<<AIN1D);
  ADCSRA=(1<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIF) | (0<<ADIE) | (1<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);
  ADCSRB=(0<<ADTS2) | (0<<ADTS1) | (0<<ADTS0);
  DIDR0=(0<<ADC5D) | (0<<ADC4D) | (0<<ADC3D) | (0<<ADC2D) | (0<<ADC1D) | (0<<ADC0D);

  // Delay needed for the stabilization of the ADC input voltage
  delay_ms(100);
  // Start the AD conversion
  ADCSRA|=(1<<ADSC);
  // Wait for the AD conversion to complete
  while ((ADCSRA & (1<<ADIF))==0);
  ADCSRA|=(1<<ADIF);

  blinkLED(2, 7);

  while (inputMode(&mode)) {
    someBehavior(mode);
  }
  
  return;
}