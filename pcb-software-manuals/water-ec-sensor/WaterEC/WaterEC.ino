/*
* Copyright 2016, by M2M4ALL BV
*
* This INO sketch is the main program for AKVO Water EC sensor.
*/

#include "MCP342X.h"
#include "ecmeter.h"
#include "FlashStorage.h"
#include "version.h"
#include <Wire.h>
#include <Arduino.h>

#define DEVICE "WaterEC"
#define STATUS "OK"
#define OK "OK"
#define Error "ERROR"

void setup();
void loop();
void GET(String KEY);
void SET(String KEY, float number);
void StoreNewPoints();
bool calibrationProcess();
void commandLine(String &inData);
static void showStartupBanner(Stream & stream);


int fluid[6]= {141,235,470,1413,3000,12880};

typedef struct {
  boolean   valid;
  uint16_t  id;
  float     vopen;  // 1970
  float     point1; //141
  float     point2; //235
  float     point3; //470
  float     point4; //1413
  float     point5; //3000
  float     point6; //12880
} configEC;
configEC points;

FlashStorage(my_flash_store, configEC);

String inData;

float onePointRawValueAavgTemp = 0;
float onePointRawValueavgavgResistance = 0;


void setup() 
{
  
  ECMeter_init();
  SerialUSB.begin(9600);
   //SerialUSB.setTimeout(30000);
    while (!SerialUSB) { } 
	
	//for debug
    //points=my_flash_store.read();
    //if (points.valid!=true) SerialUSB.println("no points in memory");
	
	showStartupBanner(SerialUSB);

	points=my_flash_store.read();
    setPoints(points.point1, points.point2, points.point3, points.point4, points.point5, points.point6, points.vopen);
    calculateCalibrationValues();
}


void loop() 
{
    inData = ""; // Clear buffer
    SerialUSB.flush();
    while (SerialUSB.available() > 0)
    {
        String inData = SerialUSB.readStringUntil('\n'); //read data from serial
        
        commandLine(inData);
    }
}

void GET(String KEY){
         points=my_flash_store.read();
         
         if (KEY.equalsIgnoreCase("POINT1")) 
           SerialUSB.println (points.point1);
         else if (KEY.equalsIgnoreCase("POINT2")) 
           SerialUSB.println (points.point2);
         else if (KEY.equalsIgnoreCase("POINT3")) 
           SerialUSB.println (points.point3);
         else if (KEY.equalsIgnoreCase("POINT4")) 
           SerialUSB.println (points.point4);
         else if (KEY.equalsIgnoreCase("POINT5")) 
           SerialUSB.println (points.point5);
         else if (KEY.equalsIgnoreCase("POINT6")) 
           SerialUSB.println (points.point6);
         else if (KEY.equalsIgnoreCase("POINTS"))
         { 
            SerialUSB.print (points.point1); SerialUSB.print(", ");
            SerialUSB.print (points.point2); SerialUSB.print(", ");
            SerialUSB.print (points.point3); SerialUSB.print(", ");
            SerialUSB.print (points.point4); SerialUSB.print(", ");
            SerialUSB.print (points.point5); SerialUSB.print(", ");
            SerialUSB.println (points.point6);
         }
         else if (KEY.equalsIgnoreCase("VOPEN"))
            SerialUSB.println (points.vopen);
          
         else if (KEY.equalsIgnoreCase("ID")) 
           SerialUSB.println (points.id);
           
         else  SerialUSB.println ("NOT_SUPPORTED");
}


void SET(String KEY, float number){
 
         if (KEY.equalsIgnoreCase("POINT1")){
              //SerialUSB.println (number);
              if (number==0)
              {
                SerialUSB.println(Error);
              }
              else
              {
                points.point1 = number;
                SerialUSB.println (OK);
              }
                
         }
          else if (KEY.equalsIgnoreCase("POINT2")) {
              //SerialUSB.println (number);
               if (number==0)
              {
                SerialUSB.println(Error);
              }
              else
              {
                points.point2 = number;
                SerialUSB.println (OK);
              }
          }
          else if (KEY.equalsIgnoreCase("POINT3")) {
              //SerialUSB.println (number);
               if (number==0)
              {
                SerialUSB.println(Error);
              }
                else
              {
                points.point3 = number;
                SerialUSB.println (OK);
              }
          }
          else if (KEY.equalsIgnoreCase("POINT4")) {
              //SerialUSB.println (number);
              if (number==0)
              {
                SerialUSB.println(Error);
              }
                else
              {
                points.point4 = number;
                SerialUSB.println (OK);
              }
              
          }
          else if (KEY.equalsIgnoreCase("POINT5")) {
              //SerialUSB.println (number);
               if (number==0)
              {
                SerialUSB.println(Error);
              }
                else
              {
                points.point5 = number;
                SerialUSB.println (OK);
              }
          }
          else if (KEY.equalsIgnoreCase("POINT6")) {
              //SerialUSB.println (number);
               if (number==0)
              {
                SerialUSB.println(Error);
              }
                else
              {
                points.point6 = number;
                SerialUSB.println (OK);
              }
          }

          else if (KEY.equalsIgnoreCase("VOPEN")) {
              //SerialUSB.println (number);
               if (number==0)
              {
                SerialUSB.println(Error);
              }
                else
              {
                points.vopen = number;
                SerialUSB.println (OK);
              }
          }
          
          else if (KEY.equalsIgnoreCase("ID")) {
              //SerialUSB.println (number);
              points.id = number;
              SerialUSB.println (OK);
          }  
              
         else SerialUSB.println ("NOT_SUPPORTED");
           
           StoreNewPoints();
           
}

void StoreNewPoints(){
   
    points.valid = true;
    my_flash_store.write(points);
    delay(10);
    points=my_flash_store.read();
    setPoints(points.point1, points.point2, points.point3, points.point4, points.point5, points.point6, points.vopen);
    calculateCalibrationValues();

}


void waitForGo(){
    String temp;
    do{
        SerialUSB.flush();
        temp = SerialUSB.readStringUntil('\n'); //read data from serial
        temp = temp.substring(0,temp.length()-1); 
        //SerialUSB.println(temp);
      } while (!(temp.equalsIgnoreCase("GO")));
  }

bool calibrationProcess(){
    
    SerialUSB.println("Start of Water EC calibration procedure."); 
    SerialUSB.println("First, make sure the sensor is dry, and put it on a dry towel");
    SerialUSB.println("When ready, press \"go\"");
    delay(100);
    waitForGo();
    float calibrationSuccess = openCircuitVoltage();
    SerialUSB.println(calibrationSuccess);
    points.vopen = calibrationSuccess;

    // the rest of the measurements depends on this value, so set it here.
    setVopen(calibrationSuccess);

    SerialUSB.print("Next, we measure the sensor for the calibration fluids. ");
    for (int i=0;i < 6;i++){
      SerialUSB.print("Make the WaterEC ready for "); SerialUSB.print("fluid number ");
      SerialUSB.print(i+1);
      SerialUSB.print(": ");SerialUSB.print(fluid[i]);SerialUSB.println(" mS" );
      SerialUSB.println(" ");
      SerialUSB.println("Put the sensor inside the fluid and press \"go\"");
      delay(100);

      waitForGo();
      
      SerialUSB.print("Starting calibration for "); SerialUSB.println(fluid[i]);
      SerialUSB.println ("it will take around 1 minute...");
    
    calibrationSuccess = onePointRawConductance();
    SerialUSB.println(calibrationSuccess);

    if      (i+1==1) points.point1 = calibrationSuccess;
    else if (i+1==2) points.point2 = calibrationSuccess;
    else if (i+1==3) points.point3 = calibrationSuccess;
    else if (i+1==4) points.point4 = calibrationSuccess;
    else if (i+1==5) points.point5 = calibrationSuccess;
    else     points.point6 = calibrationSuccess;
       
    if (i<5){
      SerialUSB.println("");
      SerialUSB.println("great! clean the WaterEC and we will go to the next fluid");
    }
    else{
      SerialUSB.println("");
      SerialUSB.println("Calibration finished");
      StoreNewPoints();
    }
   }
}

void commandLine(String &inData){
  inData=inData.substring(0,inData.length()-1); 
  
  if (inData.equalsIgnoreCase("STATUS"))
     SerialUSB.println(STATUS);
  else if (inData.equalsIgnoreCase("DEVICE")){ 
     SerialUSB.print(DEVICE);
     SerialUSB.print(" ");
     SerialUSB.println(points.id);
   }
  else if (inData.equalsIgnoreCase("READING")) {
          SerialUSB.print(getTemp(),1);
          SerialUSB.print(",");
          SerialUSB.println(getConductivity(), 1);
  }
  
  else if (inData.equalsIgnoreCase("R")) {
          SerialUSB.print(getTemp(),1);
          SerialUSB.print(",");
          SerialUSB.println(getConductivity(), 1);
  }

  // Note: no temperature correction is applied in this conductance
  else if (inData.equalsIgnoreCase("RAW")) {
            SerialUSB.print(getTemp(), 1);
            SerialUSB.print(",");
            SerialUSB.println(getVoltage(),1);
            //SerialUSB.println(getRawConductance(), 1);
          
  }
  
   else if (inData.equalsIgnoreCase("READWITHOUTTEMP")) {
            SerialUSB.println(getConductivity(), 0);
          }
          
  
  else if (inData.equalsIgnoreCase("PROCESS")) {
          calibrationProcess();
          }
          
  
  else if (inData.substring(0,3)=="GET" || inData.substring(0,3)=="get"){
     String KEY = inData.substring(4,20);
     GET(KEY);
     }
  
  
  else if (inData.substring(0,3)=="SET" || inData.substring(0,3)=="set"){
     String KEY = inData.substring(4,25);
     int firstClosingBracket = KEY.indexOf(' ');
     KEY = inData.substring(4,4+firstClosingBracket);
     String numberString = inData.substring(4+firstClosingBracket+1,inData.length() );
     float x = numberString.toFloat();
     SET(KEY,x);
     }
     
  else SerialUSB.println("NOT_SUPPORTED");
}


/*
* Show the startup banner
*
* First of all it prints the name of the application and its
* version number.
*/
static void showStartupBanner(Stream & stream)
{
	stream.println();
	stream.println(PROGRAM_NAME " " VERSION);
}
