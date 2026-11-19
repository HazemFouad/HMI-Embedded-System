# Design Architecture
## Temperature Monitoring Human–Machine Interface (HMI)

**Module:** KH5048CEM – Embedded System Design & Development  
**Assessment:** Element 2 – Human Machine Interface

## 1. Introduction

The project is a temperature-monitoring system based on the ATmega328P microcontroller. An LM35 sensor measures the temperature and sends an analogue voltage to the microcontroller. The measured value is then shown on a 16×2 LCD.

The user can change two temperature limits using separate push buttons: the high limit (LH) and the low limit (LL). An LED and a buzzer provide feedback depending on the measured temperature. The main purpose of the design is to make the temperature and its status easy to read and to allow the user to change the operating range without reprogramming the board.

## 2. Overall System Architecture

The system has four main parts:

- **Input:** LM35 temperature sensor and two push buttons.
- **Processing:** ATmega328P, which reads the sensor, processes button presses and compares the temperature with the limits.
- **Display:** 16×2 LCD connected through an I²C interface.
- **Outputs:** LED and buzzer, controlled according to the temperature status.

### 2.1 Hardware block diagram

```text
        +------------------+
        |   LM35 Sensor    |
        +--------+---------+
                 |
          Analogue signal
                 |
                 v
        +------------------+
        | ADC0 / A0        |
        | ATmega328P       |
        +--------+---------+
                 |
                 v
+-------------+     +-------------------------+     +----------------+
| LH Button   |---->|                         |---->| 16x2 LCD       |
| D2 / PD2    |     |   Main HMI Application  |     | I2C, 0x27      |
+-------------+     |                         |     +----------------+
                    | - Convert temperature   |
+-------------+     | - Process buttons       |     +----------------+
| LL Button   |---->| - Check LH and LL        |---->| LED            |
| D3 / PD3    |     | - Decide output status  |     | D8 / PB0       |
+-------------+     |                         |     +----------------+
                    +-------------------------+     +----------------+
                                                    | Buzzer         |
                                                    | D9 / PB1       |
                                                    +----------------+
```

The LM35 signal is connected to ADC channel 0 (A0). The buttons are connected to digital inputs D2 and D3. The LCD uses the I²C pins A4 (SDA) and A5 (SCL). The LED and buzzer are connected to D8 and D9 respectively.

## 3. Hardware Connections

| Component | ATmega328P / UNO pin | Direction | Purpose |
|---|---|---|---|
| LM35 output | ADC0 / A0 | Analogue input | Temperature measurement |
| LH button | PD2 / D2 | Digital input | Changes the high limit |
| LL button | PD3 / D3 | Digital input | Changes the low limit |
| LED | PB0 / D8 | Digital output | Indicates temperature within range |
| Buzzer | PB1 / D9 | Digital output | Warns when temperature is above LH |
| LCD SDA | PC4 / A4 | I²C | LCD data |
| LCD SCL | PC5 / A5 | I²C | LCD clock |

The buttons use the ATmega328P internal pull-up resistors. This means that an unpressed button reads HIGH and a pressed button reads LOW. The LCD I²C address used by the project is `0x27`.

## 4. Software Structure

The program is split into a main application and three peripheral modules. This keeps the hardware-specific operations separate from the logic that controls the HMI.

### 4.1 Main application

The main application file, `TempHMI.ino`, coordinates the whole system. Its main responsibilities are:

- Initialising the ADC, digital I/O and LCD.
- Reading the two button states and handling short and long presses.
- Reading the LM35 through ADC channel 0 and converting the result into degrees Celsius.
- Comparing the measured temperature with LH and LL.
- Updating the LED, buzzer and LCD.

The default limits are `LH = 40 °C` and `LL = 20 °C`.

### 4.2 ADC module

The ADC module provides two functions:

| Function | Purpose |
|---|---|
| `ADC_Init()` | Configures the ADC reference, enables the ADC and sets its prescaler. |
| `ADC_Read(uint8_t channel)` | Selects an ADC channel, starts a conversion and returns the 10-bit result. |

The application calls `ADC_Read(0)` to read the LM35 signal on A0. The configuration described in the project uses AVCC as the reference and a prescaler of 128.

### 4.3 Digital I/O module

The DIO module handles the buttons and the two output devices.

| Function | Purpose |
|---|---|
| `DIO_Init()` | Sets the button pins as inputs with pull-ups and the LED/buzzer pins as outputs. |
| `DIO_ReadButtons()` | Reads the two active-low buttons and returns a bit mask. |
| `DIO_SetOutputs(ledEnabled, buzzerEnabled)` | Sets the LED and buzzer output states. |

The button masks are defined in `DIO.h` as `BUTTON_HIGH_MASK` and `BUTTON_LOW_MASK`.

### 4.4 LCD module

The LCD module controls the 16×2 display through the ATmega328P TWI/I²C hardware. The interface declared in `LCD_FINAL.h` provides these functions:

- `LCD_Init()` – initialises the LCD.
- `LCD_Clear()` – clears the display.
- `LCD_SetCursor(row, column)` – moves the cursor.
- `LCD_Print(text)` – displays a text string.
- `LCD_PrintNumber(number)` – displays an unsigned integer.

The LCD is configured at I²C address `0x27`.

## 5. Button Operation and Limit Adjustment

Each button has its own state information in the main application. The program uses this information to filter contact bounce and to identify whether the user made a short press or held the button down.

| Action | LH button (D2) | LL button (D3) |
|---|---|---|
| Short press and release | Increases LH by 1 °C | Increases LL by 1 °C |
| Hold for about 500 ms | Starts decreasing LH | Starts decreasing LL |
| Keep holding | Decreases again about every 250 ms | Decreases again about every 250 ms |

The timing is based on a main-loop interval of approximately 10 ms. The debounce period is three ticks (about 30 ms), the long-press threshold is 50 ticks (about 500 ms), and the repeat interval is 25 ticks (about 250 ms).

The application also checks the limits before changing them:

- LH cannot be greater than 99.
- LL cannot be less than 0.
- LH must remain greater than LL.
- LL must remain lower than LH.

The limits are held in RAM. They are not saved to EEPROM, so the values return to 40 °C and 20 °C after the board is restarted or power is removed.

## 6. Temperature Reading and Conversion

The LM35 produces an output of approximately 10 mV for every 1 °C. The ADC returns a 10-bit value between 0 and 1023. With a 5 V reference, the application first estimates the input voltage in millivolts:

\[
V_{mV} = \frac{ADC \times 5000}{1023}
\]

The temperature is then calculated using the LM35 scale factor:

\[
T(°C) = \frac{V_{mV}}{10}
\]

The implementation stores the temperature as an integer, so the LCD displays whole degrees Celsius rather than a decimal value. This calculation assumes that AVCC is 5 V; an actual difference in the board's reference voltage can affect the reading.

## 7. Temperature Status and Output Logic

After a new temperature sample is processed, the application compares it with the two limits. The output behaviour is:

| Temperature condition | LED | Buzzer | Meaning |
|---|---|---|---|
| `T > LH` | OFF | ON | Temperature is above the high limit |
| `LL < T <= LH` | ON | OFF | Temperature is inside the selected range |
| `T <= LL` | OFF | OFF | Temperature is at or below the low limit |

The LED is therefore an in-range indicator, not a general power indicator. Both outputs are off when the temperature is at or below LL.

### 7.1 Decision flow

```text
              +---------------------+
              | Read temperature T  |
              +----------+----------+
                         |
                         v
              +---------------------+
              | Is T greater than   |
              | LH?                 |
              +-----+---------+-----+
                    |         |
                   Yes        No
                    |         |
                    v         v
             +-------------+  +---------------------+
             | LED OFF     |  | Is T greater than   |
             | Buzzer ON   |  | LL?                 |
             +-------------+  +-----+---------+-----+
                                    |         |
                                   Yes        No
                                    |         |
                                    v         v
                             +-------------+ +-------------+
                             | LED ON      | | LED OFF     |
                             | Buzzer OFF  | | Buzzer OFF  |
                             +-------------+ +-------------+
```

## 8. Main Program Flow

The program starts by initialising the peripherals. It then enters the main loop, where the buttons are checked on every loop cycle. Temperature reading and LCD/output updates take place periodically.

```text
        +----------------------+
        | Power on             |
        +----------+-----------+
                   |
                   v
        +----------------------+
        | Initialise ADC       |
        +----------+-----------+
                   |
                   v
        +----------------------+
        | Initialise DIO       |
        +----------+-----------+
                   |
                   v
        +----------------------+
        | Initialise LCD       |
        +----------+-----------+
                   |
                   v
        +----------------------+
        | Show startup message |
        +----------+-----------+
                   |
                   v
        +----------------------+
        | Enter main loop      |
        +----------+-----------+
                   |
                   v
        +----------------------+
        | Read/process buttons |
        +----------+-----------+
                   |
                   v
        +----------------------+
        | Update counter       |
        +----------+-----------+
                   |
                   v
        +----------------------+
        | 20 ticks reached?    |
        +-----+-----------+----+
              |           |
             No          Yes
              |           |
              |           v
              |  +----------------------+
              |  | Read ADC and convert |
              |  | temperature          |
              |  +----------+-----------+
              |             |
              |             v
              |  +----------------------+
              |  | Check LH and LL      |
              |  +----------+-----------+
              |             |
              |             v
              |  +----------------------+
              |  | Update LED, buzzer   |
              |  | and LCD              |
              |  +----------+-----------+
              |             |
              +-------------+
                    |
                    v
             +-------------+
             | Repeat loop |
             +-------------+
```

The loop delay is approximately 10 ms. The temperature/display counter is set to 20, giving an update interval of about 200 ms, excluding execution overhead.

## 9. Timing and Configuration Summary

| Setting | Value | Use |
|---|---:|---|
| CPU clock | 16 MHz | Microcontroller clock assumption |
| ADC reference | AVCC | ADC voltage reference |
| ADC prescaler | 128 | ADC clock division |
| ADC resolution | 10-bit | Result from 0 to 1023 |
| LCD I²C address | `0x27` | LCD communication |
| I²C speed | Approximately 100 kHz | LCD communication |
| Main-loop delay | 10 ms | Button sampling |
| Debounce | 3 ticks / 30 ms | Filters button bounce |
| Long press | 50 ticks / 500 ms | Detects a held button |
| Repeat interval | 25 ticks / 250 ms | Repeated decrease |
| Display counter | 20 ticks / about 200 ms | Temperature/display refresh |
| Default LH | 40 °C | Initial high limit |
| Default LL | 20 °C | Initial low limit |

## 10. Design Limitations

A few points are worth considering if the project is developed further:

- The limits are not stored in EEPROM, so they reset after a power cycle.
- The temperature conversion assumes a 5 V ADC reference.
- The ADC routine waits for conversion completion and does not include a timeout.
- The temperature reading is not averaged or digitally filtered.
- The LCD communication routines do not provide a full I²C bus-recovery mechanism.
- The LED and buzzer are updated when a new temperature sample is processed, rather than on every button-sampling cycle.
- The design does not describe a sensor-disconnection detection feature.

These are limitations of the current implementation, not features that have already been tested or added.

## 11. Conclusion

The architecture separates the project into a main application, an ADC module, a digital I/O module and an LCD module. The main application uses these modules to read the LM35, process the user's button presses, update the two temperature limits and control the display and alert outputs.