#pragma once
#include "RGB_Utils.h"
#include <Adafruit_PWMServoDriver.h>
#include "Enums.h"

class RGB_Controller {
public:
	RGB_Controller(void (*debug_print_fncptr)(String), void (*own_delay_fncptr)(int),int WATCHDOG_PIN_ESP32,int WATCHDOG_PIN_PWMBOARD,int BUILTIN_LED_ESP32,
		int LED1_PIN_RED,int LED1_PIN_GREEN,int LED1_PIN_BLUE,int LED2_PIN_RED,int LED2_PIN_GREEN,int LED2_PIN_BLUE,int LED_MAX_RGB_VALUE);
	void cyclic_tasks();
	void set_program(e_prog_nmbr program_number, int rgb_colors_man[3], float hsv_colors_man[3]);
	void set_program(e_prog_nmbr program_number);
	void set_esp32_statusled_on();
	void set_esp32_statusled_off();
	e_watchdog_state watchdog_state;

private:
	void refresh_I2C_watchdog();
	void check_I2C_watchdog();
	void clear_I2C_outputs();
	void set_rgbled(int led_nmbr, int rgb_color[3]);
	void auto_program1();
	void auto_program2();
	void auto_program3();
	void auto_program4();
	void auto_program5();
	void auto_program6();
	void auto_program7();
	void auto_program8();
	void auto_program9();
	void man_program_blynk(int rgb_colors[3]);
	void man_program_rgb(int rgb_colors[3]);
	void man_program_hsv(float hsv_colors_man[3]);
	void(*debug_print)(String msg);
	void (*own_delay)(int milli);

	e_prog_nmbr _last_prog_nmbr;
	e_prog_nmbr _prog_nmbr;
	Adafruit_PWMServoDriver _pwm;
	RGB_Utils _rgb_util;
	bool _watchdog_refresh_state;
	unsigned long _watchdog_refresh_last_change;
	bool _watchdog_state_check;
	int _watchdog_last_change;
	int _watchdog_fail_counts;
	int _WATCHDOG_PIN_ESP32;
	int _WATCHDOG_PIN_PWMBOARD;
	int _BUILTIN_LED_ESP32;
	int _LED1_PIN_RED;
	int _LED1_PIN_GREEN;
	int _LED1_PIN_BLUE;
	int _LED2_PIN_RED;
	int _LED2_PIN_GREEN;
	int _LED2_PIN_BLUE;
	int _LED_MAX_RGB_VALUE;
};

