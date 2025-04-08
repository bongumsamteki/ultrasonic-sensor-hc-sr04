
#define REGISTER_BASE      0x20 // arduino uno r3 base register

#define PRR_VAL            (volatile unsigned int *)(0x64) // power reduction register

#define SPCR_OFFSET        0x2C // SPI control register offset
#define SPCR_VAL           (volatile unsigned int *)(REGISTER_BASE + SPCR_OFFSET) // SPI control register

#define DDRB_OFFSET        0x04 // port b data direction register offset
#define DDRB_VAL           (volatile unsigned int *)(REGISTER_BASE + DDRB_OFFSET) // port b data direction register

#define PORTB_OFFSET       0x05
#define PORTB_VAL          (volatile unsigned int *)(REGISTER_BASE + PORTB_OFFSET) // port b data register

#define TCCR1B_VAL         (volatile unsigned int *)0x81 // timer 1 control register 1

#define ICR1H_VAL          (volatile unsigned int *)0x87
#define ICR1L_VAL          (volatile unsigned int *)0x86

#define TCCR1A_VAL         (volatile unsigned int *)0x80 // control register A

#define TCNT1H_VAL         (volatile unsigned int *)0x85 // timer val high
#define TCNT1L_VAL         (volatile unsigned int *)0x84 // timer val low

#define TIFR1_VAL          (volatile unsigned int *)0x36 // timer interrput flag register

#define TIMSK1_VAL         (volatile unsigned int *)0x6F // interrupt mask register

uint16_t timer_val;
uint8_t  get_prox;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(9, OUTPUT);
  pinMode(10, INPUT);
  
  *(DDRB_VAL) |= (1U << 1); // pin 9 as output for trigger

  *(TCCR1B_VAL) |=  (1U << 7);   // enable counter noise filter
  *(TCCR1B_VAL) &=~ (1U << 6);   // capture on rising edge

  *(TCCR1B_VAL) &=~ (1U << 0);// set prescaler to 8
  *(TCCR1B_VAL) |=  (1U << 1); 
  *(TCCR1B_VAL) &=~ (1U << 2);      

  *(TCCR1A_VAL) &=~ (1U << 0); // set mode to normal
  *(TCCR1A_VAL) &=~ (1U << 1);
  *(TCCR1B_VAL) &=~ (1U << 3);
  *(TCCR1B_VAL) &=~ (1U << 4);

  *(TCNT1H_VAL) = 0; // set timer count value to 0;
  *(TCNT1L_VAL) = 0;

  get_prox = 0;

}

void loop() {
  // put your main code here, to run repeatedly:
  
  while(!(*(TIFR1_VAL) & (1U << 0)) || get_prox); // is top reached and proximity measured?
  {
    *(PORTB_VAL) |=  (1U << 1);
    delay(1);
    *(PORTB_VAL) &=~ (1U << 1);
    
    if((*(TIFR1_VAL) & (1U << 5))) // input captured
    {
      timer_val = ((*(ICR1H_VAL) << 8) | *(ICR1L_VAL));
      *(TIFR1_VAL) |= (1U << 5); // clear input captured flag
      get_prox = 1;
    }
  }
  
  Serial.print((calc_distance(timer_val)));
  Serial.println(" cm");
  
  *(TIFR1_VAL) |= (1U << 0); // clear overflow flag

  get_prox = 0; // reset proximity val
  *(ICR1H_VAL) = 0;
  *(ICR1L_VAL) = 0;
  *(TCNT1H_VAL) = 0; // set timer count value to 0;
  *(TCNT1L_VAL) = 0;
}

double calc_distance(uint16_t timer_val)
{
  double distance =  ((((double)timer_val)/(2000000))*343)/2;
  return (distance*100); // return distance in cm
}
