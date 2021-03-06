```
Building a sustainable internet of things
```
SOM

##### USER MANUAL

#### Date: December 23, 2016

#### Version: 1.

#### Author: Gerard Hogenhout, Mart Ole

#### Support:info@sodaq.com


## Contents

- 1 Introduction
- 2 Specifications
- 3 Using the device
   - 3.1 Setting up
      - 3.1.1 Soil
      - 3.1.2 Placing and removing the sensor
   - 3.2 Using
   - 3.3 Further calibration
- A Settings and Debugging
   - A.1 Installing the Arduino IDE and drivers
   - A.2 Programming the sensor
   - A.3 Menu
- B Calibration
   - B.1 Introduction
   - B.2 Preparing the soil
   - B.3 Equations to derive the VWC
   - B.4 Simple calibration procedure
   - B.5 Advanced calibration procedure
   - B.6 Conclusion
- C Calibration graphs
- D Validation


## Chapter 1

# Introduction

The purpose of the Soil Moisture (SOM) sensor is to provide an indication of whether there is sufficient water
in the soil to grow specific crops. It measures the volumetric water content (VMC), in percentages, with a
theoretic range of 0 -100%. In practice, most soils will not have a volumetric water content higher than 50%.

The sensor unit has two probes. One probe emits a radio wave with a frequency of 75Mhz, and the other probe
measures the signal (in volts). Because the dielectric constant of soil is about 4 and the dielectric constant of
water is 80, the dielectric permittivity and thus the capacitance between the probes highly depends on the
soil water content. This is why the voltage on the second probe will vary with different water contents. The
analog voltage is digitalized and converted into VMC by the SOM’s software. To read the values, the sensor
unit needs to be connected with a cable to a smartphone.

A mobile phone app (Akvo Caddisfly) is used to display the percentage of soil moisture.


## Chapter 2

# Specifications

Measurement time Continuous
Accuracy ±10% max. VWC error
Power supply micro-USB
Power consumption 40mA at 5V USB power
Low power mode Available, 30mA at 5V USB power, possible in software
Output Digital, USB
Operating temperature -10◦C to 60◦C
Measurement range 0-100 % VWC (Volumetric Water Content)
Time to first measurement 5-15 seconds (depending on soil VWC)
Interface PC, smartphone
Supported smartphones Any smartphone with USB On-The-Go (OTG) Host support
Casing Aluminum, polyurethane, 316 stainless steel probes, waterproof


## Chapter 3

# Using the device

For the soil moisture sensor, the user can plug in the sensor to the USB port of the smartphone and start the
Caddisfly App. You can obtain the application by downloading it from the Google Play Store (listed as Akvo
Caddisfly).

### 3.1 Setting up

#### 3.1.1 Soil

As the SOM measures VWC, the measurement accuracy depends largely on how uniformly the water droplets
are distributed throughout the soil. This means that unevenness in soil compression, air bubbles, roots and
rocks all have a direct effect on the readout of the sensor. Therefore, the user should insert the sensor in
uniform soil, with the soil compression as uniform as possible. Any air gaps should be avoided, or at least they
should be uniform throughout the sensor’s measurement volume. This measurement volume is estimated to be
about 220 ml, shaped as a cylinder around the transmitting probe, the height of the cylinder slightly longer
than the probes. When the soil is wet, it takes up to 15 seconds for the sensor to reach a steady state VWC
value. In dry soils, this is about 5 seconds.

#### 3.1.2 Placing and removing the sensor

```
Figure 3.1: Advised positioning of the SOM
```

##### CHAPTER 3. USING THE DEVICE

```
The sensor should be put with care into the soil, as the stainless steel probes are connected directly to the
PCB inside. The advised sensor position is shown in figure 3.1: the probes are fully inserted in the soil. It is
best to install the sensor vertically. During the lab test, the sensor had less than 0.5 % deviation when the
angle at which the probes enter the soil was varied.
Make sure that the aluminium casing is not inserted in or touching the soil. Also, do not touch the sensor
while taking measurements. This will influence the values of the measurement.
```
When removing the sensor from the soil,do not remove the sensor from the soil by pulling the cable.
Remove the sensor by grabbing the aluminum casing.

### 3.2 Using

- To measure the moisture content of the soil, the probes of the sensor have to be pushed in the soil
- Click ’read’ in the app. The display on the smartphone provides the value of the moisture in %
    immediately, nevertheless the operator should wait until the output reading stabilizes
- The soil moisture value is read, stored to the memory and sent to the database
- After the measurement the unit is removed from the soil
- The unit has to be cleaned, preferably with water and dried with a clean cloth

### 3.3 Further calibration

```
As there can be a significant variation in the mineral and organic composition of the soil, the sensor can be
calibrated to a specific soil type. See Appendix B.
```

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


##### APPENDIX A. SETTINGS AND DEBUGGING

### A.2 Programming the sensor

The micro-USB connector that sends the data to the smartphone or the computer, is also used to program
new software on the microcontroller of the sensor.
After downloading the SODAQ board files (info onwww.support.sodaq.com), the sensor can be updated to
the latest software. For connecting the micro-USB to a computer, a micro-USB female to male USB converter
is needed. The sensors are programmed by the manufacturer with the latest software, but is otherwise fully
open source to the buyers.

```
Figure A.1: Programming the SOM with the adapter
```

##### APPENDIX A. SETTINGS AND DEBUGGING

### A.3 Menu

The sensors come pre-programmed and pre-calibrated. The following commands can be used with a computer
to get different values from the device or whenever necessary recalibrate or change certain values.

```
COMMAND RESPONSE/KEY
STATUS ”OK”
DEVICE Identifier ID (e.g. ”Soil Moisture 44”)
READING VWC %
R (same like reading, for compatibility with older versions) VWC %,
RAW Voltage
GET<KEY> The value of KEY or NOTSUPPORTED
SOM KEY options:
”POINTS”
”DRYPOINT”
”WETPOINT”
”ID”
SET<KEY>”VALUE” SOM set KEY options:
”DRYPOINT”
”WETPOINT”
”ID”
PROCESS SOM Calibration process
Instructions in the terminal
```

## Appendix B

# Calibration

### B.1 Introduction

```
The SOM’s are factory-calibrated. In this condition, they have an accuracy of 90%. When the user wants a
better accuracy, the sensor can be calibrated to a specific soil type. Using professional equipment, the team of
SODAQ verified that the voltage output of the SOM can be approximated with a linear equation. However,
this equation is different for different soil types, so the calibration procedure will be about determining this
equation. In soils with high organic matter, this equation may be quadratic, but this is again soil-specific.
```
### B.2 Preparing the soil

```
For calibration of the sensor, some care should be taken to get uniform soil. Take a representative soil sample
and heat it at 110◦C for at least 24 hours. This procedure will dry out any soil to 0% VWC. When it comes
out of the oven, it might be best to sieve the soil to remove larger soil particles or small rocks. This avoids
possible measurement errors, caused by air bubbles.
```
### B.3 Equations to derive the VWC

```
According to lab tests, the voltage on the probesV(x) can be approximated by a linear equation
```
```
V(x) =ax+b (B.1)
```
```
withxthe added water content. This equation holds for different VWC’s on the linear line from 0 to 50%
VWC, at that point most soils will be fully saturated. The equation will be used in this form for calibrating,
for determiningxagain (added water - VWC) the equation is rewritten as:
```
```
x=
V(x)−b
a
```
##### (B.2)

```
aandbare the factors that will be found by the calibration. Please note thatxis the added water and is
linked to VWC by
V W C=
```
```
Vwater
Vsoil+water
```
##### ×100% (B.3)

```
So from equation B.1, the present water volume in the soil can be found, while the VWC can be found from
equation B.3. If enough volume of soil is around the sensor in the lab calibration test, this will be representative
for the final soil. This volume is estimated to be about 300 mL.
For every sensor, this equation, the calibration equation, can be obtained. If this equation is inserted in the
sensor’s software, a calibrated VWC will be outputted by the sensor.
```

##### APPENDIX B. CALIBRATION

```
Figure B.1: Plot of a lab test with the trendline drawn in the same graph.
```
### B.4 Simple calibration procedure

During a lab sample, in which 5 SOMs were tested, the slope of the three lines (ain equation B.1) appeared to
be 0.0014, while the overall voltage-to-added water response was very linear. Because the added water-to-VWC
relation is not linear (see equation B.3, the voltage-to-VWC function is also not linear. SOM1 has a very linear
measurement curve, so when that one is taken to visualize the voltage-to-VWC relation, figure B.2 is obtained.
The polynomial regression curve is also plotted, and it can be seen that the curve has at least the order 2. As
can be seen from the 5 SOM1 curves, the dry-soil voltage is varying more than 30 %, while the slopeaof the
measurements is varying roughly by 16 %. If the dry soil voltage is measured, the first inaccuracy is already
out of the way. To remove the slope error, a second calibration point at a known VWC should be obtained.
This second point should be relatively far away from the dry-soil point to reduce the leftover error. During the
lab test, it was noticed that, when the soil approached saturation, the soil became so wet that the sensor could
move free around in the soil, affecting the reading a lot. In order to have a stable measurement point, it is
advised to choose the second point at a point where the soil is still sturdy enough to keep the sensor in a fixed
position.
With this information in mind, the following simple calibration method can be used:

1. Oven-dry 2 litres of soil for 24 hours at 105◦C.
2.Put 1 litre of soil in 2 equal boxes. The box should be such that there is 10 cm of soil next to and under
    the probes.
3. Add 300 mL of demineralized water to the second box and mix as good as possible with the soil.
4. type ’process’ and follow the instructions on the serial monitor.
5. The sensor is ready to use.
This simple calibration method is implemented in the standard software that is delivered by the manu-
facturer(SODAQ). It is activated by sendingprocessto the serial monitor. The user should follow the
instructions to sucessfully complete the calibration. If i.e. a different second calibration point is needed, the
values can be changed in software.


##### APPENDIX B. CALIBRATION

```
Figure B.2: Plot of SOM1 in 1 litre soil, relating voltage to VWC
```
### B.5 Advanced calibration procedure

Sometimes, it is difficult to mix the soil and water very good, for example when certain equipment is missing.
In that case, the second point from section B.4 could have a significant offset, causing a slope error. Because
all values are derived from this slope, the slope error will grow very large at large voltages. To compensate for
bad mixing or bad equipement or when a high precision is needed, more measurement points can be taken and
a trend line can be drawn in Excel. This is exactly what has been done for this report. The effect can be seen
in figure C.2 and C.4, where clearly some measurement values have an error, but still a reasonable slope is
obtained. To get the curve, it is advised to get 5 or more measurement points untill the soil is fully saturated.
The tested soil in the lab was fully saturated at approximately 40 % VWC, which comes down to 400 mL
added to 1 litre of soil. The description will use a 7 datapoints measurement. To calibrate using the advanced
procedure, proceed with the following steps:

1. Oven-dry 7 litres of soil for 24 hours at 105◦C.
2. Put 1 litre of soil in every box
3.Add 75 mL demineralized water to the second box, 150 mL to the third box and add for every new box
    75 mL more. Make sure it is mixed well and avoid air bubbles.
4. Type ’raw’ in the serial monitor to read the probe voltage
5. Read the probe voltage for every box and list it next to ’added mL’ in Excel or another sheet program
6. Create a scatter chart from the obtained values. The trend line gives the slope of the curve.
7. Write the new obtained values to the software and reprogram the sensor.


##### APPENDIX B. CALIBRATION

### B.6 Conclusion

Both calibration procedure -when the soil is well-mixed- allow the user to calibrate the sensor for every any
soil type. However, this means that after calibration, the sensor does work only for that soil type. Further
research is needed to quantify the differences between all different existing soil types to calibrate the sensor for
any soil type in one simple soil type that can be bought anywhere.


## Appendix C

# Calibration graphs

```
Figure C.1: Plot of SOM1 in 1 litre soil
```

##### APPENDIX C. CALIBRATION GRAPHS

Figure C.2: Plot of SOM2 in 1 litre soil

Figure C.3: Plot of SOM3 in 1 litre soil


##### APPENDIX C. CALIBRATION GRAPHS

Figure C.4: Plot of SOM4 in 1 litre soil

Figure C.5: Plot of SOM5 in 1 litre soil


## Appendix D

# Validation

```
To validate the design and calibration of Soil moisture meters, GS3 by Decagon, a commercially available and
widely used combined EC/soil moisture sensor was used.
The sensors were tested in a water-soil mixture. This water-soil mixture was created by adding 300mL of
tap-water to 1L of dried soil. This mixture is then mixed for approximately 15 minutes to ensure the soil is
completely mixed with the tap-water. After this the soil is pressed to be as compact as possible. This way the
minimum of air is left inside the mixture.
```
```
Figure D.1: Comparison Decagon GS3 and SOM sensors
```
```
The table above shows the results of the measurement with multiple Soil Moisture sensors compared to the
Decagon GS3. With each sensor 5 readings were taken to have the average Soil Moisture percentage. This
shows that the Soil Moisture sensors are within the accuracy range of the sensor.
The readings of the SODAQ sensors are consistent and within 12% of the reference value. As soil is impossible
to get absolutely homogeneous, a deviation in the moisture reading is expected.
```

