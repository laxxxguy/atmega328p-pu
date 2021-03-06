// based largely on Atmel's AVR136: Low-Jitter Multi-Channel Software PWM Application Note:
// http://www.atmel.com/dyn/resources/prod_documents/doc8020.pdf

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#define F_CPU 4000000L
#define CHMAX 3 // maximum number of PWM channels
#define PWMDEFAULT 0x00 // default PWM value at start up for all channels

#define RED_CLEAR (pinlevelB &= ~(1 << RED)) // map RED to PB0
#define GREEN_CLEAR (pinlevelB &= ~(1 << GREEN)) // map GREEN to PB1
#define BLUE_CLEAR (pinlevelB &= ~(1 << BLUE)) // map BLUE to PB2

//! Set bits corresponding to pin usage above
#define PORTB_MASK  (1 << PB3)|(1 << PB1)|(1 << PB2)

#define set(x) |= (1<<x) 
#define clr(x) &=~(1<<x) 
#define inv(x) ^=(1<<x)

#define RED PB3
#define GREEN PB1
#define BLUE PB2
#define LED_PORT PORTB
#define LED_DDR DDRB

void delay_ms(uint16_t ms);
void init();
void adc_read(uint8_t ADCchannel);
void adc_init();
char compare[3];
char compbuff[3];


int main() {
  init();
  adc_init();
  while(1) {
	adc_read(0);
	delay_ms(18);
	adc_read(1);
	delay_ms(18);
        adc_read(2);
	delay_ms(18);

  }
}


void delay_ms(uint16_t ms) {
  while (ms) {
    _delay_ms(1);
    ms--;
  }
}

void init(void) {
  // set the direction of the ports
  LED_DDR set(RED);
  LED_DDR set(GREEN);
  LED_DDR set(BLUE);
  
  unsigned char i, pwm;

  pwm = PWMDEFAULT;

  // initialise all channels
  for(i=0 ; i<CHMAX ; i++) {
    compare[i] = pwm;           // set default PWM values
    compbuff[i] = pwm;          // set default PWM values
  }

  TIFR = (1 << TOV0);           // clear interrupt flag
  TIMSK = (1 << TOIE0);         // enable overflow interrupt
  TCCR0 |= (1 << CS00);         // start timer, no prescale

  sei();
}

void adc_init(void){
	//select reference voltage
	//AVCC with external capacitor at AREF pin
	ADMUX|=(0<<REFS0) | (1<<ADLAR);
	//set prescaller and enable ADC
	ADCSRA|=(1<< ADPS0) | (1 << ADPS1) | (1<< ADPS2) | (1<<ADEN)|(1<<ADIE);//enable ADC with dummy conversion

}
volatile char remember = 0;
void adc_read(uint8_t ch)
{
	//remember current ADC channel;
	remember=ch;
	//set ADC channel
	ADMUX=(ADMUX & 0b111000)|ch;
	//Start conversion with Interrupt after conversion
	//enable global interrupts
	sei();
	ADCSRA |= (1<<ADSC)|(1<<ADIE);
}

ISR(ADC_vect)
{
	uint16_t adc_value;
	char valtmp = 0;	
	adc_value = ADCH;   
	/*shift from low level to high level ADC, from 8bit to 10bit*/
        valtmp = adc_value;
	compbuff[remember] = adc_value > 10 ? valtmp:0;
}

ISR (TIMER0_OVF_vect) {
  static unsigned char pinlevelB=PORTB_MASK;
  static unsigned char softcount=0xFF;

  PORTB = pinlevelB;            // update outputs
  
  if(++softcount == 0){         // increment modulo 256 counter and update
                                // the compare values only when counter = 0.
    compare[0] = compbuff[0];   // verbose code for speed
    compare[1] = compbuff[1];
    compare[2] = compbuff[2];

    pinlevelB = PORTB_MASK;     // set all port pins high
  }
  // clear port pin on compare match (executed on next interrupt)
  if(compare[0] == softcount) RED_CLEAR;
  if(compare[1] == softcount) GREEN_CLEAR;
  if(compare[2] == softcount) BLUE_CLEAR;
}

