# Temperature Monitoring Human–Machine Interface (HMI)

## Consolidation Report – KH5048CEM Embedded System Design \& Development

## 1\. Project Overview

This project is a temperature-monitoring Human–Machine Interface (HMI). It uses an LM35 sensor to measure temperature and displays the reading on a 16×2 LCD. The user can set a high temperature limit (LH) and a low temperature limit (LL) using two push buttons.

The microcontroller compares the measured temperature with the selected limits. An LED indicates when the temperature is inside the valid range, and a buzzer warns the user when the temperature goes above the high limit.

### 1.1 Project information

|Item|Details|
|-|-|
|Module|KH5048CEM – Embedded System Design \& Development|
|Assessment|Element 2 – Human Machine Interface|
|Project|Temperature Monitoring HMI|
|Temperature sensor|LM35|
|Microcontroller|ATmega328P / Arduino UNO board|
|Display|16×2 LCD with I²C interface|
|Inputs|Two push buttons (LH and LL)|
|Outputs|LED and buzzer|
|Team members|Hazem Fouad (2500132), Mohamed Abdullah (2401594), Adam George (2401433)|
|GitHub repository|https://github.com/HazemFouad/HMI-Embedded-System|
|Demonstration video|Uploaded to GitHub |

## 2\. Project Objectives

The project was designed to:

1. Read the analogue output of the LM35 sensor using the microcontroller ADC.
2. Display the temperature on a 16×2 LCD.
3. Show the high and low temperature limits.
4. Let the user adjust both limits using push buttons.
5. Reduce unwanted button actions using debounce logic.
6. Use short and long presses to change the selected limit.
7. Turn on the LED when the temperature is inside the valid range.
8. Turn on the buzzer when the temperature is above the high limit.
9. Keep both outputs off when the temperature is at or below the low limit.
10. Test the main functions on the hardware prototype.

## 3\. System Design

The LM35 is connected to analogue input A0. The two buttons are connected to D2 and D3. The LCD communicates with the microcontroller through I²C, while the LED and buzzer are controlled through D8 and D9.

### 3.1 System block diagram

```text
                  +------------------+
                  |   LM35 Sensor    |
                  +--------+---------+
                           |
                           v
                  +------------------+
                  | ADC0 / A0        |
                  | ATmega328P       |
                  +--------+---------+
                           |
                           v
+-------------+    +----------------------+    +----------------+
| LH Button   |--->|                      |--->| 16x2 LCD       |
| D2 / PD2    |    |  Main HMI Logic      |    | I2C, 0x27      |
+-------------+    |                      |    +----------------+
                   | - Temperature logic  |
+-------------+    | - Button handling    |    +----------------+
| LL Button   |--->| - Limit management   |--->| LED D8 / PB0   |
| D3 / PD3    |    | - Alert decisions    |    +----------------+
+-------------+    +----------------------+    +----------------+
                                                  | Buzzer D9/PB1 |
                                                  +----------------+
```

### 3.2 Pin connections

|Component|Pin|Function|
|-|-|-|
|LM35 output|A0 / ADC0|Analogue temperature input|
|LH button|D2 / PD2|High-limit adjustment|
|LL button|D3 / PD3|Low-limit adjustment|
|LED|D8 / PB0|In-range indication|
|Buzzer|D9 / PB1|High-temperature alert|
|LCD SDA|A4 / PC4|I²C data|
|LCD SCL|A5 / PC5|I²C clock|
|LCD address|`0x27`|I²C address|

The buttons use the internal pull-up resistors of the ATmega328P. As a result, a pressed button is read as LOW. The circuit should be powered using suitable voltage levels, and the LED and buzzer must be connected in a way that keeps the microcontroller pins within their electrical limits.

## 4\. Software Design

The program is divided into a main application and three modules. The main application handles the HMI behaviour, while the modules provide access to the ADC, digital pins and LCD.

### 4.1 Main application

The main file coordinates the system. It initialises the peripherals, reads the buttons, obtains the temperature, checks the limits and updates the outputs and display.

The default limits are:

* High limit (LH): 40 °C
* Low limit (LL): 20 °C

### 4.2 ADC and temperature conversion

The ADC is configured to use AVCC as its reference, with a prescaler of 128. The LM35 is read through ADC channel 0.

The LM35 output changes by approximately 10 mV for every 1 °C. Assuming a 5 V ADC reference, the input voltage is calculated as:

\[
V\_{mV} = \\frac{ADC \\times 5000}{1023}
]

The temperature is then calculated as:

\[
T(°C) = \\frac{V\_{mV}}{10}
]

The current implementation displays an integer temperature value. The calculation assumes that the ADC reference is 5 V.

### 4.3 Button handling

The two buttons have separate functions:

|Button action|Result|
|-|-|
|Short press on LH button|Increases LH by 1 °C|
|Short press on LL button|Increases LL by 1 °C|
|Hold either button for about 500 ms|Starts decreasing its selected limit|
|Continue holding|Repeats the decrease about every 250 ms|

The main loop checks the buttons approximately every 10 ms. The debounce setting is three ticks (about 30 ms). The program also prevents the limits from crossing each other.

The limits are kept within these boundaries:

* LH cannot exceed 99.
* LL cannot go below 0.
* LH must always be greater than LL.

The values are stored in RAM, not EEPROM. Therefore, the limits return to their default values after a restart.

### 4.4 LCD and output control

The LCD uses the I²C address `0x27`. It displays the temperature and the two configured limits.

The LED and buzzer follow the rules below:

|Temperature condition|LED|Buzzer|
|-|-|-|
|`T > LH`|OFF|ON|
|`LL < T <= LH`|ON|OFF|
|`T <= LL`|OFF|OFF|

The temperature, outputs and display are refreshed when the application processes a new sensor sample. The configured counter and loop delay give an update interval of approximately 200 ms.

### 4.5 Main program sequence

1. Initialise the ADC.
2. Initialise the digital inputs and outputs.
3. Initialise the LCD and show the startup message.
4. Read and process the buttons in the main loop.
5. Periodically read the LM35 through ADC0.
6. Convert the ADC result into degrees Celsius.
7. Compare the temperature with LH and LL.
8. Update the LED and buzzer.
9. Refresh the LCD.
10. Repeat continuously.

## 5\. Testing

The test plan contains twelve manual hardware test cases. They cover startup, temperature reading, button operation, limit boundaries and output behaviour.

The Robot Framework file is used to organise and record the test steps. It does not directly control the Arduino or automatically measure the hardware.

### 5.1 Test case summary

|Test ID|Test description|Expected result|
|-|-|-|
|TC01|Startup and LCD initialisation|Startup message appears, followed by the normal screen and default limits|
|TC02|Temperature reading|LCD temperature responds to the LM35 input|
|TC03|Default limits after restart|LH returns to 40 and LL returns to 20|
|TC04|Short press on LH|LH increases by one, within its limits|
|TC05|Short press on LL|LL increases by one, within its limits|
|TC06|Long press|Selected limit decreases and repeats while held|
|TC07|Debounce and independent buttons|One press causes one change to the correct limit|
|TC08|Temperature inside valid range|LED ON and buzzer OFF|
|TC09|Temperature above LH|LED OFF and buzzer ON|
|TC10|Temperature at or below LL|LED OFF and buzzer OFF|
|TC11|Return to valid range|Buzzer turns OFF and LED turns ON|
|TC12|Change limits while running|Outputs follow the updated limits|

### 5.2 Test execution record

|Test ID|Actual result / observation|Status|
|-|-|-|
|TC01|LCD initializes, briefly displays the startup message, then shows the normal temperature screen with LH=40 and LL=20|Pass|
|TC02|The displayed temperature changes in response to the sensor. The code converts the ADC reading using a 5 V reference and the LM35 scale of 10 mV per degree Celsius; updates occur approximately every 200 ms|Pass|
|TC03|After restart, the limits return to LH=40 and LL=20. The supplied code does not save the limits to EEPROM|Pass|
|TC04|LH increases by 1, LL remains unchanged, and LH cannot exceed 99 or become equal to/below LL|Pass|
|TC05|LL increases by 1, LH remains unchanged, and LL cannot become equal to or greater than LH|Pass|
|TC06|The selected limit decreases after the long-press threshold and repeats approximately every 250 ms while held. LH cannot go below LL+1 and LL cannot go below 0|Pass|
|TC07|Inputs are active-low with internal pull-ups and approximately 30 ms debounce. Each short press changes only its assigned limit once|Pass|
|TC08|LED on D8 is ON and buzzer on D9 is OFF|Pass|
|TC09|LED turns OFF and buzzer turns ON|Pass|
|TC10|LED and buzzer are both OFF. The in-range condition requires temperature to be strictly greater than LL|Pass|
|TC11|Buzzer turns OFF and LED turns ON automatically|Pass|
|TC12|The LED and buzzer respond to the new limits according to the same range rules, without restarting the board|Pass|

## 6\. Bill of Materials (BOM)

|No.|Component|Quantity|Purpose|Unit price (L.E.)|
|-:|-|-:|-|-:|
|1|Arduino UNO / ATmega328P board|1|Main controller|380|
|2|LM35 temperature sensor|1|Temperature measurement|100|
|3|16×2 LCD|1|User display|90|
|4|Push buttons|2|Limit adjustment|2|
|5|LED|1|In-range indication|1|
|6|LED current-limiting resistor|1|Limits LED current, if required|1|
|7|Buzzer|1|High-temperature alert|35|
|8|Breadboard|1|Circuit assembly|25|
|9|Jumper wires|As required|Connections|5|
|10|USB cable|1|Programming and power|60|
||**Total**|||**699 L.E.**|

## 7\. Optimised BOM for a Standalone Product

The current prototype uses an Arduino UNO board, breadboard and jumper wires. If the design is developed into a standalone product, some parts could be changed to reduce size and cost.

|Part|Possible improvement|
|-|-|
|Microcontroller|Use the ATmega328P on a custom PCB instead of the complete UNO board, if programming and power requirements are handled.|
|Power supply|Use a regulated supply suitable for the controller, LCD, sensor and alert devices.|
|LCD|Keep the I²C LCD because it uses fewer microcontroller pins.|
|Buttons|Keep two low-cost momentary push buttons and use the internal pull-ups.|
|PCB|Replace the breadboard and jumper wires with a PCB.|
|Enclosure|Add an enclosure to protect the circuit and provide openings for the display and buttons.|
|Buzzer and LED|Select output devices and drivers based on their current requirements.|

The final standalone BOM should be priced using current supplier quotations. Its total cost has not been estimated here because it depends on the selected parts and supplier.

## 8\. Current Limitations and Possible Improvements

The current version performs the main monitoring and HMI functions, but it could be improved in several ways:

1. Save LH and LL in EEPROM so that the user's settings remain after power is removed.
2. Add a sensor fault check and display an error if the LM35 is disconnected.
3. Average several ADC readings to reduce small fluctuations.
4. Calibrate the temperature reading against a reference thermometer.
5. Add I²C error handling in case the LCD stops responding.
6. Test boundary temperatures and simultaneous button presses.
7. Check whether the buzzer needs a transistor driver rather than being powered directly from a microcontroller pin.

## 9\. Demonstration Video

The demonstration video is stated to be uploaded to GitHub. The demonstration should show:

* Power-up and LCD initialisation.
* Live temperature reading from the LM35.
* The default LH and LL values.
* Short-press adjustment of both limits.
* Long-press decrease behaviour.
* LED operation when the temperature is inside the valid range.
* Buzzer operation when the temperature is above LH.
* Output behaviour when the temperature returns to the valid range.

## 10\. Source Code

**GitHub repository:** https://github.com/HazemFouad/HMI-Embedded-System

The repository should contain the main application, the ADC, DIO and LCD source/header files, the test-case file, this report and the design architecture.

## 11\. Conclusion

The project provides a basic temperature-monitoring HMI using an LM35 sensor and an ATmega328P-based controller. The user can view the measured temperature, adjust the high and low limits, and receive feedback from an LED and a buzzer.

The software is divided into separate modules for ADC, digital I/O and LCD operations, with the main application coordinating the overall behaviour. The twelve test cases provide a clear plan for checking the main functions.