

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


//-----------------------------------------------------------------
// Data packet info
const int dataLength = 24;  // size of 
  // load dummyData with ascii if you want to see it on the serial monitor
 char dummyData[dataLength] = {
   '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F',
   '0','1','2','3','4','5','6','\n'
 };
  // load byte values for testing OpenBCI data conversion
//char dummyData[dataLength] = {
//  0xFF,0xFF,0x00,  // -256
//  0x00,0x00,0xFF,  // 255
//  0xFF,0xFF,0x00,
//  0x00,0x00,0xFF,
//  0xFF,0xFF,0x00,
//  0x00,0x00,0xFF,
//  0xFF,0xFF,0x00,
//  0x00,0x00,0xFF
//};

char serialCheckSum;            // holds the byte count for streaming 
int sampleCounter = 0;         // sample counter
word packetCounter = 0;         // used to limit the number of packets during tests
boolean streamingData = false;  // streamingData flag is set when 'b' is received    

//------------------------------------------------------------------------------
//  << OpenBCI BUSINESS >>
OpenBCI OBCI; //Uses SPI bus and pins to say data is ready.  Uses Pins 13,12,11,10,9,8,4
//#define MAX_N_CHANNELS (8)  //must be less than or equal to length of channelData in ADS1299 object!!
int nActiveChannels = 1;   //how many active channels would I like?
byte gainCode = ADS_GAIN24;   //how much gain do I want
byte inputType = ADSINPUT_SHORTED;   //here's the normal way to setup the channels
boolean is_running = false;    // is the ADS1299 running?
boolean startBecauseOfSerial = false;
char leadingChar;
int outputType;


// use cout to save memory use pstr to store strings in flash to save RAM
ArduinoOutStream cout(Serial); 

void startData(void) {
  //cout << pstr("Starting OpenBCI data log to ") << currentFileName << pstr("\n"); 
  //SPI.setDataMode(SPI_MODE1);
  OBCI.start_ads();
  is_running = true;
}

void stopData(void) {
  is_running = false;
  //SPI.setDataMode(SPI_MODE1);
  OBCI.stop_ads();
  //SPI.setDataMode(SPI_MODE0);
  //OBCI.disable_accel(); 
  delay(100);
  
}

void setup(void) {

  Serial.begin(115200);
// pinMode(LIS3DH_SS,OUTPUT); digitalWrite(LIS3DH_SS,HIGH);   // de-select the LIS3DH
  pinMode(SD_SS,OUTPUT); digitalWrite(SD_SS,HIGH);           // de-select the SD card
//  pinMode(ADS_SS,OUTPUT); digitalWrite(ADS_SS,HIGH);         // de-select the ADS
//  pinMode(DAISY_SS,OUTPUT); digitalWrite(DAISY_SS,HIGH);         // de-select the Daisy Module

  
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  delay(3000); //was 4000
  cout << ("OBCI_StreamOneChannel\n");  delay(1000);
 
  //initialize the ADS Hardware
  SPI.setDataMode(SPI_MODE1);
  OBCI.initialize_ads();
  for (int chan=1; chan <= nActiveChannels; chan++) OBCI.activateChannel(chan, gainCode, inputType);
  delay(1000);
  
  //send instructions to user
  cout << pstr("'b' to start\n"); delay(500);// prompt user to begin test
  cout << pstr("'s' to stop\n"); delay(500);// prompt user to begin test
  //benchWriteTime = 0;           // used to benchmark transmission time
  serialCheckSum = dataLength + 1;  // serialCheckSum includes sampleCounter
  sampleCounter = 0;

}



void loop() {
   
  if (is_running){
      
    //wait until data is available
    //SPI.setDataMode(SPI_MODE1);
    while(!(OBCI.isDataAvailable())){   // watch the DRDY pin
      delayMicroseconds(10);
    }
    
    //get the data
    //SPI.setDataMode(SPI_MODE1);
    OBCI.updateChannelData();
    sampleCounter++;  //count how many samples we've taken

    if(streamingData){              // receive 'b' on serial to set this
      //benchWriteTime = micros();                 // BENCHMARK SAMPLE WRITE TIME
      Serial.write(serialCheckSum);              // send the number of bytes to follow
      Serial.write(packetCounter);         // send the sampleCounter
      
      //write OpenBCI's raw bytes
      for (int i=0; i<dataLength; i++) {
        Serial.write(OBCI.ads.rawChannelData[i]);  // send a data sample
      }
      packetCounter++;  // count the number of times we send a sample   
      if (packetCounter > 255) packetCounter = 0; //constrain
      delay(100);
    }
    
    //is it time to stop?
    if (sampleCounter >= 30) {
      Serial.write(0x01);
      streamingData=false;
      cout << pstr("\nStopping\n");delay(1000);
      stopData();
    } 
  }
}

void serialEvent(){ //done at end of every loop(), when serial data has been detected
  
  while (Serial.available()) { //loop here until the receive buffer is empty!
    char token = Serial.read();
    //Serial.print("got ");  Serial.write(token);  Serial.print('\n'); delay(400);
       
    switch (token) {
      case 'b':
        cout << pstr("\nStarting\n");delay(1000);
        streamingData = true;
        sampleCounter = 0;
        startData();
        //Serial.print('B'); delay(20);        // give Device time to catch up
        break;
        
      case 's':
        if(streamingData){Serial.write(0x01);} // send checkSum for the command to follow
        streamingData = false;
        stopData();
        cout << pstr("\nStopping\n");delay(1000);
        //Serial.print('S'); delay(20);        // give Device time to catch up
        break;
        
      default:
        break;
     }
     if(!streamingData){  // send checkSum for the verbose to follow
       Serial.print("got the ");
       Serial.write(token);
       Serial.print('\n');
     }
     delay(20);
  }
}
