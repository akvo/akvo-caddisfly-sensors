```
Building a sustainable internet of things
```
WEC

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
Figure 1.1: Water EC meter
```
The purpose of the Water Electrical Conductivity (WEC) sensor is to provide an indication of (drinking) water
quality. Electrical conductivity (EC) reflects the ability of a specific material to conduct an electric current,
in micro Siemens, orμS/cm. For reference, tap water in the Netherlands has a value of 200 to 300μS/cm,
seawater around 50,000μS/cm depending on where the sample is taken from.

To read the values, the sensor unit needs to be connected with a cable to a smartphone. A mobile app, Akvo
Caddisfly, is used to display the water electrical conductivity.


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
- Take a sample of the water to be tested (has to be taken in a clean and dry sample container)

### 3.2 Using

- The sensor is placed in the measuring container
- When inserting the sensor in the water sample,make sure the probes are fully submerged in the
    sample!
       - For the most constant reading, it is advised to stir the sample slightly with the sensor when inserting
          the sensor.
- Click ’read’ in the app. The display on the smart-phone provides the value of the EC inμS/cm
    immediately, nevertheless the operator should wait until the output reading stabilizes
- The actual value of the water is read, stored to the memory and sent to the database
- After the measurement the unit has to be rinsed with clean water and dried with a clean cloth


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
DEVICE Identifier ID (e.g. ”WaterEC 101”)
READING Temperature, EC,
ranges 0-50, 0-100000 (e.g. 25.1,254.3)
R (same like reading, for compatibility with older versions) Temperature, EC,
ranges 0-50, 0-100000 (e.g. 25.1,254.3)
RAW Temperature, Resistance,
ranges 0-50, 0-100000 (e.g. 25.1,254.3)
READWITHOUTTEMP EC 0-100000 (e.g. ”352”)
GET<KEY> The value of KEY or NOTSUPPORTED
Water EC KEY options:
”POINT<NUMBER 1-6>” (no space)
”POINTS”
”ID”
(e.g. ”55.55”)
SET<KEY>”VALUE” Water EC set KEY options:
”POINT<NUMBER 1-6>” (no space)
”ID”
PROCESS Water EC Calibration process
Instructions in the terminal
```

## Appendix B

# Calibration

```
The sensors come pre-programmed and pre-calibrated from the factory. If a sensor needs to be re-calibrated,
the following steps have to be followed:
1.The Water EC is calibrated using 6 measurements. Prepare the salt-water solutions using the readily
available solutions:
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

Figure B.1: Calibrating the Ec sensor in 235 liquid


## Appendix C

# Validation

```
The water EC meter validation document from water quality laboratory of UNESCO-IHE in Delft, The
Netherlands (3 pages, written by Arthur Heijstek of Akvo):
```

Performance of the Akvo Caddisfly EC

sensor.

Validation of Akvo’s EC sensor

1. Methodology

The following section discusses the methodology that is used in performing a

validation test of a sample of random selected Akvo Caddisfly EC sensors. The test

has been performed on 10-10-2016 at the water quality laboratory of UNESCO-IHE

in Delft, The Netherlands. This test is performed in order to describe the accuracy of

Akvo Caddisfly EC sensors.

1.1 Definition of accuracy: trueness and precision

Accuracyis defined as the ‘closeness of agreement between a test result or

measurement result and the true value’. Accuracy refers to a combination of

trueness and precision (ISO 3534-2:2006, clause 3.3.1). See figure 1 for a visual

explanation of trueness and precision.

Truenessis defined as the ‘closeness of agreement between the expectation of a

test result or a measurement result and a true value’. In practice, the accepted

reference valueis substituted for the true value (ISO 3534-2:2006, clause 3.3.3).

```
⦁ Trueness is determined by calculating the deviation percentageof the mean
of the sample in relation to the reference value.
```
Precisionis defined as the ‘closeness of agreement between independent test /

measurement results obtained under stipulated conditions (ISO 3534-2:2006, clause

3.3.4).

```
⦁ Precision is determined by the standard errorformula and then transformed
into relative standard error in %.
```
1.2 Standard Solutions

The following standard solutions have been used in performing the validation test.

```
Concentration (Mols/L) Electric Conductivity
(μS/cm)
0,10 11550
0,02 2600
0,0030 458
0,0013 223
0,0010 137,
```

1.2 Test procedures

Six random selected Akvo Caddisfly EC sensors were selected for analysis. With

every sensor, 5 measurements were performed on each standard solution.

Moreover, a reference value of the standard solution was created by measuring the

conductivity with a ...... meter. The average of the 5 measurements for each sensor

and the reference value were used to calculate trueness and precision, according to

the definitions of the previous section. In between measurements sensors were

rinsed with distilled water and dried with dry paper. For every sensor measurement

were performed starting at the lowest EC-standard solution and ending the highest

EC-standard solution in order to minimize the effect of the mixing of different

standard solutions (a small concentration has fewer impact on high concentration

then vice versa). The results are displayed in the next section.

1.3 Results

```
EC standard solution (uS/cm)
205/235 205/220 435/454 2680 12580
PVC-EC1 EC-lab value 205 205 435 2680 12580
Average EC (n=5) 200 199 430 2616 12267
Relative STDEV
(%)
```
```
0,55 0,22 0,27 0,11 2,
```
```
Trueness 98 97 99 98 98
PVC-EC2 EC-lab value 205 205 435 2680 12580
Average EC (n=5) 203 200 431 2643 12553
Relative STDEV
(%)
0,66 0,57 0,26 0,06 1,
```
```
Trueness 99 97 99 99 100
PVC-EC3 EC-lab value 205 205 435 2680 12580
Average EC (n=5) 203 198 443 2616 10344
Relative STDEV
(%)
0,41 0,57 0,20 0,11 1,
```
```
Trueness 99 97 98 98 82
REGGS-EC1 EC-lab value 226 220 454 2680 12580
Average EC (n=5) 225 218 439 2639 11731
Relative STDEV
(%)
0,58 0,38 0,12 0,06 0,
```
```
Trueness 100 99 97 98 93
REGGS-EC2 EC-lab value 205 205 453 2680 12580
Average EC (n=5) 213 203 432 2623 11893
Relative STDEV
(%)
```
```
0,33 1,02 0,38 0,03 0,
```
```
Trueness 96 99 95 98 95
REGGS-EC3 EC-lab value 235 205 455 2680 12580
Average EC (n=5) 224 205 433 2626 12142
```

```
Relative STDEV
(%)
0,45 1,14 0,26 0,13 0,
```
```
Trueness 95 100 95 98 97
```
2. Software improvement.

In this version of the Akvo Caddisfly EC sensor, the following items are changed:

```
⦁ Timing and duration of the measurement
⦁ Measure open circuit voltage as part of the calibration, and use it in
computations (and store it in flash memory)
⦁ Avoid computing resistance, but compute conductivity directly from
measured voltage
⦁ Optimization of averaging
⦁ Use Steinhart-hart temperature formula instead of B-parameter
⦁ A number of other small items.
```
3. Conclusion

Taking into account the software improvement the following conclusions can be

drawn

(note: the higher the trueness percentage, the smaller the error (1), the higher the

precision percentage, the higher the error (2):

```
⦁ Concerning the standard solutions of 205 until 2680 us/cm trueness ranges
from 97 till 100%. The average trueness for this range for the six random
selected sensors is 98%.
⦁ Concerning the standard solutions of 205 until 2680 us/cm precision ranges
from 0.1 till 0,7%.
⦁ Concerning the standard solution of 11.550 us/cm trueness ranges from 82
till 100%. The average trueness for this range for the six random selected
sensors is 94%.
⦁ Concerning the standard solution of 11.550 us/cm precision ranges from 0.
till 2,9%.
```

