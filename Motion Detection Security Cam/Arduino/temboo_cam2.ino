/* Setup shield-specific #include statements */
#include <SPI.h>
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <Temboo.h>
//#include "TembooAccount.h" // Contains Temboo account information
#include <Adafruit_VC0706.h>
#include <SoftwareSerial.h>
#include <Base64.h>

/*
IMPORTANT NOTE about TembooAccount.h

TembooAccount.h contains your Temboo account information and must be included
alongside your sketch. To do so, make a new tab in Arduino, call it TembooAccount.h,
and copy this content into it.
*/

#define TEMBOO_ACCOUNT "krohak1"  // Your Temboo account name
#define TEMBOO_APP_KEY_NAME "myFirstApp"  // Your Temboo app key name
#define TEMBOO_APP_KEY "APP_KEY"  // Your Temboo app key
//#define TEMBOO_DEVICE_TYPE "uno+yun"

//#if TEMBOO_LIBRARY_VERSION < 2
//#error "Your Temboo library is not up to date. You can update it using the Arduino library manager under Sketch > Include Library > Manage Libraries..."
//#endif



byte ethernetMACAddress[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetClient client;

char encodedStringBuffer;


SoftwareSerial cameraconnection = SoftwareSerial(2, 3); // Arduino RX, TX
Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);

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

  if (cam.begin()) {
    Serial.println("Camera Found:");
  } else {
    Serial.println("No camera found");
    return;
  }

  // Set the picture size
  cam.setImageSize(VC0706_160x120);
  Serial.println("Setup complete.\n");

  delay(10);
}

void loop() {
  if (! cam.takePicture()) {
    Serial.println("Failed to snap.\n");
}

  else
{
    Serial.println("Picture taken.\n");

  // Get the size of the image (frame) taken
  uint16_t jpglen = cam.frameLength();

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
    //UploadChoreo.addInput("URL", "https://www.arduino.cc/en/uploads/Main/ArduinoEthernetFront450px.jpg");
    UploadChoreo.addInput("APISecret", "");
    UploadChoreo.addInput("IsPublic","1");

  int j=0;
    while (jpglen > 0) {
        // read 32 bytes at a time;
        uint8_t *buffer;
        uint8_t bytesToRead = min(32, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
        buffer = cam.readPicture(bytesToRead);
        int encodedStringLength = base64_enc_len(bytesToRead);
        char encodedStringBuffer[encodedStringLength];
        Serial.println(*buffer);
        encodedStringLength = base64_encode(encodedStringBuffer, (char *)buffer, bytesToRead);
         Serial.println(encodedStringBuffer);
        jpglen -= bytesToRead;
        j++;
     }

    String ImageFileContentsValue = String(encodedStringBuffer);
  //  Serial.println("hello");
    Serial.println(j);
    Serial.println(ImageFileContentsValue);
    //Serial.println("he");
    UploadChoreo.addInput("ImageFileContents", ImageFileContentsValue);


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
  delay(10000); // wait 30 seconds between Upload calls
}
