/*
* Copyright 2016, by M2M4ALL BV
* 
* This INO sketch is the main program for AKVO Soil Moisture sensor.
*
*/


#include "FlashStorage.h"
#include "version.h"
#include "Settings.h"
#include <Wire.h>
#include <Arduino.h>

#define STATUS "OK"
#define OK "OK"
#define Error "ERROR"

void setup();
void loop();
void GET(String KEY);
void SET(String KEY, float number);
void storePoints();
void readPoints();
bool calibrationProcess();
void calculateCalibrationValues();
void commandLine(String &inData);
static void showStartupBanner(Stream & stream);
double getProbeVoltage(int nr_samples);
double getReading();

double somOffset = 0;
double somSlope = 0;


typedef struct {
  boolean   valid;
  uint16_t  id;
  double    som_dry_volts;
  double    som_secondpoint_volts;
  double    calibrating_value;
} configSOM;

configSOM somStorage;

FlashStorage(my_flash_store, configSOM);

String inData;


void setup() 
{
    pinMode(A4, INPUT);
    analogReadResolution(12);
    SerialUSB.begin(9600);
   //SerialUSB.setTimeout(30000);
    while (!SerialUSB) { } 
	
	//for debug
    //points=my_flash_store.read();
    //if (points.valid!=true) SerialUSB.println("no points in memory");
	
	showStartupBanner(SerialUSB);

    somStorage = my_flash_store.read();
    if (!somStorage.valid) {
        SerialUSB.println("Sensor did not calibrated!");
        somStorage.valid = false;
        somStorage.id = 0;
        somStorage.som_dry_volts = STANDARD_OFFSET;
        somStorage.som_secondpoint_volts = STANDARD_WET_V;
        somStorage.calibrating_value = POINT_OF_CALIBRATION;
        storePoints();
    }
    else readPoints();
    
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
         
         if (KEY.equalsIgnoreCase("DRYPOINT")) 
           SerialUSB.println (somStorage.som_dry_volts,3);
         else if (KEY.equalsIgnoreCase("WETPOINT")) 
           SerialUSB.println (somStorage.som_secondpoint_volts,3);
         else if (KEY.equalsIgnoreCase("CALIBRATIONVALUE")) 
           SerialUSB.println (somStorage.calibrating_value,0);

         else if (KEY.equalsIgnoreCase("POINTS"))
         { 
            SerialUSB.print (somStorage.som_dry_volts); SerialUSB.print(",");
            SerialUSB.print (somStorage.som_secondpoint_volts); SerialUSB.print(",");
            SerialUSB.println (somStorage.calibrating_value);
         }
           
         else if (KEY.equalsIgnoreCase("ID")) 
           SerialUSB.println (somStorage.id);

         else if (KEY.equalsIgnoreCase("EQUATION")) {
             SerialUSB.print("Calibration equation: V(x) = "); SerialUSB.print(somSlope, 4); SerialUSB.print("x + "); SerialUSB.println(somOffset, 3);
         }
           
         else  SerialUSB.println ("NOT_SUPPORTED");
}


void SET(String KEY, float number){
 
    bool dontStoreFlag = false;

    if (KEY.equalsIgnoreCase("DRYPOINT")){
        //SerialUSB.println (number);
        if (number==0)
        {
        SerialUSB.println(Error);
        }
        else
        {
        somStorage.som_dry_volts = number;
        SerialUSB.println (OK);
        }
                
    }
    else if (KEY.equalsIgnoreCase("WETPOINT")) {
        //SerialUSB.println (number);
        if (number==0)
        {
        SerialUSB.println(Error);
        }
        else
        {
        somStorage.som_secondpoint_volts = number;
        SerialUSB.println (OK);
        }
    }
    else if (KEY.equalsIgnoreCase("CALIBRATIONVALUE")) {
        //SerialUSB.println (number);
        if (number==0)
        {
        SerialUSB.println(Error);
        }
        else
        {
        somStorage.calibrating_value = number;
        SerialUSB.println (OK);
        }
    }

    else if (KEY.equalsIgnoreCase("ID")) {
        //SerialUSB.println (number);
        if (number == 0)
        {
            SerialUSB.println(Error);
        }
        else
        {
            somStorage.id = number;
            SerialUSB.println(OK);
        }
    }  
              
    else {
        SerialUSB.println("NOT_SUPPORTED");
        dontStoreFlag = true;
    }
           
    if (!dontStoreFlag) {
        somStorage.valid = true;
        storePoints();
        calculateCalibrationValues();
    }
           
}

void storePoints(){
    
    my_flash_store.write(somStorage);
    delay(10);
    somStorage = my_flash_store.read();
}

void readPoints() {
    somStorage = my_flash_store.read();
}

bool calibrationProcess(){
    
    String inData = SerialUSB.readStringUntil('\n'); //read data from serial

    SerialUSB.println("Starting two-point calibration procedure");
    delay(500);
    
    SerialUSB.println("What is the sensor ID? ");
    String temp;
    float x;
    do {
        SerialUSB.flush();
        temp = SerialUSB.readStringUntil('\n'); //read data from serial
        temp = temp.substring(0, temp.length() - 1);
        x = temp.toFloat();
        //SerialUSB.println(temp);
    } while (x==0);

    SET("ID", x);
    
    
    SerialUSB.println("Clean the sensor, put it in the dry soil and send 'go'.");
    temp = "";
    do {
        SerialUSB.flush();
        temp = SerialUSB.readStringUntil('\n'); //read data from serial
        temp = temp.substring(0, temp.length() - 1);
        //SerialUSB.println(temp);
    } while (!temp.equalsIgnoreCase("go"));

    SerialUSB.println("Reading dry soil moisture value, wait 12 seconds.");
    delay(8000); // Wait 8 seconds
    somStorage.som_dry_volts = getProbeVoltage(2000);
    SerialUSB.println(" ");
    
    SerialUSB.println("Clean the sensor, put it in a mixture of 1 litre soil and 300 mL water and send 'go'.");
    temp = "";
    do {
        SerialUSB.flush();
        temp = SerialUSB.readStringUntil('\n'); //read data from serial
        temp = temp.substring(0, temp.length() - 1);
        //SerialUSB.println(temp);
    } while (!temp.equalsIgnoreCase("go"));
    
    SerialUSB.println("Reading wet soil moisture value, wait 12 seconds.");
    delay(8000); // Wait 8 seconds
    somStorage.som_secondpoint_volts = getProbeVoltage(2000);

    somStorage.valid = true;
    storePoints();

    calculateCalibrationValues();

    SerialUSB.print("Voltage in dry soil: "); SerialUSB.print(somStorage.som_dry_volts, 3); SerialUSB.println(" V");
    SerialUSB.print("Voltage at VWC = "); SerialUSB.print((somStorage.calibrating_value /1000)*100,0);  SerialUSB.print("%:"); SerialUSB.print(somStorage.som_secondpoint_volts, 3); SerialUSB.println(" V");

    
    SerialUSB.print("Calibration equation: V(x) = "); SerialUSB.print(somSlope, 4); SerialUSB.print("x + "); SerialUSB.println(somOffset, 3);

    SerialUSB.println("Two-point calibration finished.");
}



void commandLine(String &inData){
  inData=inData.substring(0,inData.length()-1); 
  
  if (inData.equalsIgnoreCase("STATUS"))
     SerialUSB.println(STATUS);
  else if (inData.equalsIgnoreCase("DEVICE")){ 
     SerialUSB.print(DEVICE);
     SerialUSB.print(" ");
     SerialUSB.println(somStorage.id);
   }
  else if (inData.equalsIgnoreCase("READING")) {
      double value = getReading();
      
      if (value>0) SerialUSB.println(value, 2);
      else SerialUSB.println(0, 2); //showing zero if the value is under zero

  }
  
  else if (inData.equalsIgnoreCase("R")) {
      double value = getReading();

      if (value>0) SerialUSB.println(value, 2);
      else SerialUSB.println(0, 2); //showing zero if the value is under zero
  }
  
  else if (inData.equalsIgnoreCase("RAW")) {
      SerialUSB.println(getProbeVoltage(500), 3);
  }
          
  else if (inData.equalsIgnoreCase("PROCESS")) {
          calibrationProcess();
          }

          
  else if (inData.substring(0,3)=="GET" || inData.substring(0,3)=="get"){
     String KEY = inData.substring(4,20);
     //SerialUSB.println(KEY);
     GET(KEY);}
  
  
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


/*
* Calculate new values for the offset and the slop base on the calibration
*
*/
void calculateCalibrationValues() {

    somOffset = somStorage.som_dry_volts;
    somSlope = (somStorage.som_secondpoint_volts - somStorage.som_dry_volts)/ somStorage.calibrating_value;

}


/*
* Get the analog value from the probe
*
*/
double getProbeVoltage(int nr_samples)
{
    uint32_t total = 0;
    for (int i = 0; i < nr_samples; i++) {
        total += (int)analogRead(A4);
        delay(2);
    }
    total = total / nr_samples;
    double voltage = (3.3 / 4095) * total;
    return voltage;
}


/*
* Get the analog value from the probe and make it percentage base on the calibration 
*
*/
double getReading() {

    double added_water = (getProbeVoltage(500) - somOffset) / somSlope;
    double soil_vwc = (added_water / 1000.0) * 100.0;

    return soil_vwc;

}