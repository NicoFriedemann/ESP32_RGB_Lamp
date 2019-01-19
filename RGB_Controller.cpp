#include "RGB_Controller.h"

RGB_Controller::RGB_Controller(void (*debug_print_fncptr)(String), void (*own_delay_fncptr)(int), int WATCHDOG_PIN_ESP32, 
	int WATCHDOG_PIN_PWMBOARD, int BUILTIN_LED_ESP32,int LED1_PIN_RED, int LED1_PIN_GREEN, int LED1_PIN_BLUE, 
	int LED2_PIN_RED, int LED2_PIN_GREEN, int LED2_PIN_BLUE, int LED_MAX_RGB_VALUE)
{
	debug_print = debug_print_fncptr;
	own_delay = own_delay_fncptr;
	_WATCHDOG_PIN_ESP32 = WATCHDOG_PIN_ESP32;
	_WATCHDOG_PIN_PWMBOARD = WATCHDOG_PIN_PWMBOARD;
	_BUILTIN_LED_ESP32 = BUILTIN_LED_ESP32;
	_LED1_PIN_RED = LED1_PIN_RED;
	_LED1_PIN_GREEN = LED1_PIN_GREEN;
	_LED1_PIN_BLUE = LED1_PIN_BLUE;
	_LED2_PIN_RED = LED2_PIN_RED;
	_LED2_PIN_GREEN = LED2_PIN_GREEN;
	_LED2_PIN_BLUE = LED2_PIN_BLUE;
	_LED_MAX_RGB_VALUE = LED_MAX_RGB_VALUE;
	_watchdog_refresh_state = false;
	_watchdog_refresh_last_change = 0;
	_watchdog_state_check = false;
	_watchdog_last_change = 0;
	_watchdog_fail_counts = 0;
	watchdog_state = e_watchdog_state::ok;
	_pwm = Adafruit_PWMServoDriver();
	_pwm.begin();
	_rgb_util = RGB_Utils(LED_MAX_RGB_VALUE);
	pinMode(_BUILTIN_LED_ESP32, OUTPUT);
	pinMode(_WATCHDOG_PIN_ESP32, INPUT);
	randomSeed(analogRead(22));
	clear_I2C_outputs();
	debug_print("RGB_Controller::RGB_Controller - constructor");
}

void RGB_Controller::cyclic_tasks()
{
	refresh_I2C_watchdog();
	check_I2C_watchdog();
	if (watchdog_state==e_watchdog_state::triggered) {
		debug_print("RGB_Controller::cyclic_tasks - watchog triggered -> wait 5s -> reset!");
		set_esp32_statusled_off();
		delay(5000);
		ESP.restart();
	}
}

//sometimes the I2C connection with the Adafruit PWM Board fails, watchdog checks if the board reacts
//here the watchog-pin is triggered
void RGB_Controller::refresh_I2C_watchdog()
{
	debug_print("RGB_Controller::refresh_I2C_watchdog - run");
	if (_watchdog_refresh_last_change <= (millis() - 1000)) {
		if (_watchdog_refresh_state) {
			_pwm.setPWM(_WATCHDOG_PIN_PWMBOARD, 0, 0);
			_watchdog_refresh_state = false;
		}
		else {
			_pwm.setPWM(_WATCHDOG_PIN_PWMBOARD, 0, _LED_MAX_RGB_VALUE);
			_watchdog_refresh_state = true;
		}
		_watchdog_refresh_last_change = millis();
	}
}

//sometimes the I2C connection with the Adafruit PWM Board fails, watchdog checks if the board reacts
//here the watchog-pin is read
void RGB_Controller::check_I2C_watchdog()
{
	debug_print("RGB_Controller::check_I2C_watchdog - run");
	if (_watchdog_last_change <= (millis() - 1000)) {
		int state_watchdog_pin = digitalRead(_WATCHDOG_PIN_ESP32);
		if (state_watchdog_pin != _watchdog_state_check) {
			watchdog_state = e_watchdog_state::ok;
			_watchdog_fail_counts = 0;
			set_esp32_statusled_on();
			_watchdog_state_check = state_watchdog_pin;
			debug_print("RGB_Controller::check_watchdog - ok");
		}
		else {
			debug_print("RGB_Controller::check_watchdog - failed");
			_watchdog_fail_counts++;
		}
		_watchdog_last_change = millis();
	}
	if (_watchdog_fail_counts > 10) {
		watchdog_state = e_watchdog_state::triggered;
	}
}

void RGB_Controller::clear_I2C_outputs()
{
	debug_print("RGB_Controller::clear_I2C_outputs - run");
	digitalWrite(_BUILTIN_LED_ESP32, LOW);
	_pwm.setPin(_WATCHDOG_PIN_PWMBOARD, _LED_MAX_RGB_VALUE);
	_pwm.setPin(_LED1_PIN_RED, _LED_MAX_RGB_VALUE);
	_pwm.setPin(_LED1_PIN_GREEN, _LED_MAX_RGB_VALUE);
	_pwm.setPin(_LED1_PIN_BLUE, _LED_MAX_RGB_VALUE);
	_pwm.setPin(_LED2_PIN_RED, _LED_MAX_RGB_VALUE);
	_pwm.setPin(_LED2_PIN_GREEN, _LED_MAX_RGB_VALUE);
	_pwm.setPin(_LED2_PIN_BLUE, _LED_MAX_RGB_VALUE);
}

void RGB_Controller::set_esp32_statusled_on()
{
	debug_print("RGB_Controller::set_esp32_statusled_on - run");
	digitalWrite(_BUILTIN_LED_ESP32, HIGH);
}

void RGB_Controller::set_esp32_statusled_off()
{
	debug_print("RGB_Controller::set_esp32_statusled_off - run");
	digitalWrite(_BUILTIN_LED_ESP32, LOW);
}

void RGB_Controller::set_rgbled(int led_nmbr, int rgb_color[3])
{
	if (led_nmbr == 1) {
		_pwm.setPin(_LED1_PIN_RED, rgb_color[0]);
		_pwm.setPin(_LED1_PIN_GREEN, rgb_color[1]);
		_pwm.setPin(_LED1_PIN_BLUE, rgb_color[2]);
	}
	else if (led_nmbr == 2) {
		_pwm.setPin(_LED2_PIN_RED, rgb_color[0]);
		_pwm.setPin(_LED2_PIN_GREEN, rgb_color[1]);
		_pwm.setPin(_LED2_PIN_BLUE, rgb_color[2]);
	}
}

void RGB_Controller::set_program(e_prog_nmbr program_number,int rgb_colors_man[3], float hsv_colors_man[3])
{
	if (program_number != _last_prog_nmbr) {
		debug_print("RGB_Controller::set_program_auto - program=" + String(program_number));
		_last_prog_nmbr = program_number;
		switch (program_number)
		{
		case auto1:
			auto_program1();
			break;
		case auto2:
			auto_program2();
			break;
		case auto3:
			auto_program3();
			break;
		case auto4:
			auto_program4();
			break;
		case auto5:
			auto_program5();
			break;
		case auto6:
			auto_program6();
			break;
		case auto7:
			auto_program7();
			break;
		case auto8:
			auto_program8();
			break;
		case auto9:
			auto_program9();
			break;
		case man_blynk:
			man_program_blynk(rgb_colors_man);
			break;
		case man_pc_rgb:
			man_program_rgb(rgb_colors_man);
			break;
		case man_pc_hsv:
			man_program_hsv(hsv_colors_man);
			break;
		default:
			debug_print("RGB_Controller::set_program_auto - program_number not valid");
			break;
		}
	}
}

void RGB_Controller::set_program(e_prog_nmbr program_number)
{
	if (program_number >= e_prog_nmbr::auto1 && program_number <= e_prog_nmbr::auto9)
	{
		debug_print("RGB_Controller::set_program - run");
		int rgb[3];
		float hsv[3];
		set_program(program_number, rgb, hsv);
	}
	else
	{
		debug_print("RGB_Controller::set_program - invalid program number");
	}
}

void RGB_Controller::auto_program1()
{
	debug_print("RGB_Controller::auto_program1 - run");
	int hue_1 = random(0, 360);
	int hue_2 = random(0, 360);
	int rgb_colors[3];
	while (true)
	{
		_rgb_util.get_rgb(hue_1, 1.0, 1.0, rgb_colors);
		set_rgbled(1, rgb_colors);
		_rgb_util.get_rgb(hue_2, 1.0, 1.0, rgb_colors);
		set_rgbled(2, rgb_colors);
		own_delay(1);
	}
}

void RGB_Controller::auto_program2()
{
	debug_print("prog2 : run");
	int prog2_spd = 3000;
	float var = 0.0;
	int rgb_colors[3];
	while (true)
	{
		if (var <= 360) { var += 10; }
		else var = 0;
		for (float prog2_hue = var; prog2_hue <= 359.0; prog2_hue += 60.0) {
			_rgb_util.get_rgb(prog2_hue, 1.0, 1.0, rgb_colors);
			set_rgbled(1, rgb_colors);
			set_rgbled(2, rgb_colors);
			own_delay(prog2_spd);
		}
		for (float prog2_hue = 359.0 - var; prog2_hue >= 0.0; prog2_hue -= 60.0) {
			_rgb_util.get_rgb(prog2_hue, 1.0, 1.0, rgb_colors);
			set_rgbled(1, rgb_colors);
			set_rgbled(2, rgb_colors);
			own_delay(prog2_spd);
		}
	}
}

void RGB_Controller::auto_program3()
{
	debug_print("prog3 : run");
	int prog3_spd = 500;
	int rgb_colors[3];
	while (true)
	{
		for (float prog3_hue = 0.0; prog3_hue <= 359.0; prog3_hue += 0.5) {
			_rgb_util.get_rgb(prog3_hue, 1.0, 1.0, rgb_colors);
			set_rgbled(1, rgb_colors);
			set_rgbled(2, rgb_colors);
			own_delay(prog3_spd);
		}
		for (float prog3_hue = 359.0; prog3_hue >= 0.0; prog3_hue = -0.5) {
			_rgb_util.get_rgb(prog3_hue, 1.0, 1.0, rgb_colors);
			set_rgbled(1, rgb_colors);
			set_rgbled(2, rgb_colors);
			own_delay(prog3_spd);
		}
	}
}

void RGB_Controller::auto_program4()
{
	debug_print("prog4 : run");
	int prog4_speed = 20;
	int rgb_colors[3];
	while (true)
	{
		for (float h = 0; h <= 359; h += 20) {
			for (float v = 0; v <= 1; v += 0.0025) {
				_rgb_util.get_rgb(h, 1.0, v, rgb_colors);
				set_rgbled(1, rgb_colors);
				set_rgbled(2, rgb_colors);
				own_delay(prog4_speed);
			}
			own_delay(1500);
			for (float v = 1; v >= 0; v -= 0.0025) {
				_rgb_util.get_rgb(h, 1.0, v, rgb_colors);
				set_rgbled(1, rgb_colors);
				set_rgbled(2, rgb_colors);
				own_delay(prog4_speed);
			}
		}
	}
}

void RGB_Controller::auto_program5()
{
	debug_print("prog5 : run");
	int prog5_speed = 40;
	int time = 0;
	int hue = 0;
	int rgb_colors[3];
	while (true)
	{
		hue = random(1, 360);
		for (float v = 0.00001; v <= 1; v += 0.0025) {
			_rgb_util.get_rgb(hue, 1.0, v, rgb_colors);
			set_rgbled(1, rgb_colors);
			set_rgbled(2, rgb_colors);
			time = map(v * 254, 0, 254, 1, 30);
			own_delay(prog5_speed - time);
		}
		own_delay(1500);
		for (float v = 1; v >= 0.00001; v -= 0.0025) {
			_rgb_util.get_rgb(hue, 1.0, v, rgb_colors);
			set_rgbled(1, rgb_colors);
			set_rgbled(2, rgb_colors);
			time = map(v * 254, 0, 254, 30, 1);
			own_delay(prog5_speed - time);
		}
	}
}

void RGB_Controller::auto_program6()
{
	debug_print("prog6 : run");
	int hue = 0;
	int rgb_colors[3];
	while (true)
	{
		if (hue > 360) { hue = hue - 360; }
		for (float s = 0.0; s <= 1.0; s += 0.001) {
			_rgb_util.get_rgb(hue, s, 1.0, rgb_colors);
			set_rgbled(1, rgb_colors);
			_rgb_util.get_rgb(hue, 1.0 - s, 1.0, rgb_colors);
			set_rgbled(2, rgb_colors);
			own_delay(5);
		}
		for (float s = 1.0; s >= 0.0; s -= 0.001) {
			_rgb_util.get_rgb(hue, s, 1.0, rgb_colors);
			set_rgbled(1, rgb_colors);
			_rgb_util.get_rgb(hue, 1.0 - s, 1.0, rgb_colors);
			set_rgbled(2, rgb_colors);
			own_delay(5);
		}
		hue += 50;
	}
}

void RGB_Controller::auto_program7()
{
	debug_print("prog7 : run");
	int hue_offset;
	int prog7_speed = 20;
	int rgb_colors[3];
	while (true)
	{
		for (float h = 0; h <= 359; h += hue_offset) {
			hue_offset = random(0, 45);
			for (float v = 0; v <= 1; v += 0.0025) {
				_rgb_util.get_rgb(h, 1.0, v, rgb_colors);
				set_rgbled(1, rgb_colors);
				_rgb_util.get_rgb(h, 1.0, 1.0 - v, rgb_colors);
				set_rgbled(2, rgb_colors);
				own_delay(prog7_speed);
			}
			own_delay(1500);
			for (float v = 1; v >= 0; v -= 0.0025) {
				_rgb_util.get_rgb(h, 1.0, v, rgb_colors);
				set_rgbled(1, rgb_colors);
				_rgb_util.get_rgb(h, 1.0, 1.0 - v, rgb_colors);
				set_rgbled(2, rgb_colors);
				own_delay(prog7_speed);
			}
		}
	}
}

void RGB_Controller::auto_program8()
{
	debug_print("prog8 : run");
	int prog8_speed = 50;
	float h = 1.0;
	int rgb_colors[3];
	while (true)
	{
		for (float v = 0.0; v <= 1.0; v += 0.0028) {
			h++;
			_rgb_util.get_rgb(h, 1.0, v, rgb_colors);
			set_rgbled(1, rgb_colors);
			set_rgbled(2, rgb_colors);
			own_delay(prog8_speed);
		}
		own_delay(1500);
		for (float v = 1.0; v >= 0.0; v -= 0.0028) {
			h--;
			_rgb_util.get_rgb(h, 1.0, v, rgb_colors);
			set_rgbled(1, rgb_colors);
			set_rgbled(2, rgb_colors);
			own_delay(prog8_speed);
		}
	}
}

void RGB_Controller::auto_program9()
{
	debug_print("prog9 : run");
	float min_val = 0.0;
	float max_val = 1.0;
	float hue = 0.0;
	int delay = 0;
	int rgb_colors[3];
	while (true)
	{
		for (float val = min_val; val <= max_val; val += 0.01) {
			_rgb_util.get_rgb(hue, 1.0, val, rgb_colors);
			set_rgbled(1, rgb_colors);
			set_rgbled(2, rgb_colors);
			delay = 50 - map(pow(val, 3), 0, 1, 1, 50);
			own_delay(delay);
		}
		for (float val = max_val; val >= min_val; val -= 0.01) {
			_rgb_util.get_rgb(hue, 1.0, val, rgb_colors);
			set_rgbled(1, rgb_colors);
			set_rgbled(2, rgb_colors);
			delay = map(pow(val, 3), 0, 1, 1, 50);
			own_delay(delay);
		}
	}
}

void RGB_Controller::man_program_blynk(int rgb_colors[3])
{
	debug_print("prog_man_blynk : run");
	while (true)
	{
		set_rgbled(1, rgb_colors);
		set_rgbled(2, rgb_colors);
		own_delay(1);
	}
}

void RGB_Controller::man_program_rgb(int rgb_colors[3])
{
	debug_print("prog_man_pc_rgb : run");
	while (true)
	{
		set_rgbled(1, rgb_colors);
		set_rgbled(2, rgb_colors);
		own_delay(1);
	}
}

void RGB_Controller::man_program_hsv(float hsv_colors_man[3])
{
	debug_print("prog_man_pc_hsv : run");
	int rgb_colors[3];
	while (true)
	{
		_rgb_util.get_rgb(hsv_colors_man[0], hsv_colors_man[1], hsv_colors_man[2], rgb_colors);
		set_rgbled(1, rgb_colors);
		set_rgbled(2, rgb_colors);
		own_delay(1);
	}
}