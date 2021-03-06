```
Building a sustainable internet of things
```
HAP AND SUM

##### USER MANUAL

#### Date: December 23, 2016

#### Version: 1.

#### Author: Gerard Hogenhout, Kees Hogenhout, Mart Ole

#### Support:info@sodaq.com


## Contents

- 1 Introduction
   - 1.1 HAP
   - 1.2 SUM
- 2 Specifications
   - 2.1 HAP
   - 2.2 SUM
- 3 Using the devices
   - 3.1 Setting up
      - 3.1.1 HAP
      - 3.1.2 SUM
   - 3.2 Using
- A Settings and Debugging
   - A.1 Installing the Arduino IDE and drivers
   - A.2 Menu
   - A.3 SPUL connection
- B Technical details
- C Validation


## Chapter 1

# Introduction

### 1.1 HAP

The purpose of the Household Air Polution (HAP) unit is to collect data on indoor air quality, during usage of
a cook stove. The sensors measure carbon monoxide (CO) and particulate matter (PM). The Stove Usage
Monitoring unit (SUM) measures the usage of the stove and transfers the data to the HAP through a wireless
connection. The HAP sends data to a server every hour through a mobile network.
The unit is connected to the regular power grid with a 5V adaptor, which also charges the battery. The battery
powers the unit in case of power outage.

```
Figure 1.1: The HAP
```

##### CHAPTER 1. INTRODUCTION

### 1.2 SUM

The purpose of the Stove Usage Monitoring unit (SUM) is to collect and transfer data about usage of a stove.
It uses a temperature sensor to determine, if the stove is used or not. The sensor measures temperature in
degrees Celsius (◦C). The stove temperature is measured with the thermocouple that is attached to the clamp.
This clamp is placed on the stove.

The SUM transfers the data to the HAP which collects data on air pollution. This allows correlation analysis
between usage of the stove and air pollution. The SUM data is sent wireless to the HAP after every cooking
event.

```
Figure 1.2: The SUM
```

## Chapter 2

# Specifications

### 2.1 HAP

Powered by 5V adapter input, 800mAh LiPo battery or 5V micro-USB
Output Data transmitted over GSM-network or digital, USB
Operating current 10-150mA
Operating temperature -10°Cto 50°C
Recommended Operating temperature 10 °Cto 40°C
Casing Aluminum, polyurethane
Measurement range PM-sensor Small particles: 1-2μm, large particles: 3-10μm
Measurement range CO-sensor 20ppm-2000ppm carbon monoxide

Datasheet PM-sensor^1
Datasheet CO-sensor^2

### 2.2 SUM

Powered by 1200mAh LiPo battery or micro-USB, 5V (for charging)
Output Wireless communication with HAP or over USB
Operating current 600 μA (in deep-sleep), 12mA (during active-mode)
Operating temperature -10°Cto 50°C
Recommended Operating temperature 10 °Cto 40°C
Waterproofing Epoxy filled
Casing Aluminum, polyurethane
Measurement range 0-600°C

(^1) https://goo.gl/OJBHU
(^2) https://goo.gl/y5eWYf


## Chapter 3

# Using the devices

### 3.1 Setting up

#### 3.1.1 HAP

The HAP is hung on a wall on a nail or a screw (using a hole in the casing), preferably directly above the stove.

The placing of the HAP is depending on the conditions of the location. The best placement can only be
determined at the site. To find the best position for the HAP, it is recommended to start a cooking event and
carefully observe the airflow.

```
Figure 3.1: The hole for hanging the HAP
```

##### CHAPTER 3. USING THE DEVICES

```
To power the HAP, the 5V power adapter has to be connected to the round connector on the bottom of the
HAP.
```
```
Figure 3.2: USB and 5V connectors, end cap screws
```
```
To insert the SIM-card, one of the end caps has to be unscrewed and the PCB has to be slid out. The SIM-card
slot is located on the back side of the PCB.
```
```
Figure 3.3: The SIM-card slot
```
After placing the SIM-card, the settings of the HAP need to be changed according to the network provider
(see annex A2 & A3).

#### 3.1.2 SUM

```
SUM is placed on the stove, using the integrated stainless steel clamp, with the module hanging on the outside
of the stove.
To charge the battery of the SUM, a 5V USB charger (with a micro-USB cable) can be connected to the
bottom of the SUM. The rubber cap has to be folded open to get access to the USB-connector. The SUM
must not be charged while it is attached to the cookstove.
```

##### CHAPTER 3. USING THE DEVICES

```
Figure 3.4: Correct placement of the SUM
```
### 3.2 Using

```
The HAP and the SUM run autonomously and do not require any user input to function after setup.
```
When using the stove and starting the cooking process, it is important tolet the coals or fuel-wood start
before placing any pots or pans on the stove.Cookware placed on the stove helps to direct the heat to
the SUM and will enable the temperature to rise faster, which in turn will trigger the cooking event in the
software.


## Appendix A

# Settings and Debugging

```
Use the following steps to change the settings of the HAP and perform simple debugging:
```
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


##### APPENDIX A. SETTINGS AND DEBUGGING

### A.2 Menu

```
Figure A.1: HAP menu
```

##### APPENDIX A. SETTINGS AND DEBUGGING

Settings in the HAP can be changed by connecting a micro-USB cable to the HAP. After connecting the HAP,
the serial monitor must be opened to get a configuration menu.

Configuring the menu:

1. General information
    (a) The device name and version
    (b) The cause for the last reset:
       (c) Device ID, a unique code for each of the HAP-s
    (d) Data used for debugging, does not concern the user
2. Extra commands:
    (a)Dump SUM Records (DUR): dump all the SUM records from the data flash. Gives all recorded
       sensor values to serial monitor. To choose this option type ’DUR’→ENTER
    (b) Dump Sensor Records (DSR): dump all the sensor records from the data flash. Gives all recorded
       sensor values to serial monitor. To choose this option type ’DSR’→ENTER
       (c)Clear SUM Records (DUR): erase all the SUM records from the data flash. To choose this option
          type ’CLRUR’→ENTER
    (d)Clear Sensor Records (DSR): erase all the sensor records from the data flash. To choose this option
       type ’CLRSR’→ENTER
       (e)Commit Settings (CS): saves the settings without starting the program immediately. To choose this
          option type ’CS’→ENTER
       (f)(Hidden)Start (OK): saves the settings and starts the program. To choose this option type
          ’OK’→ENTER
3. Menu configuration:^1
    (a) Aggregate: time interval for average value of measurements, in minutes
    (b) Upload: time interval to upload the Aggregate values, in minutes
       (c) sync RTC: time interval to synchronize the real-time clock, in minutes
    (d) APN: the APN of the network/operator
       (e) APN user: username for the network APN, usually left blank
       (f) APN password: password for the network APN, usually left blank
    (g) SPUL server: server to use with the WDT
    (h) Timestamp port: port of the server for the timestamp
       (i) SPUL port: port of the server for the sending data
4. Contingencies:
    (a)If HAP says ’SUM not seen’: HAP and SUM connect when both are started at the same time.
       If either of them has been running while the other has just started, then they will not connect
       immediately. If a cooking event is detected, then they will connect and no data will be lost. Without
       any other connection, they will connect after 24 hours.

(^1) To start the HAP you should fill in the aggregate, upload, sync RTC, APN , SPUL server, timestamp port and SPUL port


##### APPENDIX A. SETTINGS AND DEBUGGING

### A.3 SPUL connection

For connecting to the backend, the SPUL protocol is being used.

The SPUL protocol is open source and available to implement:
https://github.com/kukua/concava-connector-spul


## Appendix B

# Technical details

```
The main components of the HAP:
```
```
Figure B.1: HAP components
```

## Appendix C

# Validation

```
To validate the work cycle of the HAP and SUM combination, a cooking event was simulated.
A 3m^2 room with a 7m^3 volume was used to run the test. The stove with the SUM was placed in the middle
of the room. The HAP was placed on the wall at 1,75m height.
Charcoal briquettes were used to fuel the stove. A flat bottomed clay pot with a diameter of 30cm was used
on top of the stove. A pot is required to direct the heat to the SUM. Without the pot, the SUM does not
receive enough heat to trigger the cooking event.
The serial monitor output was logged for both the HAP and the SUM and the data sent over the network
was read from the dashboard. Carbon monoxide(CO), small particles(Psmall) and large particles(Plarge) are
measured by the HAP and the cooking temperature(SUM Temp) is measured by the SUM.
```
```
Figure C.1: Graph of HAP&SUM readings
```
```
The cooking event lasted 140 minutes from igniting the coals to the end. The pot and the SUM were removed
from the oven at 110 minutes to allow for faster cooling after the coals had died.
```

##### APPENDIX C. VALIDATION

The results of the simulation can be seen in the graph in figure C.1.


