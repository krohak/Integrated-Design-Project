/*****************************************************
  IDP Lab 2 autolight
  Usage of ADC, PWM, feedback and control

*****************************************************/
#include <SPI.h>       // this is needed for display
#include <SD.h>
#include <Wire.h>    //I2C for touch
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h> // TFT by SPI library
#include <Adafruit_FT6206.h>  //touch
#include "Adafruit_HTU21DF.h"


// Define constant
#define temp_control 5

#define LED  6
//#define
//#define
#define T2_COMPARE  156     // value of the timer 2 output compare register OCR2A
                            // Sampling time (Ts) = (1/ f_clk) x prescaler x (1+OCR2A)
//Define type of controller
#define control 2          // control = 1 when select proportional controller
                           // control = 2 when select proportional-integral compensator

// Declare variable
int temp=24;
float fanspeed;
float tempCC;

int PWM_MAX=25500;
int PWM_MIN=0;

int int_adc0_ref;
int int_adc0 = 0;

int int_PWMLED = 0;
 int num;

char kc='o';

int consKp = 10;          // proportional term
int consKi = 5;
/*float consKp = 32;          // proportional term
float consKi = 30;           // integral term
float consKd = 30;*/           // integral term
int hysBand = 10;        // Hysteresis band
double e_k0 = 0, e_k1 = 0, e_k2=0;

/***********For the fan PID****************/
int temp_ref = 2600;
double fe_k0 = 0, fe_k1 = 0, fe_k2=0;
double fhysBand = 0.005;
double PWM_Power = 0;
float fconsKp = 10;          // proportional term 
float fconsKi = 5;           // integral term
float fconsKd = 3; 

long previousMillis = 0;
long interval = 1000; 

Adafruit_HTU21DF htu = Adafruit_HTU21DF();

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);   // Setup serial communication 115200
  pinMode(LED, OUTPUT);   // Set LED pin as an output
  analogWrite(LED, 0);    // Set initial LED as O (duty cycle = 0)
  int_adc0_ref=analogRead(A0);

  //prepare Timer 2 interupt
  noInterrupts();
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;

  OCR2A = T2_COMPARE;      // Sampling time (Ts) = (1/ f_clk) x prescaler x (1+OCR2A)
  TCCR2A |= (1 << WGM21);  //CTC mode
  TCCR2B |= (1 << CS22);
  TCCR2B |= (1 << CS21);   //prescaler =1024
  TCCR2B |= (1 << CS20);
  TIMSK2 |= (1 << OCIE2A); //compare match A interrupt enable
  interrupts();

 // Serial.println("HTU21D-F test");
pinMode(temp_control, OUTPUT);
 // pinMode(tempC,INPUT);
  digitalWrite(temp_control,LOW);
  if (!htu.begin()) {
    //Serial.println("Couldn't find sensor!");
    while (1);
  }
   //Serial.println("HKU EEE IDP Lab 1");

}


ISR(TIMER2_COMPA_vect)     //Timer 2 interrupt service routine T2ISR
{
  controller();  //enable to change the LED brightness
   
}


void loop()
{
  
  
   
   
   
   

 if (Serial.available() > 0) {
   

   kc = Serial.read();

   if (kc=='n'){
  
     // noInterrupts(); 
 
       num= Serial.parseInt();

     
     PWM_MAX=map(num,0,100,0,255)*100;
     
      //Serial.print(kc);
      //Serial.print('\t');
      //Serial.println(num);
     
     kc='o';
     
     //interrupts();

     }
     
   if (kc=='t'){
  
     // noInterrupts(); 
     float flt;
 
       flt= Serial.parseFloat();

     
     temp_ref=flt*100;
     
      //Serial.print(kc);
      //Serial.print('\t');
      //Serial.println(num);
     
     kc='o';
     
     //interrupts();

     }
     
     
     
 }
    
  //Serial.println(int_PWMLED);


 
 
 
 
//else if (kc=='f'){//Serial.println("f");


//} 


//
 
// if(currentMillis - previousMillis > interval) {
 if(kc=='s') {
 
 /*Serial.print(htu.readTemperature());
 Serial.print("\t\t");
  Serial.println(PWM_Power);
  Serial.println(htu.readHumidity());*/
 
  Serial.print("{\"current\":");
  Serial.print(int_PWMLED);
  Serial.print(",\"max\":");
  Serial.print(PWM_MAX);
   Serial.print(",\"temp\":");
  Serial.print(tempCC);
  Serial.print(",\"hum\":");
  Serial.print(htu.readHumidity());
  Serial.print(",\"targettemp\":");
  Serial.print(temp_ref);
  Serial.print(",\"fanspeed\":");
  Serial.print(PWM_Power);
   Serial.println("}");
  //{"lamp":{"current":"PWM_led","max":"(PWM_MAX)"},"weather":{"temp":"","hum":"","targettemp":"","fanspeed":""}}
  
  //previousMillis = currentMillis;
 kc='o'; 
 }

  adc_task();

  delay(5);

}

void adc_task()  //read ADC and output to PWMLED and Fan speed control
{
  tempCC=htu.readTemperature();
  analogWrite(temp_control, PWM_Power/100);
  fe_k0 =( tempCC* 100)- temp_ref;     // Calculate present error
  
  if(abs(fe_k0)>fhysBand){
    PWM_Power = PWM_Power + (fconsKp * (fe_k0 - fe_k1)) + (fconsKp * fconsKi * fe_k0) + ((fconsKp * fconsKd) *(fe_k0+fe_k2-2*fe_k1));
    
    fe_k2=fe_k1;
    fe_k1 = fe_k0;                         // Save previous errors
    
   
   
     if (PWM_Power > 25500)    // Limit maximum PWM
  {
    PWM_Power = 25500;
  }
  if (PWM_Power < 0)   // Limit minimum PWM
  {
    PWM_Power = 0;
  }
   
   
   
    
   }
   

   
}


void controller()
{
  // put your main code here, to run repeatedly:
 

  //if (kc!='f'){

    //Serial.println("on");
    

  analogWrite(LED, int_PWMLED/100);
  //Serial.println(int_PWMLED);
  int_adc0 = analogRead(A0); // Read the light intensity from the photo-resistor
  e_k1 = int_adc0_ref - int_adc0;     // Calculate present error

  if(abs(e_k1)>hysBand)
  {
  switch (control)
  {
  case 1: //  Call function controller_P
  controller_P();
    break;
  case 2: //  Call function controller_PI
  controller_PI();
  break;
  }

  }



  //}


  /*else if (kc=='f'){

    //Serial.print(kc);
    //  Serial.println("off");
    int_PWMLED=0;
     analogWrite(LED, int_PWMLED);


  }*/
}


void controller_P()
{
  // digital P controller. consKp represents propertional gain Kp
  int_PWMLED = consKp * e_k1;
  limiter();
}

void controller_PI()
{
  // digital PI controller controller, consKp represents propertional gain Kp, consKi represents the ratio of sanpling time (Ts) / integral time(Ti)
  int_PWMLED = int_PWMLED + (consKp * (e_k1 - e_k0)) + (consKp * consKi * e_k1);
  e_k0 = e_k1;  // Save previous error
 // int_PWMLED = int_PWMLED  + (consKp * (e_k0 - e_k1)) + (consKp * consKi * e_k0) + ((consKp * consKd) *(e_k0+e_k2-2*e_k1));

    //e_k2=e_k1;
    //e_k1 = e_k0;
  limiter();
}

void limiter()
{
    if (int_PWMLED > PWM_MAX)    // Limit maximum PWM
  {
    int_PWMLED = PWM_MAX;
  }
  if (int_PWMLED < PWM_MIN)   // Limit minimum PWM
  {
    int_PWMLED = PWM_MIN;
  }
}
