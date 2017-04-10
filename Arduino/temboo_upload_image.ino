/* Setup shield-specific #include statements */
#include <SPI.h>
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <Temboo.h>
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



byte ethernetMACAddress[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetClient client;

int numRuns = 1;  // Execution count, so this doesn't run forever
int maxRuns = 10;  // Maximum number of times the Choreo should be executed

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
    UploadChoreo.addInput("URL", "https://www.arduino.cc/en/uploads/Main/ArduinoEthernetFront450px.jpg");
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
