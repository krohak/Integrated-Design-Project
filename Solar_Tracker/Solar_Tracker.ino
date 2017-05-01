/*****************************************************/
#include <Servo.h>

// define input constant name
#define SAMPLING 11
#define PWMOUT 5
#define VOUT_LIMIT 25
#define TON_STEP 5              // STEP update Ton
#define LOOP_TIME 30           // Main loop time = 500 msec
#define TIME_LIMIT 3000         // Stop time: 300*0.5 sec=150 sec
#define SERVO_PIN 9        //servo connected at pin 9
#define TOL   10           //tolerance for adc different, avoid oscillation
#define K   1



// Solar panel variables
float float_i_pv;               //PV current at t=0
float float_i_pv_k0 = 0;        //PV current at t=-1
float float_i_pv_k1 = 0;        //PV current at t=-2

float float_v_pv;               //PV voltage at t=0
float float_v_pv_k0 = 0;        //PV voltage at t=-1
float float_v_pv_k1 = 0;        //PV voltage at t=-2

float float_p_pv;               //power at t=0, t=-1, t=-2
float float_p_pv_k0 = 0;
float float_p_pv_k1 = 0;

//Converter variables
float float_duty;
float float_v_out;
byte byte_Ton;                  //on-time at t=0, t=-1, t=-2
byte byte_Ton_k0 =1;
byte byte_Ton_k1 =0;

int int_slope = 1;            //mppt searching slope

int A1read;

float float_dPdV = 0;
float float_dPdV_k0;

long long_time_limit = 0;       //counter for timer

long previousMillis = 0;
long interval = 300;

/*****************2*********************/

Servo myservo;            //decare servo object

int int_position=90;      //Servo initial position

//variables for light intensity to ADC reading equations
int int_adc4, int_adc4_m, int_adc4_c;
int int_adc5, int_adc5_m, int_adc5_c;
int int_left, int_right;



void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(SAMPLING, OUTPUT);
  pinMode(PWMOUT, OUTPUT);
  digitalWrite(SAMPLING,LOW);
  Serial.println();
  Serial.println("HKU EEE IDP Lab5");
  Serial.println("***** Program setting *****");
  Serial.print("***** Max. Storage Vout = ");
  Serial.print(VOUT_LIMIT);
  Serial.println(" V.*****");
  Serial.print("***** Time out = ");
  Serial.print(LOOP_TIME/100*TIME_LIMIT/10);
  Serial.println(" sec.*****");
  Serial.println("***** Waiting for PV connect *****");

  while(1)
  {
    if ((analogRead(A2)*5.0/1024)>2.5) { break; }   //program start when PV voltage > 3V
  }

  Serial.println("*******************************************");
  Serial.println("***** PV is connected, Program start ******");
  Serial.println("******************************************");

  byte_Ton = 255*0.5;                //Initial PWM dutycycle
  analogWrite(PWMOUT,byte_Ton);



  /******************SERVO*************************/


  myservo.attach(SERVO_PIN);
  myservo.write(int_position);
  Serial.println();

// measure the sensors reading at ambient light intensity
  int_adc4=analogRead(A4);   // Left sensor at ambient light intensity
  int_adc5=analogRead(A5);   // Right sensor at ambient light intensity

  Serial.print("Calibration in progress, cover the sensors with your hand ......");
  delay(10000);        // delay 10000 ms

// measure the sensors reading at zero light intensity
  int_adc4_c=analogRead(A4);   // Left sensor at zero light intensity
  int_adc5_c=analogRead(A5);   // Right sensor at zero light intensity

// calculate the slope of light intensity to ADC reading equations
  int_adc4_m=(int_adc4-int_adc4_c)/100;
  int_adc5_m=(int_adc5-int_adc5_c)/100;

  Serial.println("...... Completed! Remove your hand");
  delay(5000);


}

//-------------------------------------------------------------------------------------------
//------------------                  Main program                 --------------------------
//-------------------------------------------------------------------------------------------
void loop()
{

  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > interval) {

    digitalWrite(SAMPLING,HIGH);
    pv_sen();                       //read PV voltage, current and storage voltage form ADCs
    check_over_v_out();             //check storage voltage
    check_over_time_limit();        //check time
    mppt_tracking();                //perform mppt tracking
    print_para();                   //display information on the serial monitor
    long_time_limit++;
    digitalWrite(SAMPLING,LOW);

    previousMillis = currentMillis;
  }

    int_left=(analogRead(A4)-int_adc4_c)/int_adc4_m;
    int_right=(analogRead(A5)-int_adc5_c)/int_adc5_m;

    Serial.print("Left sensor intensity=");
    Serial.print(int_left);
    Serial.print(";  Right sensor intensity=");
    Serial.print(int_right);
    Serial.print(";  Servo position=");
    Serial.println(int_position);
    tracker();



                                    // time to execute the above codes ~ 3.5 msec
    delay (10);            //delay ms
}

/************************TRACKER***********************/

void tracker()
{
 // If left sensor is brighter than right sensor, decrease servo angle and turn LEFT
  if (int_left>(int_right+TOL))
  {
    if (int_position>0)
    {
      int_position = int_position-K;
      myservo.write(int_position);
    }
  }

 // if right sensor is bright than left sensor, increase servo angle and turn RIGHT
  if (int_left<(int_right-TOL))
  {
    if (int_position<180)
    {
      int_position = int_position+K;
      myservo.write(int_position);
    }
  }
}




//-------------------------------------------------------------------------------------------
//------------------                    functions for solar            ---------------------------
//-------------------------------------------------------------------------------------------

void pv_sen() // read PV voltage, current and storage voltage from ADCs
{
  float_v_pv = analogRead(A2)*5.0/1024;
  float_v_pv = (float_v_pv+2*(float_v_pv_k0))/3;  //averaging to remove noise

  float_i_pv = analogRead(A1)*5.0/1024;           //note. opamp gain = 10
  float_i_pv = (float_i_pv+2*(float_i_pv_k0))/3;  //averaging to remove noise

  float_p_pv = ((float_v_pv*float_i_pv)+2*(1*float_p_pv_k0))/3;  //averaging to remove noise


  float_v_out = analogRead(A0)*5.0/1024;
  float_v_out = float_v_out*(12+220)/12;
}


//-------------------------------------------------------------------------------------------
void check_over_v_out() // check storage voltage
{
  if (float_v_out >= VOUT_LIMIT)
  {
    analogWrite(PWMOUT,0);
    Serial.print("**********   Storage is full   **********");
    digitalWrite(SAMPLING,LOW);
    while(1);
  }
}


//-------------------------------------------------------------------------------------------
void check_over_time_limit() // check overtime limit
{
  if (long_time_limit == TIME_LIMIT)
  {
    analogWrite(PWMOUT,0);
    Serial.println("**********   Time out   **********");
    digitalWrite(SAMPLING,LOW);
    while(1);
  }
}


//-------------------------------------------------------------------------------------------
void mppt_tracking()
{
    float_v_pv_k1 = float_v_pv_k0;
    float_v_pv_k0 = float_v_pv;

    float_i_pv_k1 = float_i_pv_k0;
    float_i_pv_k0 = float_i_pv;

    float_p_pv_k1 = float_p_pv_k0;
    float_p_pv_k0 = float_p_pv;

    byte_Ton_k1 = byte_Ton_k0;
    byte_Ton_k0 = byte_Ton;

//  calculate dP/dV to obtain the searching slope
    float_dPdV = (float_p_pv-float_p_pv_k1)/(float_v_pv-float_v_pv_k1);
    float_dPdV = (float_dPdV+(2*float_dPdV_k0))/3;  //averaging to remove noise


 if (float_dPdV < 0)
  {
    byte_Ton = byte_Ton+(TON_STEP);
    int_slope = -1;
  }
  else
  {
    byte_Ton = byte_Ton-(TON_STEP);
    int_slope = 1;
  }


// PWM Generator D_min=0.05, D_max=0.95
  if(byte_Ton < 255*0.05) {byte_Ton = 255*0.05;}
  if(byte_Ton > 255*0.95) {byte_Ton = 255*0.95;}
  analogWrite(PWMOUT,byte_Ton);

}


//-------------------------------------------------------------------------------------------
void print_para() // display information on the serial monitor
{
  Serial.print("Solar Panel: ");
  Serial.print(float_v_pv,2);
  Serial.print(" V ; ");
  Serial.print(float_i_pv*100,0);
  Serial.print(" mA ; ");
  Serial.print(float_p_pv/10,3);
  Serial.print(" W    ");

  //Serial.print("MPPT: ");
  //if (int_slope==1)        {Serial.print("  L  ; ");}
  //else if (int_slope==-1)  {Serial.print("  R  ; ");}
  //else                     {Serial.print(" MPP ; ");}

  //Serial.print("dP/dV=");
  //if (float_dPdV > 0)      {Serial.print("+");}
  //else if (float_dPdV == 0)      {Serial.print(" ");}
  //Serial.print(float_dPdV,3);

  Serial.print("Charger: ");
  Serial.print(float_v_pv/float_i_pv*10,2);
  Serial.print(" Ohm");

  Serial.print(" ; D=");
  Serial.print(byte_Ton*100/255);
  Serial.print("%    ");

  Serial.print("Storage:");
  Serial.print(float_v_out,2);

  Serial.println(" V   ");
}
