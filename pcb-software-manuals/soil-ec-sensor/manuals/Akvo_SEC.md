```
Building a sustainable internet of things
```
SEC

#### USER MANUAL

Date: December 23, 2016
Version: 1.
Author: Gerard Hogenhout, Kees Hogenhout, Mart Ole
Support:info@sodaq.com


## Contents

- 1 Introduction
- 2 Specifications
- 3 Using the device
   - 3.1 Setting up
   - 3.2 Using
- A Settings and Debugging
   - A.1 Installing the Arduino IDE and drivers
   - A.2 Programming the sensor
   - A.3 Menu
- B Calibration
- C Validation


## Chapter 1

# Introduction

```
Figure 1.1: Soil EC meter
```
The purpose of the Soil Electrical Conductivity (SEC) sensor is to measure the salinity of soil by measuring
the electrical conductivity. Electrical conductivity (EC) reflects the ability of a specific material to conduct an
electric current, in micro Siemens per centimeter, orμS/cm.

To read the values, the sensor unit needs to be connected with a cable to a smartphone. A mobile app, Akvo
Caddisfly, is used to display the soil conductivity.


## Chapter 2

# Specifications

Powered by micro-USB, 5V
Output Digital, USB
Operating current 30mA
Operating temperature -10°Cto 60°C
Recommended Operating temperature 20 °Cto 25°C
Waterproofing epoxy filling, rated for 1.5m depth of water
Probe material Stainless steel 316
Casing Aluminum, polyurethane
Measurement range 50-12800μS/cm


## Chapter 3

# Using the device

### 3.1 Setting up

- Make sure, that the Caddisfly app is installed on your phone and start the app. You can obtain the
    application by downloading it from the Google Play Store (listed as Akvo Caddisfly)
- Plug the sensor in the micro-USB port of the phone
- To measure the electrical conductivity of the soil, the probes of the sensor have to be pushed in the soil

### 3.2 Using

- The sensor is placed in the soil.Do not put extra sideways force on the probes when inserting
    them in the soil!
       - Make sure that the whole length of the probes is inserted in the soil
- Click ’read’ in the app. The display on the smart-phone provides the value of the EC inμS/cm
    immediately, nevertheless the operator should wait until the output reading stabilizes
- The actual value of the soil is read, stored to the memory and sent to the database
- After the measurement the unit is removed from the soil.Do not put extra sideways force on the
    probes when removing the sensor from the soil!
- After the measurement the unit has to be cleaned, preferably with water and dried with a clean cloth


## Appendix A

# Settings and Debugging

### A.1 Installing the Arduino IDE and drivers

```
Info onwww.support.sodaq.com
```
1. Download the Arduino IDEhttps://www.arduino.cc/en/Main/Software
2. Install the Arduino IDE and start it
3.Click on File→Preferences and at the bottom you should see ’Additional Boards Manager URLs’. This
    is where you need to paste the following URL:http://downloads.sodaq.net/test/package_sodaq_
    index.json
4. When you have pasted the URL, click ’OK’ and you are ready for the next step.
5. Click on Tools→Board→Boards Manager
6.Scroll all the way to the bottom, you should see SODAQ SAMD Boards. Click on it, and install the
    latest version.
Now it is possible to use the built in serial monitor of the Arduino IDE (Tools>Serial Monitor) or use a
different serial monitior i.e. PuTTY.

### A.2 Programming the sensor

```
The micro-USB connector that sends the data to the smartphone or the computer, is also used to program
new software on the microcontroller of the sensor.
After downloading the SODAQ board files, the sensor can be updated to the latest software. For connecting the
micro-USB to a computer, a micro-USB female to male USB converter is needed. The sensors are programmed
by the manufacturer with the latest software, but is otherwise fully open source to the buyers.
```

#### APPENDIX A. SETTINGS AND DEBUGGING

### A.3 Menu

The sensors come pre-programmed and pre-calibrated. The following commands can be used with a computer
to get different values from the device or whenever necessary recalibrate or change certain values.

```
COMMAND RESPONSE/KEY
STATUS ”OK”
DEVICE Identifier ID (e.g. ”SoilEC 101”)
READING Temperature, EC,
ranges 0-50, 0-100000 (e.g. 25.1,254.3)
R (same like reading, for compatibility with older versions) Temperature, EC,
ranges 0-50, 0-100000 (e.g. 25.1,254.3)
RAW Temperature, Resistance,
ranges 0-50, 0-100000 (e.g. 25.1,254.3)
READWITHOUTTEMP EC 0-100000 (e.g. ”352”)
GET<KEY> The value of KEY or NOTSUPPORTED
Soil EC KEY options:
”POINT<NUMBER 1-6>” (no space)
”POINTS”
”ID”
(e.g. ”55.55”)
SET<KEY>”VALUE” Soil EC set KEY options:
”POINT<NUMBER 1-6>” (no space)
”ID”
PROCESS Soil EC Calibration process
Instructions in the terminal
```

## Appendix B

# Calibration

```
The sensors come pre-programmed and pre-calibrated from the factory. If a sensor needs to be re-calibrated,
the following steps have to be followed:
1.The Soil EC is calibrated using 6 measurements. We use standard water EC solutions for the calibration.
These solutions are known values. If we would calibrate the sensors in soil, the salt of soil would influence
the EC values, which would make the calibration more complicated.
Prepare the salt-water solutions using the readily available solutions:
```
- 141 μS/cm (mix of 50ml of 1413 and 450ml distilled water)
- 235 μS/cm (mix of 50ml of 1413 and 250ml distilled water)
- 471 μS/cm (mix of 100ml of 1413 and 200ml distilled water)
- 1413 μS/cm (fixed number)
- 3000 μS/cm (fixed number)
- 12880 μS/cm (fixed number)
2.Open the serial monitor (in Arduino IDE: Tools>Serial Monitor) and type ’process’→ENTER to start
the calibration process
3. Follow the instructions on serial monitor.


#### APPENDIX B. CALIBRATION

Figure B.1: Calibrating the Soil EC sensor in 235 liquid, using the same process as with the Water EC sensor


## Appendix C

# Validation

```
To validate the design and calibration of the Soil EC meters, a commercially available and widely used combined
EC/soil moisture sensor was used (GS3 by Decagon).
The sensors were tested in a salt-water solution and in water-soil mixture.
As a reference, the water electrical conductivity was measured with Greisinger GMH 3431 Water EC meter.
```
```
Figure C.1: SEC validation values
```
```
The actual Water EC value was 25% higher in water than the reading of the Decagon GS3 sensor; a similar
difference can be seen in the Soil EC reading.
The readings of the SODAQ sensors are within 4% of the reference value in the water reading. The average
Soil EC readings fall within 6% of the average.
```

