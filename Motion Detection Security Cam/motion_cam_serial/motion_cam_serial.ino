#include <Adafruit_VC0706.h>
#include <SPI.h>
#include <SD.h>
#include <Base64.h>
#include <SoftwareSerial.h>         


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

void setup() {

  // When using hardware SPI, the SS pin MUST be set to an
  // output (even if not connected or used).  If left as a
  // floating input w/SPI on, this can cause lockuppage.
#if !defined(SOFTWARE_SPI)
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  if(chipSelect != 53) pinMode(53, OUTPUT); // SS on Mega
#else
 // if(chipSelect != 10) pinMode(10, OUTPUT); // SS on Uno, etc.
#endif
#endif

  Serial.begin(9600);
  //Serial.println("VC0706 Camera snapshot test");
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
   // Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }  
  
  // Try to locate the camera
  if (cam.begin()) {
 //   Serial.println("Camera Found:");
  } else {
   // Serial.println("No camera found?");
    return;
  }
  // Print out the camera version information (optional)
  char *reply = cam.getVersion();
  if (reply == 0) {
   // Serial.print("Failed to get version");
  } else {
   // Serial.println("-----------------");
   // Serial.print(reply);
   // Serial.println("-----------------");
  }

  // Set the picture size - you can choose one of 640x480, 320x240 or 160x120 
  // Remember that bigger pictures take longer to transmit!
  
  //cam.setImageSize(VC0706_640x480);        // biggest
  cam.setImageSize(VC0706_320x240);        // medium
  //cam.setImageSize(VC0706_160x120);          // small

  // You can read the size back from the camera (optional, but maybe useful?)
  uint8_t imgsize = cam.getImageSize();
 // Serial.print("Image size: ");
  if (imgsize == VC0706_640x480) Serial.println("640x480");
  if (imgsize == VC0706_320x240) //Serial.println("320x240");
  if (imgsize == VC0706_160x120) Serial.println("160x120");
cam.setMotionDetect(true); 
  //Serial.println("Snap in 3 secs...");
  delay(100);

  //Serial.println("done"); 
  
}

void loop() {

if (cam.motionDetected()) {
   Serial.print("ys");   
   cam.setMotionDetect(false);
// if (! cam.takePicture()) 
  cam.takePicture();
   // Serial.println("Failed to snap!");
//  else 
   // Serial.println("Picture taken!");
  
  // Create an image with the name IMAGExx.JPG
  char filename[13];
  strcpy(filename, "IMAGE00.JPG");
  for (int i = 0; i < 100; i++) {
    filename[5] = '0' + i/10;
    filename[6] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename)) {
      break;
    }
  }
  
  // Open the file for writing
 File imgFile = SD.open(filename, FILE_WRITE);

  // Get the size of the image (frame) taken  
  uint16_t jpglen = cam.frameLength();
 // Serial.print("Storing ");
 // Serial.print(jpglen, DEC);
 // Serial.print(" byte image.");

  int32_t time = millis();
  pinMode(8, OUTPUT);
  // Read all the data up to # bytes!
  byte wCount = 0; // For counting # of writes
  while (jpglen > 0) {
    // read 32 bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min(64, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = cam.readPicture(bytesToRead);
  /*  int j=bytesToRead;
   while (j<0){
    for (int i=0; i<6; i++){
      Serial.print(buffer[i]);
      }
    j--;
    }*/
    imgFile.write(buffer, bytesToRead);
  /*  if(++wCount >= 64) { // Every 2K, give a little feedback so it doesn't appear locked up
     // Serial.print('.');
      wCount = 0;
    }*/
    //Serial.print("Read ");  Serial.print(bytesToRead, DEC); Serial.println(" bytes");
    jpglen -= bytesToRead;
  }
  imgFile.close();
//Serial.print("!");
  //time = millis() - time;
  //Serial.println("done!");
  //Serial.print(time); Serial.println(" ms elapsed");


  // if (SD.exists("site.html")){SD.remove("site.html");}
  
  //Serial.println("<html><body style=\"margin: 0px; background: #0e0e0e;\"><img src=\"data:image/jpeg;base64,");
  
  File myFile = SD.open(filename,FILE_READ);
  if (myFile) {
   // Serial.println(filename);

    // read from the file until there's nothing else in it:
  
     char bytes[36];
      int encodedLen = base64_enc_len(36);
      char encoded[encodedLen];
    while (myFile.available()) {
     
      for (int i=0; i<36; i++){
     bytes[i] = myFile.read();
    //Serial.print(bytes[i]);
     }
     //Serial.print(bytes);
     base64_encode(encoded, bytes, 36);
    Serial.print(encoded); 
 
    }
    
  Serial.print("!");
   //Serial.print("</body></html>");
    // close the file:
    
    myFile.close();
     //Serial.println(); 
  } else {
    // if the file didn't open, print an error:
    //Serial.println("error opening image");
  }
cam.resumeVideo();
  cam.setMotionDetect(true);  
}

else{Serial.print("No"); delay(10);
};




  
}

