

/*
 * 
 *  >>>> THIS CODE USED TO TEST OBCI V3 PROTOTYPES <<<<
 *
 * testing BLOCK WRITE using REAL OBCI data  
 *
 * This code is written to run on the OpenBCI V3 board. 
 * Adjust as needed if you are testing on different hardware.
 *
 * By Chip Audette August, 2014
 * Based on Skeleton Code from Joel Muprhy, Spring/Summer, 2014. 
 *
 */

#include <EEPROM.h>
#include <SPI.h>
#include <SdFat.h>      // from https://github.com/greiman/SdFat
#include <SdFatUtil.h>  // from https://github.com/greiman/SdFat
#include "OpenBCI_04.h"  

int byteCounter = 0;     // used to hold position in cache
boolean logging = false;
//unsigned int timeBetweenFiles;
//boolean betweenFiles = false;

//------------------------------------------------------------------------------
//  << OpenBCI BUSINESS >>
OpenBCI OBCI; //Uses SPI bus and pins to say data is ready.  Uses Pins 13,12,11,10,9,8,4
//#define MAX_N_CHANNELS (8)  //must be less than or equal to length of channelData in ADS1299 object!!
int nActiveChannels = 1;   //how many active channels would I like?
byte gainCode = ADS_GAIN24;   //how much gain do I want
byte inputType = ADSINPUT_SHORTED;   //here's the normal way to setup the channels
unsigned int sampleCounter = 0;      // used to time the tesing loop
boolean is_running = false;    // this flag is set in serialEvent on reciept of prompt
boolean startBecauseOfSerial = false;
char leadingChar;
int outputType;

//------------------------------------------------------------------------------
//  << LIS3DH Accelerometer Business >>
//  LIS3DH_SS on pin 5 defined in OpenBCI library
int axisData[3];  // holds X, Y, Z accelerometer data
boolean xyzAvailable = false;

// use cout to save memory use pstr to store strings in flash to save RAM
ArduinoOutStream cout(Serial); 


void setup(void) {

  Serial.begin(115200);
// pinMode(LIS3DH_SS,OUTPUT); digitalWrite(LIS3DH_SS,HIGH);   // de-select the LIS3DH
//  pinMode(SD_SS,OUTPUT); digitalWrite(SD_SS,HIGH);           // de-select the SD card
//  pinMode(ADS_SS,OUTPUT); digitalWrite(ADS_SS,HIGH);         // de-select the ADS
//  pinMode(DAISY_SS,OUTPUT); digitalWrite(DAISY_SS,HIGH);         // de-select the Daisy Module
  
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2);
    
  delay(3000); //was 4000
 
  OBCI.initialize_ads();
  cout << ("OBCI_StreamOneChannel\n");  delay(1000);

//  SPI.setDataMode(SPI_MODE1);
  for (int chan=1; chan <= nActiveChannels; chan++) {
    OBCI.activateChannel(chan, gainCode, inputType);
  }
  
  delay(1000);
}


void loop() {
  while (Serial.read() >= 0) {}  // clear out the serial buffer
  
  cout << pstr("Type any character to start\n"); delay(1000);// prompt user to begin test
  while (Serial.read() <= 0) {}  // wait here for serial input
  cout << pstr("Starting\n");  delay(1000);
  sampleCounter = 0;

  digitalWrite(SD_SS,HIGH);  // de-select SD card

//    cout << pstr("Enable Accelerometer\n");
//    OBCI.enable_accel();
    
    //cout << pstr("Starting OpenBCI data log to ") << currentFileName << pstr("\n"); 
    SPI.setDataMode(SPI_MODE1);
    OBCI.start_ads();
    
    is_running = true;
    
    while(is_running){
      while(!(OBCI.isDataAvailable())){   // watch the DRDY pin
        delayMicroseconds(10);
      }

      SPI.setDataMode(SPI_MODE1);
      OBCI.updateChannelData();
      sampleCounter++;
      
      if(OBCI.LIS3DH_DataReady()){
        SPI.setDataMode(SPI_MODE3);
        axisData[0] = OBCI.getX();
        axisData[1] = OBCI.getY();
        axisData[2] = OBCI.getZ();
        xyzAvailable = true;
      }
      SPI.setDataMode(SPI_MODE0);
      
      if (sampleCounter >= 256) {
        cout << pstr("Stopping\n");
        is_running = false;
        SPI.setDataMode(SPI_MODE1);
        OBCI.stop_ads();
        SPI.setDataMode(SPI_MODE0);
        OBCI.disable_accel(); 
        delay(1000);
      } 
  }
}

