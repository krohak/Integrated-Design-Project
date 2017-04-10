#include <Adafruit_VC0706.h>
#include <SPI.h>
#include <SD.h>
#include <Ethernet.h>

// comment out this line if using Arduino V23 or earlier
#include <SoftwareSerial.h>         

// uncomment this line if using Arduino V23 or earlier
// #include <NewSoftSerial.h>       

//#define IRQ_TTL 2
int motion =0;
#define T1_COMPARE 15624
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

// Using hardware serial on Mega: camera TX conn. to RX1,
// camera RX to TX1, no SoftwareSerial object is required:
//Adafruit_VC0706 cam = Adafruit_VC0706(&Serial1);

//ethernet stuff
// size of buffer used to capture HTTP requests
#define REQ_BUF_SZ   20

// MAC address from Ethernet shield sticker under board
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetServer server(80);  // create a server at port 80
File webFile;
char HTTP_req[REQ_BUF_SZ] = {0}; // buffered HTTP request stored as null terminated string
char req_index = 0;              // index into HTTP_req buffer
int value=42;
char filename[13]="IMAGE00.JPG";

void setup() {
  // When using hardware SPI, the SS pin MUST be set to an
  // output (even if not connected or used).  If left as a
  // floating input w/SPI on, this can cause lockuppage.
#if !defined(SOFTWARE_SPI)
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  if(chipSelect != 53) pinMode(53, OUTPUT); // SS on Mega
#else
  //if(chipSelect != 10) pinMode(10, OUTPUT); // SS on Uno, etc.
#endif
#endif

// disable Ethernet chip
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);
    

  Serial.begin(9600);
  Serial.println("VC0706 Camera test");
  //attachInterrupt(digitalPinToInterrupt(IRQ_TTL), INT1_ttl, FALLING);
 
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }  
  
  // Try to locate the camera
  if (cam.begin()) {
    Serial.println("Camera Found:");
  } else {
    Serial.println("No camera found?");
    return;
  }
  // Print out the camera version information (optional)
  char *reply = cam.getVersion();
  if (reply == 0) {
    Serial.print("Failed to get version");
  } else {
    Serial.println("-----------------");
    Serial.print(reply);
    Serial.println("-----------------");
  }

  // Set the picture size - you can choose one of 640x480, 320x240 or 160x120 
  // Remember that bigger pictures take longer to transmit!
  
  //cam.setImageSize(VC0706_640x480);        // biggest
  //cam.setImageSize(VC0706_320x240);        // medium
  cam.setImageSize(VC0706_160x120);          // small

  // You can read the size back from the camera (optional, but maybe useful?)
  uint8_t imgsize = cam.getImageSize();
  Serial.print("Image size: ");
  if (imgsize == VC0706_640x480) Serial.println("640x480");
  if (imgsize == VC0706_320x240) Serial.println("320x240");
  if (imgsize == VC0706_160x120) Serial.println("160x120");

   


  //  Motion detection system can alert you when the camera 'sees' motion!
  cam.setMotionDetect(true);           // turn it on
  //cam.setMotionDetect(false);        // turn it off   (default)

  // You can also verify whether motion detection is active!
  Serial.print("Motion detection is ");
  if (cam.getMotionDetect()) 
    Serial.println("ON");
  else 
    Serial.println("OFF");

     Ethernet.begin(mac);  // initialize Ethernet device
    server.begin();           // start to listen for clients
    Serial.print("Arduino webserver can be found at ");
  Serial.println(Ethernet.localIP());
  delay(10);
 // noInterrupts();
 cli();
 /* TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;*/

/*  OCR2A = T2_COMPARE;      // Sampling time (Ts) = (1/ f_clk) x prescaler x (1+OCR2A)
  TCCR2A |= (1 << WGM21);  //CTC mode
  TCCR2B |= (1 << CS22);
  TCCR2B |= (1 << CS21);   //prescaler =1024
  TCCR2B |= (1 << CS20);
  TIMSK2 |= (1 << OCIE2A); //compare match A interrupt enable*/
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = T1_COMPARE;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
//  interrupts();
sei();
    
}



ISR(TIMER1_COMPA_vect)     //Timer 2 interrupt service routine T2ISR
{
   INT1_ttl();      //enable to change the LED brightness
}

void INT1_ttl(){
 
 if (cam.motionDetected()) {motion =1;}
 

  
}

void take_pic(){
  
if (motion){
 // cli(); 
  delay(10);
   Serial.print("here1");
   Serial.println("Motion!");   
   cam.setMotionDetect(false);
   
  if (! cam.takePicture()) 
    Serial.println("Failed to snap!");
  else 
    Serial.println("Picture taken!");
  
   
  strcpy(filename, "IMAGE00.JPG");
  for (int i = 0; i < 100; i++) {
    filename[5] = '0' + i/10;
    filename[6] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename)) {
      break;
    }
  }
  
  
  File imgFile = SD.open(filename, FILE_WRITE);
  
  uint16_t jpglen = cam.frameLength();
  Serial.print(jpglen, DEC);
  Serial.println(" byte image");
 
  Serial.print("Writing image to "); Serial.print(filename);
  
  while (jpglen > 0) {
    // read 32 bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min(32, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = cam.readPicture(bytesToRead);
    imgFile.write(buffer, bytesToRead);

    //Serial.print("Read ");  Serial.print(bytesToRead, DEC); Serial.println(" bytes");

    jpglen -= bytesToRead;
  }
  imgFile.close();
  Serial.println("...Done!");
  cam.resumeVideo();
  cam.setMotionDetect(true);
  motion =0;
// sei();
 }

}

void loop() {

 take_pic();
  //INT1_ttl(); 
  

EthernetClient client = server.available();  // try to get client

    if (client) {  // got client?
      cli(); 
      Serial.print("here2");
        boolean currentLineIsBlank = true;
        if (client.connected()) {
            if (client.available()) {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                // buffer first part of HTTP request in HTTP_req array (string)
                // leave last element in array as 0 to null terminate string (REQ_BUF_SZ - 1)
                if (req_index < (REQ_BUF_SZ - 1)) {
                    HTTP_req[req_index] = c;          // save HTTP request character
                    req_index++;
                }
                // print HTTP request character to serial monitor
                Serial.print(c);
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank) {
                  Serial.print("HELPPPP");
                    // open requested web page file
                /*    if (StrContains(HTTP_req, "GET / ")
                                 || StrContains(HTTP_req, "GET /index.htm")) {
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-Type: text/html");
                        client.println("Connnection: close");
                        client.println();
                        webFile = SD.open("index.htm");        // open web page file
                    }*/
                     /*  else if (StrContains(HTTP_req, "GET /page2.htm")) {
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-Type: text/html");
                        client.println("Connnection: close");
                        client.println();
                        //webFile = SD.open("page2.htm");        // open web page file
                        client.println("<!DOCTYPE html>");
                        client.println("<html>");
                       // client.println("<head><title>Arduino SD Card Web Page 2</title></head>")
                         client.println("<body>");
                       // client.println(String(value));
                      //  client.println("<img src=\"IMAGE00.JPG\" />");
                       // client.println("<p>Go back to <a href=\"index.htm\">main page</a>.</p>");
                        client.println("</body></html>");

                    }*/ 
                    char filename1[13];
                     strcpy(filename1, filename);
                      if (StrContains(HTTP_req, "GET / ")) {
                        Serial.print("AAAAAA");
                        Serial.print(filename1);
                        if ( SD.exists(filename1) && !(SD.open(filename1))) {
                           Serial.print("BBBBB");
                            webFile = SD.open(filename1);
                          }
                        
                        if (webFile) {
                          Serial.print("WOOOOOO");
                            client.println("HTTP/1.1 200 OK");
                            client.println();
                        }
                    }
                 /*
                    else if (StrContains(HTTP_req, "GET /IMAGE01.JPG")) {
                        webFile = SD.open("IMAGE01.JPG");
                        if (webFile) {
                            client.println("HTTP/1.1 200 OK");
                            client.println();
                        }
                    }*/
                    if (webFile) {
                        while(webFile.available()) {
                          Serial.print("WHAAAAA");
                            client.write(webFile.read()); // send web page to client
                        }
                        webFile.close();
                    }
                    // reset buffer index and all buffer elements to 0
                    req_index = 0;
                    StrClear(HTTP_req, REQ_BUF_SZ);
                    //break;
                }
                // every line of text received from the client ends with \r\n
                if (c == '\n') {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                } 
                else if (c != '\r') {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            } // end if (client.available())
              } // end while (client.connected())
        delay(5);      // give the web browser time to receive the data
        client.stop(); // close the connection
 sei();    } // end if (client)


}


// sets every element of str to 0 (clears array)
void StrClear(char *str, char length)
{
    for (int i = 0; i < length; i++) {
        str[i] = 0;
    }
}

// searches for the string sfind in the string str
// returns 1 if string found
// returns 0 if string not found
char StrContains(char *str, char *sfind)
{
    char found = 0;
    char index = 0;
    char len;

    len = strlen(str);
    
    if (strlen(sfind) > len) {
        return 0;
    }
    while (index < len) {
        if (str[index] == sfind[found]) {
            found++;
            if (strlen(sfind) == found) {
                return 1;
            }
        }
        else {
            found = 0;
        }
        index++;
    }

    return 0;
}
