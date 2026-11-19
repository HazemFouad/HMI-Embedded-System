*** Settings ***
Documentation     12 manual hardware test cases for the supplied Temperature HMI.
...               Arduino UNO / ATmega328P, LM35 on A0, active-low buttons
...               LH on D2 and LL on D3, LED on D8, buzzer on D9, LCD 16x2.
...               These tests are performed manually on the physical prototype.
Test Tags         manual    hardware    HMI
Suite Setup       Bench Step    Power the prototype over USB and wait for initialization.

*** Test Cases ***
TC01 Startup And LCD Initialization
    Bench Step         Apply USB power to the Arduino and wait for the startup screen to finish.
    Expected Result    LCD initializes, briefly displays the startup message, then shows the normal temperature screen with LH=40 and LL=20.

TC02 Temperature Reading And Display Update
    Bench Step         Warm or cool the LM35 connected to A0 and observe the LCD.
    Expected Result    The displayed temperature changes in response to the sensor. The code converts the ADC reading using a 5 V reference and the LM35 scale of 10 mV per degree Celsius; updates occur approximately every 200 ms.

TC03 Default Limits And Power Reset
    Bench Step         Change the limits, then remove and restore USB power.
    Expected Result    After restart, the limits return to LH=40 and LL=20. The supplied code does not save the limits to EEPROM.

TC04 Short Press Increases High Limit
    Bench Step         Briefly press and release the LH button on D2/PD2.
    Expected Result    LH increases by 1, LL remains unchanged, and LH cannot exceed 99 or become equal to/below LL.

TC05 Short Press Increases Low Limit
    Bench Step         Briefly press and release the LL button on D3/PD3.
    Expected Result    LL increases by 1, LH remains unchanged, and LL cannot become equal to or greater than LH.

TC06 Long Press Decreases Selected Limit
    Bench Step         Hold LH, then repeat with LL, for longer than 500 ms.
    Expected Result    The selected limit decreases after the long-press threshold and repeats approximately every 250 ms while held. LH cannot go below LL+1 and LL cannot go below 0.

TC07 Button Debounce And Independent Operation
    Bench Step         Press and release each button separately, observing both limits.
    Expected Result    Inputs are active-low with internal pull-ups and approximately 30 ms debounce. Each short press changes only its assigned limit once.

TC08 Temperature Within Valid Range
    Bench Step         Set LL=20 and LH=40, then apply a temperature greater than 20 C and less than or equal to 40 C.
    Expected Result    LED on D8 is ON and buzzer on D9 is OFF.

TC09 Temperature Above High Limit
    Bench Step         With LH=40, warm the LM35 until the displayed temperature is greater than 40 C.
    Expected Result    LED turns OFF and buzzer turns ON.

TC10 Temperature At Or Below Low Limit
    Bench Step         With LL=20, cool the LM35 until the displayed temperature is 20 C or lower.
    Expected Result    LED and buzzer are both OFF. The in-range condition requires temperature to be strictly greater than LL.

TC11 Alarm Recovers When Temperature Returns To Range
    Bench Step         After triggering the high-temperature alarm, allow the temperature to return to a value greater than LL and less than or equal to LH.
    Expected Result    Buzzer turns OFF and LED turns ON automatically.

TC12 Outputs Follow Updated Limits
    Bench Step         Keep the sensor temperature approximately steady and change LH/LL so the temperature moves outside and then back inside the configured range.
    Expected Result    The LED and buzzer respond to the new limits according to the same range rules, without restarting the board.

