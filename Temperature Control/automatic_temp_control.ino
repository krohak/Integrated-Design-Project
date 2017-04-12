/*************************************************** 
  This is an example for the HTU21D-F Humidity & Temp Sensor

  Designed specifically to work with the HTU21D-F sensor from Adafruit
  ----> https://www.adafruit.com/products/1899

  These displays use I2C to communicate, 2 pins are required to  
  interface
 ****************************************************/

#include <Wire.h>
#include "Adafruit_HTU21DF.h"
#define temp_control 5
#define temp_show 6
#define PWM_MAX 25500
#define PWM_MIN 0
#define T2_COMPARE  156

int int_adc0_ref = 3700;
double e_k0 = 0, e_k1 = 0, e_k2=0;  
double hysBand = 0.005;        // Hysteresis band
double PWM_Power = 0;
float consKp = 32;          // proportional term 
float consKi = 30;           // integral term
float consKd = 30;           // derivative term
boolean flag = 0;
int outputValue = 0;
float temperature=0;

// Connect Vin to 3-5VDC
// Connect GND to ground
// Connect SCL to I2C clock pin (A5 on UNO)
// Connect SDA to I2C data pin (A4 on UNO)

Adafruit_HTU21DF htu = Adafruit_HTU21DF();

void setup() {
  Serial.begin(9600);
  Serial.print("HTU21D-F test");
  pinMode(temp_control, OUTPUT);
  pinMode(temp_show, OUTPUT);
  analogWrite(temp_control, 0);
  if (!htu.begin()) {
    Serial.println("Couldn't find sensor!");
    while (1);
  }
  
    //prepare Timer 2 interupt
/*  noInterrupts();
    Serial.println("test");
  TCCR2A = 0;
    Serial.println("test");
  TCCR2B = 0;
  TCNT2 = 0;

  OCR2A = T2_COMPARE;      // Sampling time (Ts) = (1/ f_clk) x prescaler x (1+OCR2A)
  TCCR2A |= (1 << WGM21);  //CTC mode
  TCCR2B |= (1 << CS22);
  TCCR2B |= (1 << CS21);   //prescaler =1024
  TCCR2B |= (1 << CS20);
  TIMSK2 |= (1 << OCIE2A); //compare match A interrupt enable
  interrupts();*/
}

ISR(TIMER2_COMPA_vect)     //Timer 2 interrupt service routine T2ISR
{
  controller_PI();          //enable to change the LED brightness
}


void loop() {
  temperature = htu.readTemperature();
  //Serial.print("Temp: "); 
  Serial.println(temperature);
  outputValue = map(temperature*100, 2200, 4247, 0, 25500); 
  analogWrite(temp_show, outputValue/100);
  //Serial.print("\t\tshow: "); Serial.print(outputValue/100);
  //Serial.println();
  delay(100);
  controller_PI();
}

void controller_PI()
{
  
  analogWrite(temp_control, PWM_Power/100);
  e_k0 = int_adc0_ref - (htu.readTemperature() * 100);     // Calculate present error
  
        //Serial.println(e_k1);
  if(abs(e_k0)>hysBand){
    PWM_Power = PWM_Power + (consKp * (e_k0 - e_k1)) + (consKp * consKi * e_k0) + ((consKp * consKd) *(e_k0+e_k2-2*e_k1));
    
    e_k2=e_k1;
    e_k1 = e_k0;                         // Save previous errors
    
   // Serial.println(PWM_Power);
    limiter();
    
   }
         //Serial.print("\t\t"); Serial.println(int(PWM_Power/100));
}

void limiter()
{
    if (PWM_Power > PWM_MAX)    // Limit maximum PWM
  {
    PWM_Power = PWM_MAX;
  }
  if (PWM_Power < PWM_MIN)   // Limit minimum PWM
  {
    PWM_Power = PWM_MIN;
  }
}
