/* Setup shield-specific #include statements */
#include <SPI.h>
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <Temboo.h>
#include <SoftwareSerial.h>
#include <Adafruit_VC0706.h>
#include <Base64.h>
//#include "TembooAccount.h" // Contains Temboo account information

/*
IMPORTANT NOTE about TembooAccount.h

TembooAccount.h contains your Temboo account information and must be included
alongside your sketch. To do so, make a new tab in Arduino, call it TembooAccount.h,
and copy this content into it.
*/

#define TEMBOO_ACCOUNT "krohak1"  // Your Temboo account name
#define TEMBOO_APP_KEY_NAME "myFirstApp"  // Your Temboo app key name
#define TEMBOO_APP_KEY ""  // Your Temboo app key
//#define TEMBOO_DEVICE_TYPE "uno+yun"

//#if TEMBOO_LIBRARY_VERSION < 2
//#error "Your Temboo library is not up to date. You can update it using the Arduino library manager under Sketch > Include Library > Manage Libraries..."
//#endif

/************************************/
// comment out this line if using Arduino V23 or earlier


// uncomment this line if using Arduino V23 or earlier
// #include <NewSoftSerial.h>

//#define IRQ_TTL 2
int motion =0;
//15624

// SD card chip select line varies among boards/shields:
// Adafruit SD shields and modules: pin 10
// Arduino Ethernet shield: pin 4
// Sparkfun SD shield: pin 8
// Arduino Mega w/hardware SPI: pin 53
// Teensy 2.0: pin 0
// Teensy++ 2.0: pin 20
#define chipSelect 4

#if ARDUINO >= 100
// On Uno: camera TX connected to pin 2, camera RX to pin 3:
SoftwareSerial cameraconnection = SoftwareSerial(2, 3);
// On Mega: camera TX connected to pin 69 (A15), camera RX to pin 3:
//SoftwareSerial cameraconnection = SoftwareSerial(69, 3);
#else
NewSoftSerial cameraconnection = NewSoftSerial(2, 3);
#endif
Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);


/*********************************/

byte ethernetMACAddress[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetClient client;

int numRuns = 1;  // Execution count, so this doesn't run forever
int maxRuns = 10;  // Maximum number of times the Choreo should be executed
char filecontents[64];

void setup() {
  Serial.begin(9600);

  // For debugging, wait until the serial console is connected
  delay(4000);
  while(!Serial);

  Serial.print("DHCP:");
  if (Ethernet.begin(ethernetMACAddress) == 0) {
    Serial.println("FAIL");
    while(true);
  }
  Serial.println("OK");
  delay(5000);

  Serial.println("Setup complete.\n");
}

void loop() {
 if (numRuns <= maxRuns) {
  Serial.println("Running Upload - Run #" + String(numRuns++));

  TembooChoreo UploadChoreo(client);

  // Invoke the Temboo client
  UploadChoreo.begin();

  // Set Temboo account credentials
  UploadChoreo.setAccountName(TEMBOO_ACCOUNT);
  UploadChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
  UploadChoreo.setAppKey(TEMBOO_APP_KEY);

  // Set Choreo inputs
    UploadChoreo.addInput("APIKey", "");
    UploadChoreo.addInput("AccessToken", "");
    UploadChoreo.addInput("AccessTokenSecret", "");
    UploadChoreo.addInput("ImageFileContents",filecontents);
    //UploadChoreo.addInput("URL", "https://www.arduino.cc/en/uploads/Main/ArduinoEthernetFront450px.jpg");
    UploadChoreo.addInput("APISecret", "");
    UploadChoreo.addInput("IsPublic","1");

  // Identify the Choreo to run
  UploadChoreo.setChoreo("/Library/Flickr/Photos/Upload");

  // Run the Choreo; when results are available, print them to serial
  UploadChoreo.run();

  while(UploadChoreo.available()) {
   char c = UploadChoreo.read();
   Serial.print(c);
  }
  UploadChoreo.close();
  }

  Serial.println("\nWaiting...\n");
  delay(30000); // wait 30 seconds between Upload calls
}


void INT1_ttl(){

 if (cam.motionDetected()) {motion =1;}

}

void take_pic(){

if (motion){
  delay(10);
   Serial.print("here1");
   Serial.println("Motion!");
   cam.setMotionDetect(false);

  if (! cam.takePicture())
    Serial.println("Failed to snap!");
  else
    Serial.println("Picture taken!");


  uint16_t jpglen = cam.frameLength();
  Serial.print(jpglen, DEC);
  Serial.println(" byte image");

  Serial.print("Converting image to base64");
 // int inputLen = jpglen;
  //sizeof(input);

  int encodedLen = base64_enc_len(jpglen);
  int encoded[encodedLen];

    while (jpglen > 0) {
    // read 32 bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min(64, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = cam.readPicture(bytesToRead);

    //Serial.print("Read ");  Serial.print(bytesToRead, DEC); Serial.println(" bytes");

    /*boolean takePicture(void);//snap
void getPicture(uint16_t length);//send cmd to read picture
uint8_t* readPicture(uint8_t length);//get data*/

    encoded +=  buffer;
    jpglen -= bytesToRead;
  }


  Serial.println("...Done!");
  cam.resumeVideo();
  cam.setMotionDetect(true);
  motion =0;
// sei();
 }

}
