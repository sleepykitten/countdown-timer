# Countdown Timer
Countdown Timer is an interactive Arduino program that plays sounds and blinks an LCD after a configurable delay.

The display shows the remaining time and available commands executable by pushbuttons.

The `settings.h` file contains the default timer values (which can be modified after the program starts), various settings (alarm stuff, auto backlight toggling, auto countdown restart...), and the pins used to receive input and control the components.

Since the program uses the `micros` function to keep track of time, the timer might become less accurate over time. The time drift is automatically corrected by the program (by default every hour a second is added to the timer), and can be controlled via the `TIMER_ADJUST_ACCURACY` setting and others in the settings file.

## Components
- LCD (WH1602B-TMI)
- piezo (PKM22EPP-40)
- two pushbuttons
- two 1k ohm resistors (pull-down resistors for the buttons)
- 220 ohm resistor (for the LCD backlight diode)
- 1k ohm resistor (for the piezo)

## The circuit
![Circuit schematic](https://raw.githubusercontent.com/sleepykitten/countdown-timer/master/countdown_timer/images/circuit-schematic.png)

![Circuit photo](https://raw.githubusercontent.com/sleepykitten/countdown-timer/master/countdown_timer/images/circuit-photo.jpg)
