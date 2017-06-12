/* Timer */
unsigned long       timer_default_days           = 0;     //0-4294967295
char                timer_default_hours          = 8;     //0-23
char                timer_default_minutes        = 30;    //0-59
char                timer_default_seconds        = 0;     //0-59
const char          TIMER_ADJUST_ACCURACY        = true;  //whether the time left will be automatically adjusted every X seconds to prevent time drift (see https://forum.arduino.cc/index.php?topic=121088)
const unsigned long TIMER_ADJUST_EVERY_X_SECONDS = 3600;  //after how many seconds the timer values will be adjusted
const short         TIMER_ADJUST_BY_SECONDS      = 1;     //how many seconds will be added (positive value) or subtracted (negative value) from the timer values
const char          COUNTDOWN_AUTO_RESTART       = false; //whether the countdown will automatically restart after the last alarm notification

/* Alarm */
const char          ALARM_PLAY_SOUNDS            = true;  //whether sounds will be played when the alarm is triggered
const char          ALARM_BLINK_LCD              = true;  //whether the LCD backlight diode will be blinked when the alarm is triggered
const char          ALARM_EXECUTIONS             = 3;     //how many times the alarm will be executed
const short         ALARM_DELAY                  = 400;   //delay after each alarm execution (ms); during delays the program doesn't do anything else (e.g. register inputs), so if you want to stop the alarm right after the countdown is completed when the alarm is still on, you need to press the right button for a few moments
const char          ALARM_SOUNDS                 = 5;     //how many sounds will be played every time the alarm is executed
const short         ALARM_SOUND_FREQUENCY        = 3000;  //list of notes and their frequencies: http://arduino.cc/en/Tutorial/Tone
const short         ALARM_SOUND_LENGTH           = 100;   //how long a single sound will be played (ms)
const short         ALARM_SOUND_DELAY            = 120;   //delay after each sound begins to play (should be at least a bit higher than ALARM_SOUND_LENGTH) (ms)
const short         ALARM_LCD_DELAY              = 100;   //how long the backlight will be on if sounds are turned off (ms)

/* Input */
const short         ANALOG_INPUT_TRESHOLD        = 1000;  //0-1023; used to determine whether a button has been pressed; needs to be tested using the debug mode if a different circuit is used
const char          DEBUG_MODE                   = false; //if enabled, the screen will only show detected button presses and input values

/* LCD */
const char          LCD_AUTO_BACKLIGHT           = true;  //whether the backlight will be automatically turned off and on, depending on user interactions; if true and the backlight is off, when a button is pressed for the first time, no command will be executed
const unsigned long LCD_AUTO_BACKLIGHT_DELAY     = 10000; //how long it will take to turn off the backlight after the last user interaction or alarm notification (ms)
const char          LCD_LINE_LENGTH              = 16;
const char          LCD_LINES                    = 2;

/* Pins */
const char          PIN_BUTTON_LEFT              = 15;    //15 = A1
const char          PIN_BUTTON_RIGHT             = 14;    //14 = A0
const char          PIN_PIEZO                    = 10;
const char          PIN_LCD_15_LED               = 12;    //LCD backlight
const char          PIN_LCD_3_VO                 = 9;     //LCD contrast
const char          PIN_LCD_14_D7                = 8;
const char          PIN_LCD_13_D6                = 6;
const char          PIN_LCD_12_D5                = 7;
const char          PIN_LCD_11_D4                = 5;
const char          PIN_LCD_6_E                  = 4;
const char          PIN_LCD_4_RS                 = 2;

/* Miscellaneous */
const char          SECOND                       = 1;
const char          MINUTE                       = SECOND * 60;
const short         HOUR                         = MINUTE * 60;
const long          DAY                          = ( long ) HOUR * 24;
const unsigned long MICROS_BACKLIGHT_DELAY       = LCD_AUTO_BACKLIGHT_DELAY * 1000;
const unsigned long MICROS_SECOND                = SECOND * 1000000;
const unsigned long MICROS_DEBUG_DELAY_1         = 100000;
const unsigned long MICROS_DEBUG_DELAY_2         = 200000;
