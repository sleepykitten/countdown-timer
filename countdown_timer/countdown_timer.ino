/* 
 * Countdown Timer: Interactive Arduino program that plays sounds and blinks an LCD after a configurable delay.
 * Copyright (C) 2017 sleepykitten
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "settings.h"
#include <LiquidCrystal.h>

LiquidCrystal lcd( PIN_LCD_4_RS, PIN_LCD_6_E, PIN_LCD_11_D4, PIN_LCD_12_D5, PIN_LCD_13_D6, PIN_LCD_14_D7 );

String last_screen, next_screen, next_state, pressed_button;
bool settings_valid, lcd_update_required, backlight_on, backlight_activation_required, change_state_already_executed;
char timer_remaining_hours, timer_remaining_minutes, timer_remaining_seconds, alarm_executions_remaining;
unsigned long timer_remaining_days, last_micros_event, last_micros_detected_second;
unsigned long timer_correction_extra_microseconds, timer_correction_seconds_since_adjustment;

/*
 * Executes when Arduino is turned on
 */
void setup(){
	setup_hw_stuff();
	validate_settings();

	if( DEBUG_MODE ){
		display_DEBUG_MODE_screen();
	} else {
		next_state = "start program";
	}
}

/*
 * Executes continuously after setup() is executed
 */
void loop(){
	if( settings_valid ){
		if( DEBUG_MODE ){
			debug_check_buttons();
		} else {
			change_state();
			update_screen();
			check_buttons();
			execute_command();
			toggle_backlight();
		}
	}
}

/*
 * Executes stuff depending on the state of the program
 */
void change_state(){
	if( next_state == "start program" ){
		if( change_state_already_executed ){
			//if change_state_already_executed is true ( meaning that this is not the first execution of this function ) and auto restart is enabled, restart the timer; otherwise just show the default screen
			if( COUNTDOWN_AUTO_RESTART ){
				next_state = "start countdown";
			}

			//turns on the backlight on again if the alarm was stopped while the backlight was off
			if( LCD_AUTO_BACKLIGHT == false && backlight_on == false ){
				digitalWrite( PIN_LCD_15_LED, HIGH );
				backlight_activation_required = true;
			}
		} else {
			backlight_on = false;
			backlight_activation_required = true;
		}

		//resets variables
		timer_remaining_days = timer_default_days;
		timer_remaining_hours = timer_default_hours;
		timer_remaining_minutes = timer_default_minutes;
		timer_remaining_seconds = timer_default_seconds;
		timer_correction_extra_microseconds = 0;
		timer_correction_seconds_since_adjustment = 0;
		alarm_executions_remaining = ALARM_EXECUTIONS;
		pressed_button = "";
		change_state_already_executed = true;
		last_micros_detected_second = NULL;
		last_screen = NULL;
		lcd_update_required = true;

		next_screen = "default screen";
		next_state = "none"; //prevens executing the state we've just been in again
	} else if( next_state == "start countdown" ){
		last_micros_detected_second = micros();
		next_screen = "countdown started";
		next_state = "update timer";
	} else if( next_state == "pause countdown" ){
		next_screen = "countdown paused";
		next_state = "none";
	} else if( next_state == "resume countdown" ){
		next_screen = "countdown started";
		next_state = "update timer";
	} else if( next_state == "update timer" ){
		update_timer_values();

		if( are_timer_values_non_zero() == false ){
			next_screen = "countdown completed";
			next_state = "trigger alarm";
		}
	} else if( next_state == "trigger alarm" ){
		if( alarm_executions_remaining == 0 ){
			if( COUNTDOWN_AUTO_RESTART ){
				next_state = "start program";
			}

			next_state = "none";
		} else {
			start_alarm();
			update_last_micros_event();
		}
	} else if( next_state == "none" ){
		//in this state does nothing happens until a command is given
	} else {
		print_centered_first_line( "STATE NOT FOUND" );
		print_centered_second_line( "\"" + next_state + "\"" );
		delay( DAY );
	}
}

/*
 * Updates the screen
 */
void update_screen(){
	show_time_left();

	//makes sure that the screen will be updated only once
	if( last_screen != next_screen ){
		if( next_screen == "default screen" ){
			print_buttons_on_second_line( "SELECT:", "START" );
		} else if( next_screen == "default screen - edit" ){
			print_right_button_on_second_line( "EDIT" );
		} else if( next_screen == "countdown started" ){
			print_right_button_on_second_line( "PAUSE" );
		} else if( next_screen == "countdown started - reset" ){
			print_right_button_on_second_line( "RESET" );
		} else if( next_screen == "countdown paused" ){
			print_right_button_on_second_line( "RESUME" );
		} else if( next_screen == "countdown paused - reset" ){
			print_right_button_on_second_line( "RESET" );
		} else if( next_screen == "countdown completed" ){
			print_buttons_on_second_line( "", "RETURN" );
		} else if( next_screen == "edit adding days" ){
			print_right_button_on_second_line( "+DAY" );
		} else if( next_screen == "edit removing days" ){
			print_right_button_on_second_line( "-DAY" );
		} else if( next_screen == "edit removing days - next to hours" ){
			print_right_button_on_second_line( "NEXT" );
		} else if( next_screen == "edit adding hours" ){
			print_right_button_on_second_line( "+HOUR" );
		} else if( next_screen == "edit removing hours" ){
			print_right_button_on_second_line( "-HOUR" );
		} else if( next_screen == "edit removing hours - next to minutes" ){
			print_right_button_on_second_line( "NEXT" );
		} else if( next_screen == "edit adding minutes" ){
			print_right_button_on_second_line( "+MINUTE" );
		} else if( next_screen == "edit removing minutes" ){
			print_right_button_on_second_line( "-MINUTE" );
		} else if( next_screen == "edit removing minutes - next to seconds" ){
			print_right_button_on_second_line( "NEXT" );
		} else if( next_screen == "edit adding seconds" ){
			print_right_button_on_second_line( "+SECOND" );
		} else if( next_screen == "edit removing seconds" ){
			print_right_button_on_second_line( "-SECOND" );
		} else if( next_screen == "edit removing seconds - return to default screen" ){
			print_right_button_on_second_line( "RETURN" );
		} else {
			print_centered_first_line( "SCREEN NOT FOUND" );
			print_centered_second_line( "\"" + next_screen + "\"" );
			delay( DAY );
		}

		last_screen = next_screen;
	}
}

/*
 * Executes commands corresponding to options on the screen when buttons are pressed
 */
void execute_command(){
	if( next_screen == "default screen" ){
		if( pressed_button == "left" ){
			next_screen = "default screen - edit";
		} else if( pressed_button == "right" ){
			lcd_update_required = true;
			next_state = "start countdown";
		}
	} else if( next_screen == "countdown started" ){
		if( pressed_button == "left" ){
			next_screen = "countdown started - reset";
		} else if( pressed_button == "right" ){
			next_state = "pause countdown";
		}
	} else if( next_screen == "countdown started - reset" ){
		if( pressed_button == "left" ){
			next_screen = "countdown started";
		} else if( pressed_button == "right" ){
			next_state = "start program";
		}
	} else if( next_screen == "default screen - edit" ){
		if( pressed_button == "left" ){
			next_screen = "default screen";
		} else if( pressed_button == "right" ){
			next_screen = "edit adding days";
		}
	} else if( next_screen == "countdown paused" ){
		if( pressed_button == "left" ){
			next_screen = "countdown paused - reset";
		} else if( pressed_button == "right" ){
			//the current micros() value must be saved to prevent timer correction
			last_micros_detected_second = micros();
			next_state = "resume countdown";
		}
	} else if( next_screen == "countdown paused - reset" ){
		if( pressed_button == "left" ){
			next_screen = "countdown paused";
		} else if( pressed_button == "right" ){
			next_state = "start program";
		}
	} else if( next_screen == "countdown completed" ){
		if( pressed_button == "right" ){
			next_state = "start program";
		}
	} else if( next_screen == "edit adding days" ){
		if( pressed_button == "left" ){
			next_screen = "edit removing days";
		} else if( pressed_button == "right" ){
			//assumes that the integer type is unsigned (the number will roll over to 0 and not to a negative value)
			timer_remaining_days++;
			timer_default_days = timer_remaining_days;
			lcd_update_required = true;
		}
	} else if( next_screen == "edit removing days" ){
		if( pressed_button == "left" ){
			next_screen = "edit removing days - next to hours";
		} else if( pressed_button == "right" ){
			timer_remaining_days--;
			timer_default_days = timer_remaining_days;
			lcd_update_required = true;
		}
	} else if( next_screen == "edit removing days - next to hours" ){
		if( pressed_button == "left" ){
			next_screen = "edit adding days";
		} else if( pressed_button == "right" ){
			next_screen = "edit adding hours";
		}
	} else if( next_screen == "edit adding hours" ){
		if( pressed_button == "left" ){
			next_screen = "edit removing hours";
		} else if( pressed_button == "right" ){
			if( timer_remaining_hours < 23 ){
				timer_remaining_hours++;
				timer_default_hours = timer_remaining_hours;
			} else {
				timer_remaining_hours = 0;
				timer_default_hours = timer_remaining_hours;
			}

			lcd_update_required = true;
		}
	} else if( next_screen == "edit removing hours" ){
		if( pressed_button == "left" ){
			next_screen = "edit removing hours - next to minutes";
		} else if( pressed_button == "right" ){
			if( timer_remaining_hours > 0 ){
				timer_remaining_hours--;
				timer_default_hours = timer_remaining_hours;
			} else {
				timer_remaining_hours = 23;
				timer_default_hours = timer_remaining_hours;
			}

			lcd_update_required = true;
		}
	} else if( next_screen == "edit removing hours - next to minutes" ){
		if( pressed_button == "left" ){
			next_screen = "edit adding hours";
		} else if( pressed_button == "right" ){
			next_screen = "edit adding minutes";
		}
	} else if( next_screen == "edit adding minutes" ){
		if( pressed_button == "left" ){
			next_screen = "edit removing minutes";
		} else if( pressed_button == "right" ){
			if( timer_remaining_minutes < 59 ){
				timer_remaining_minutes++;
				timer_default_minutes = timer_remaining_minutes;
			} else {
				timer_remaining_minutes = 0;
				timer_default_minutes = timer_remaining_minutes;
			}

			lcd_update_required = true;
		}
	} else if( next_screen == "edit removing minutes" ){
		if( pressed_button == "left" ){
			next_screen = "edit removing minutes - next to seconds";
		} else if( pressed_button == "right" ){
			if( timer_remaining_minutes > 0 ){
				timer_remaining_minutes--;
				timer_default_minutes = timer_remaining_minutes;
			} else {
				timer_remaining_minutes = 59;
				timer_default_minutes = timer_remaining_minutes;
			}

			lcd_update_required = true;
		}
	} else if( next_screen == "edit removing minutes - next to seconds" ){
		if( pressed_button == "left" ){
			next_screen = "edit adding minutes";
		} else if( pressed_button == "right" ){
			next_screen = "edit adding seconds";
		}
	} else if( next_screen == "edit adding seconds" ){
		if( pressed_button == "left" ){
			next_screen = "edit removing seconds";
		} else if( pressed_button == "right" ){
			if( timer_remaining_seconds < 59 ){
				timer_remaining_seconds++;
				timer_default_seconds = timer_remaining_seconds;
			} else {
				timer_remaining_seconds = 0;
				timer_default_seconds = timer_remaining_seconds;
			}

			lcd_update_required = true;
		}
	} else if( next_screen == "edit removing seconds" ){
		if( pressed_button == "left" ){
			next_screen = "edit removing seconds - return to default screen";
		} else if( pressed_button == "right" ){
			if( timer_remaining_seconds > 0 ){
				timer_remaining_seconds--;
				timer_default_seconds = timer_remaining_seconds;
			} else {
				timer_remaining_seconds = 59;
				timer_default_seconds = timer_remaining_seconds;
			}

			lcd_update_required = true;
		}
	} else if( next_screen == "edit removing seconds - return to default screen" ){
		if( pressed_button == "left" ){
			next_screen = "edit adding seconds";
		} else if( pressed_button == "right" ){
			next_screen = "default screen";
		}
	}

	//resets the value
	pressed_button = "";
}

/*
 * Updates the remaining time every time a second passes; maintains precision by including extra microseconds
 */
void update_timer_values(){
	unsigned long micros_difference_timer = micros() - last_micros_detected_second;

	//adds extra microseconds
	micros_difference_timer += timer_correction_extra_microseconds;

	//updates the time left only when the number of passed microseconds is equivalent to 1 second or higher
	if( micros_difference_timer >= MICROS_SECOND ){
		last_micros_detected_second = micros();

		short seconds_to_adjust;
		seconds_to_adjust = ( short ) -( micros_difference_timer / MICROS_SECOND ); //determines the difference in seconds and makes negative

		//prevents time drift
		if( TIMER_ADJUST_ACCURACY ){
			if( timer_correction_seconds_since_adjustment >= TIMER_ADJUST_EVERY_X_SECONDS ){
				seconds_to_adjust += TIMER_ADJUST_BY_SECONDS; //adds positive or negative number
				timer_correction_seconds_since_adjustment = 0;
			} else {
				timer_correction_seconds_since_adjustment++;
			}
		}

		timer_correction_extra_microseconds = micros_difference_timer % MICROS_SECOND; //saves the remainder for next time
		adjust_timer_values_by_seconds( seconds_to_adjust );
		lcd_update_required = true;
	}
}

/*
 * Automatically toggles the LCD backlight when it's supposed to be turned on
 */
void toggle_backlight(){
	if( LCD_AUTO_BACKLIGHT ){
		//the difference is equal or higher than the delay
		if( backlight_on ){
			if( ( unsigned long ) micros() - last_micros_event >= MICROS_BACKLIGHT_DELAY ){
				digitalWrite( PIN_LCD_15_LED, LOW );
				backlight_on = false;
			}
		} else {
			if( backlight_activation_required ){
				digitalWrite( PIN_LCD_15_LED, HIGH );
				backlight_on = true;
				backlight_activation_required = false;
			}
		}
	}
}

/*
 * Saves micros() value if auto backlight is enabled
 */
void update_last_micros_event(){
	if( LCD_AUTO_BACKLIGHT ){
		last_micros_event = micros();
	}
}

/*
 * Starts the alarm
 */
void start_alarm(){
	//turns on the LCD backlight
	if( ALARM_BLINK_LCD == true && backlight_on == false ){
		digitalWrite( PIN_LCD_15_LED, HIGH );
		backlight_activation_required = true;
	}

	if( ALARM_PLAY_SOUNDS == true && ALARM_SOUNDS > 0 ){
		for( short i = 1; i <= ALARM_SOUNDS; i++ ){
			tone( PIN_PIEZO, ALARM_SOUND_FREQUENCY, ALARM_SOUND_LENGTH );
			delay( ALARM_SOUND_DELAY );
		}
	}

	//turns off the backlight unless it's the last alarm execution
	if( ALARM_BLINK_LCD == true && alarm_executions_remaining != 0 ){
		if( ALARM_PLAY_SOUNDS == false ){
			delay( ALARM_LCD_DELAY );
		}

		digitalWrite( PIN_LCD_15_LED, LOW );
		backlight_on = false;
	}

	alarm_executions_remaining--;
	delay( ALARM_DELAY );

	//turns the backlight on if it's the last execution
	if( ALARM_BLINK_LCD == true && alarm_executions_remaining == 0 ){
		digitalWrite( PIN_LCD_15_LED, HIGH );
		backlight_on = true;
	}
}

/*
 * Adds or subtracts seconds from the timer values one second at a time
 */
void adjust_timer_values_by_seconds( short seconds ){
	if( are_timer_values_non_zero() == true ){
		while( seconds != 0 ){
			if( seconds < 0 ){
				if( timer_remaining_seconds == 0 ){
					timer_remaining_seconds = 59;

					if( timer_remaining_minutes == 0 ){
						timer_remaining_minutes = 59;

						if( timer_remaining_hours == 0 ){
							timer_remaining_hours = 23;

							if( timer_remaining_days == 0 ){
								timer_remaining_seconds = 0;
								timer_remaining_minutes = 0;
								timer_remaining_hours = 0;
								timer_remaining_days = 0;
								return;
							} else {
								timer_remaining_days--;
							}
						} else {
							timer_remaining_hours--;
						}
					} else {
						timer_remaining_minutes--;
					}
				} else {
					timer_remaining_seconds--;
				}

				seconds++;
			} else {
				if( timer_remaining_seconds == 59 ){
					timer_remaining_seconds = 0;

					if( timer_remaining_minutes == 59 ){
						timer_remaining_minutes = 0;

						if( timer_remaining_hours == 23 ){
							timer_remaining_hours = 0;
							timer_remaining_days++;
						} else {
							timer_remaining_hours++;
						}
					} else {
						timer_remaining_minutes++;
					}
				} else {
					timer_remaining_seconds++;
				}

				seconds--;
			}
		}
	}
}

/*
 * Executes the check_button() functions
 */
void check_buttons(){
	check_button( "left" );
	check_button( "right" );
}

/*
 * Checks whether a button has been pressed
 */
void check_button( String button ){
	short button_pin;
	bool button_pressed = false;

	if( button == "left" ){
		button_pin = PIN_BUTTON_LEFT;
	} else if( button == "right" ){
		button_pin = PIN_BUTTON_RIGHT;
	}

	//ensures that nothing else happens until the button is released
	while( is_analog_signal_detected( button_pin ) == true ){
		button_pressed = true;
	}

	if( button_pressed ){
		//the value will be reset when executing the apropriate command
		pressed_button = button;
		backlight_activation_required = true;

		//when auto backlight is enabled and the backlight is off, a button press will be ignored and no command will be executed
		if( LCD_AUTO_BACKLIGHT && backlight_on == false ){
			pressed_button = "";
		}

		update_last_micros_event();
	}
}

/*
 * Determines whether an input signal was detected
 */
bool is_analog_signal_detected( short pin ){
	if( analogRead( pin ) >= ANALOG_INPUT_TRESHOLD ){
		return true;
	}

	return false;
}
/*
 * Initializes pins and the LCD
 */
void setup_hw_stuff(){
	pinMode( PIN_BUTTON_RIGHT, INPUT );
	pinMode( PIN_BUTTON_LEFT, INPUT );
	pinMode( PIN_PIEZO, OUTPUT );
	pinMode( PIN_LCD_15_LED, OUTPUT );
	pinMode( PIN_LCD_3_VO, OUTPUT );

	digitalWrite( PIN_LCD_3_VO, LOW );
	digitalWrite( PIN_LCD_15_LED, HIGH );

	lcd.begin( LCD_LINE_LENGTH, LCD_LINES );
}

/*
 * Stops the program if invalid settings are found
 */
void validate_settings(){
	settings_valid = are_time_ranges_valid();

	if( settings_valid == false ){
		print_centered_first_line( "ERROR" );
		print_centered_second_line( "INVALID SETTINGS" );
		delay( DAY );
	}
}

/*
 * Checks whether the timer values are non-zero
 */
bool are_timer_values_non_zero(){
	if( timer_remaining_seconds != 0 ){
		return true;
	}

	if( timer_remaining_minutes != 0 ){
		return true;
	}

	if( timer_remaining_hours != 0 ){
		return true;
	}

	if( timer_remaining_days != 0 ){
		return true;
	}

	return false;
}

/*
 * Checks ranges of the timer values and determines whether they're valid
 */
bool are_time_ranges_valid(){
	if( ! ( timer_default_days >= 0 ) ){
		return false;
	}

	if( ! ( timer_default_hours >= 0 && timer_default_hours <= 23 ) ){
		return false;
	}

	if( ! ( timer_default_minutes >= 0 && timer_default_minutes <= 59 ) ){
		return false;
	}

	if( ! ( timer_default_seconds >= 0 && timer_default_seconds <= 59 ) ){
		return false;
	}

	return true;
}

/*
 * Prints a centered string on the first line
 */
void print_centered_first_line( String str ){
	lcd.setCursor( 0, 0 );
	lcd.print( center_text( str ) );
}

/*
 * Prints a centered second on the second line
 */
void print_centered_second_line( String str ){
	lcd.setCursor( 0, 1 );
	lcd.print( center_text( str ) );
}

/*
 * Displays buttons on the second line of the screen
 */
void print_buttons_on_second_line( String left, String right ){
	lcd.setCursor( 0, 1 );
	lcd.print( align_buttons( left, right ) );
}

/*
 * Displays a button aligned to the right on the second line of the screen
 */
void print_right_button_on_second_line( String str ){
	//since we must clear previously used characters, we'll need to calculate the number of spaces we'll put before the string
	short spaces_before_str = 7 - str.length(); //assumes that the longest previously used string had 7 characters
	short modified_str_length = str.length() + spaces_before_str;

	String spaces = "";

	//generates a string of spaces
	for( short i = 1; i <= spaces_before_str; i++ ){
		spaces += " ";
	}

	str = spaces + str;

	//calculates the offset where the new string will be printed
	short cursor_offset = LCD_LINE_LENGTH - modified_str_length;

	lcd.setCursor( cursor_offset, 1 );
	lcd.print( str );
}

/*
 * Displays the string representation of timer values on the first line of the screen
 */
void show_time_left(){
	//ensures that the screen will be updated only when necessary ( when the timer values change )
	if( lcd_update_required ){
		lcd_update_required = false;
		print_centered_first_line( convert_timer_values_to_string() );
	}
}

/*
 * Creates a string from timer values
 */
String convert_timer_values_to_string(){
	String output = add_leading_zero( timer_remaining_hours ) + ":" + add_leading_zero( timer_remaining_minutes ) + ":" + add_leading_zero( timer_remaining_seconds );

	if( timer_remaining_days > 0 ){
		output = ( String ) timer_remaining_days + "d "+ output;
	}

	return output;
}

/*
 * Adds a leading zero to single digit numbers
 */
String add_leading_zero( short number ){
	String str = ( String ) number;

	if( str.length() == 1 ){
		str = "0" + str;
	}

	return str;
}

/*
 * Adds spaces around a string to center it on a 16 short line
 */
String align_buttons( String str1, String str2 ){
	String output;
	short remaining_spaces = LCD_LINE_LENGTH - ( str1.length() + str2.length() );

	if( remaining_spaces > 0 ){
		String spaces = "";

		for( short i = 1; i <= remaining_spaces; i++ ){
			spaces = spaces + " ";
		}

		output = str1 + spaces + str2;
	} else {
		output = str1 + str2;
	}

	return output;
}

/*
 * Adds spaces around a string to center it on a 16 short line
 */
String center_text( String str ){
	short remaining_spaces = LCD_LINE_LENGTH - str.length();

	if( remaining_spaces > 1 ){
		short leading_spaces = remaining_spaces / 2;
		String spaces = "";

		for( short i = 1; i <= leading_spaces; i++ ){
			spaces = spaces + " ";
		}

		str = spaces + str + spaces; //spaces are added also after the string to make sure no old shorts are there when the string length changes 

		//adds an extra space if necessary to make sure the whole row is overwritten
		if( LCD_LINE_LENGTH - str.length() > 0 ){
			str += " ";
		}
	}

	return str;
}

/*
 * Checks whether a button has been pressed and displays detected values
 */
void debug_check_button( String button ){
	short button_pin, lcd_line, input_value;
	bool button_pressed = false;

	if( button == "left" ){
		button_pin = PIN_BUTTON_LEFT;
		lcd_line = 0;
	} else if( button == "right" ){
		button_pin = PIN_BUTTON_RIGHT;
		lcd_line = 1;
	}

	if( ( unsigned long ) micros() - last_micros_event > MICROS_DEBUG_DELAY_1 ){
		last_micros_event = micros();
		input_value = analogRead( button_pin );
		lcd.setCursor( 7, lcd_line );
		lcd.print( ( String ) input_value + "   " ); //ensures that the display is always up to date
	}

	//ensures that nothing else happens until the button is released
	while( is_analog_signal_detected( button_pin ) == true ){
		if( ( unsigned long ) micros() - last_micros_event  > MICROS_DEBUG_DELAY_2 ){
			last_micros_event = micros();
			input_value = analogRead( button_pin );
			lcd.setCursor( 7, lcd_line );
			lcd.print( ( String ) input_value + "   " );
		}

		//ensures that the screen will be rewritten only when the button is not considered to be pressed 
		if( button_pressed == false ){
			lcd.setCursor( 14, lcd_line );
			lcd.print( "on" );
		}

		button_pressed = true;
	}

	if( button_pressed ){
		//removes "on" when the button is released
		lcd.setCursor( 14, lcd_line );
		lcd.print( "  " );
	}
}

/*
 * Executes the debug_check_button() functions
 */
void debug_check_buttons(){
	debug_check_button( "left" );
	debug_check_button( "right" );
}

/*
 * Shows a screen with button labels 
 */
void display_DEBUG_MODE_screen(){
	lcd.setCursor( 0, 0 );
	lcd.print( "left:" );
	lcd.setCursor( 0, 1 );
	lcd.print( "right:" );
}
