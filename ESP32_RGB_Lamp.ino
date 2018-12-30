#include <Adafruit_PWMServoDriver.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <math.h>
#include <Arduino.h>
#include <BlynkSimpleEsp32.h>
#include "login_credentials.h"

enum e_prog_nmbr {
	undefined_prog_nmbr,
	auto1,
	auto2,
	auto3,
	auto4,
	auto5,
	auto6,
	auto7,
	auto8,
	auto9,
	man_blynk,
	man_pc_rgb,
	man_pc_hsv,
	element_count_prog_nmbr,
};

enum e_udpmsg_type {
	undefined_udpmsg_type,
	debug,
	data,
	control,
	element_count_udpmsg_type,
};

enum e_udpmsg_cmd {
	undefined_udpmsg_cmd,
	change_prog,
	change_color,
	element_count_udpmsg_cmd,
};

enum e_udpmsg_parname {
	undefined_udpmsg_parname,
	program_number,
	red,
	green,
	blue,
	hue,
	saturation,
	value,
	element_count_udpmsg_parname,
};

struct hsv {
	float h;       // angle in degrees
	float s;       // a fraction between 0 and 1
	float v;       // a fraction between 0 and 1
};

struct rgb {
	float r;       // a fraction between 0 and 1
	float g;       // a fraction between 0 and 1
	float b;       // a fraction between 0 and 1
};

void debug_print(String msg);
float check_hsv(float val, e_udpmsg_parname par_name);
void send_udp_debug_msg(String msg);
void own_delay(int milli);
void send_udp_msg(String header, String msg);
String recv_udp_msg();
int check_rgb(int val);
float check_hsv(float val,e_udpmsg_parname par_name);
void get_command_udpmsg(String msg);
bool validate_command_udpmsg(String msg);
int get_char_occurrences_string(String search_string, char charcter);
float get_next_element_udp_msg(String &msg);
float get_dimval(float lightness);
struct rgb convert_hsv2rgb(struct hsv in);
struct hsv convert_rgb2hsv(struct rgb in);
void get_rgb(float hue, float sat, float val, int colors[3]);
void check_watchdog();
void clear_outputs();
void set_statusled_on();
void set_statusled_off();
void set_rgbled(int led_nmbr, int rgb_color[3]);
void prog1();
void prog2();
void prog3();
void prog4();
void prog5();
void prog6();
void prog7();
void prog8();
void prog9();
void prog_man_blynk();
void prog_man_pc_rgb();
void prog_man_pc_hsv();
void set_program();
void cyclic_tasks();

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
const int watchdog_pin_esp32 = 4;
const int LED_BUILTIN = 2;
const int watchdog_pin_pwmboard = 0;
const int led1_pin_r = 1;
const int led1_pin_g = 2;
const int led1_pin_b = 3;
const int led2_pin_r = 4;
const int led2_pin_g = 5;
const int led2_pin_b = 6;
const int max_wlan_connect_retry = 3;
const int max_val = 4095;
const int udp_port_partner = 11000;
const char* udp_address_partner = "192.168.178.20";
const int udp_port_esp32 = 11001;
const boolean DEBUG_SETUP = false;
const boolean DEBUG_SERIAL = false;
const boolean DEBUG_UDP = true;
const boolean BLYNK = true;


bool watchdog_refresh_state = false;
unsigned long watchdog_refresh_last_change = 0;
bool watchdog_state_check = false;
int watchdog_last_change = 0;
int watchdog_fail_counts = 0;
unsigned long cyclictask_last_run = 0;
e_prog_nmbr prog_nmbr = e_prog_nmbr::auto1;
e_prog_nmbr last_prog_nmbr = e_prog_nmbr::man_pc_rgb;
int rgb_colors[3] = { max_val,max_val,max_val };
int rgb_colors_man[3] = { max_val,max_val,max_val };
float hsv_colors_man[3] = { 240.0,1.0,1.0};
WiFiUDP udp_receiver;
WiFiUDP udp_sender;


void setup() {
  int restart_count = 0;
  if (DEBUG_SERIAL) {
    Serial.begin(115200);
    delay(1000);  
  }
	debug_print("");
	debug_print("");
	debug_print("#################");
	debug_print("setup : started!");
	debug_print("setup : chip temperature=" + String(temperatureRead()) + " Grad");
	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(watchdog_pin_esp32, INPUT);
	pwm.begin();
	clear_outputs();
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	while (WiFi.waitForConnectResult() != WL_CONNECTED) {
		debug_print("setup :  wlan connection failed!");
		own_delay(5000);
		ESP.restart();
    restart_count+=1;
    if (restart_count>=max_wlan_connect_retry){
      break;
    }
	}
	udp_receiver.begin(udp_port_esp32);
  if (BLYNK) {
    Blynk.config(auth);
    Blynk.connect();  
  }
	ArduinoOTA.setPassword("esP32");
	ArduinoOTA.onStart([]() {
		debug_print("OTA : start updating");
	});
	ArduinoOTA.onEnd([]() {
		debug_print("OTA : end updating");
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		int progress_percent = (progress / (total / 100));
		debug_print("OTA : progress " + String(progress_percent) + "%");
	});
	ArduinoOTA.onError([](ota_error_t error) {
		debug_print("OTA : Error " + String(error));
		if (error == OTA_AUTH_ERROR) debug_print("OTA : auth failed");
		else if (error == OTA_BEGIN_ERROR) debug_print("OTA : begin failed");
		else if (error == OTA_CONNECT_ERROR) debug_print("OTA : connect failed");
		else if (error == OTA_RECEIVE_ERROR) debug_print("OTA : receive failed");
		else if (error == OTA_END_ERROR) debug_print("OTA : end failed");
	});
	ArduinoOTA.begin();
	debug_print("setup : wlan connection established!");
	debug_print("setup : finished!");
	set_statusled_on();
	randomSeed(analogRead(22));
	prog_nmbr = (e_prog_nmbr)random(1, 9); //automatic programs only till nr. 9
	last_prog_nmbr = element_count_prog_nmbr;
}

void own_delay(int milli) {
	unsigned long start_time = millis();
	unsigned long end_time = start_time + milli;
	while (millis() < end_time) {
		cyclic_tasks();
		delay(1);
	}
}

void send_udp_debug_msg(String msg) {
	send_udp_msg("DEBUG", msg);
}

void send_udp_msg(String header, String msg) {
	if (WiFi.isConnected())
	{
		String _msg = "";
		_msg = "|" + header + " | " + msg + " |";
		const char * c = _msg.c_str();
		udp_sender.beginPacket(udp_address_partner, udp_port_partner);
		udp_sender.printf(c);
		udp_sender.endPacket();
	}
}

String recv_udp_msg() {
	char packetBuffer[255];
	if (WiFi.isConnected())
	{
		int packetSize = udp_receiver.parsePacket();
		String msg = "";
		if (packetSize)
		{
			int len = udp_receiver.read(packetBuffer, sizeof(packetBuffer));
			if (len > 0) {
				packetBuffer[len] = 0;
			}
			msg = String(packetBuffer);
			debug_print("recv_udp_msg : msg received=" + msg);
			get_command_udpmsg(msg);
		}
		return msg;
	}
}

int check_rgb(int val) {
	if (val > max_val) {return max_val;}
	else if (val < 0) {	return 0;}
	else{return val;}
}

float check_hsv(float val,e_udpmsg_parname par_name) {
	if (val > 1.0 && (par_name == e_udpmsg_parname::saturation || par_name == e_udpmsg_parname::value)) {return 1.0;}
	else if (val > 360.0 && (par_name == e_udpmsg_parname::hue)) { return 360.0; }
	else if (val < 0.0) {return 0.0;}
	else {return val;}
}

void get_command_udpmsg(String msg) {
	debug_print("get_command_udpmsg : run");
	e_udpmsg_cmd cmd;
	e_udpmsg_parname par_name;
	float par_val = 0;
	if (validate_command_udpmsg(msg))
	{
		debug_print("get_command_udpmsg : invalid udpmsg=" + msg);
		return;
	}
	cmd = (e_udpmsg_cmd)int(get_next_element_udp_msg(msg));

	switch (cmd)
	{
	case change_prog:
		debug_print("get_command_udpmsg : change prog");
		par_name = (e_udpmsg_parname)int(get_next_element_udp_msg(msg));
		if (par_name == program_number)
		{
			par_val = get_next_element_udp_msg(msg);
			prog_nmbr = (e_prog_nmbr)int(par_val);
			set_program();
		}
		break;
	case change_color:
		debug_print("get_command_udpmsg : change color");
		do
		{
			par_name = (e_udpmsg_parname)int(get_next_element_udp_msg(msg));
			par_val = get_next_element_udp_msg(msg);
			switch (par_name)
			{
			case undefined_udpmsg_parname:
				break;
			case red:
				if (prog_nmbr!=e_prog_nmbr::man_pc_rgb)
				{
					debug_print("get_command_udpmsg : error -> changed red -> prog=" + String(prog_nmbr));
				}
				rgb_colors_man[0] = check_rgb(max_val - int(par_val));
				break;
			case green:
				if (prog_nmbr != e_prog_nmbr::man_pc_rgb)
				{
					debug_print("get_command_udpmsg : error -> changed green -> prog=" + String(prog_nmbr));
				}
				rgb_colors_man[1] = check_rgb(max_val - int(par_val));
				break;
			case blue:
				if (prog_nmbr != e_prog_nmbr::man_pc_rgb)
				{
					debug_print("get_command_udpmsg : error -> changed blue -> prog=" + String(prog_nmbr));
				}
				rgb_colors_man[2] = check_rgb(max_val - int(par_val));
				break;
			case hue:
				if (prog_nmbr != e_prog_nmbr::man_pc_hsv)
				{
					debug_print("get_command_udpmsg : error -> changed hue -> prog=" + String(prog_nmbr));
				}
				hsv_colors_man[0] = check_hsv(par_val,hue);
				break;
			case saturation:
				if (prog_nmbr != e_prog_nmbr::man_pc_hsv)
				{
					debug_print("get_command_udpmsg : error -> changed saturation -> prog=" + String(prog_nmbr));
				}
				hsv_colors_man[1] = check_hsv(par_val,saturation);
				break;
			case value:
				if (prog_nmbr != e_prog_nmbr::man_pc_hsv)
				{
					debug_print("get_command_udpmsg : error -> changed value -> prog=" + String(prog_nmbr));
				}
				hsv_colors_man[2] = check_hsv(par_val,value);
				break;
			default:
				break;
			}
		} while (msg.length() > 0);
		break;	
	default:
		debug_print("get_command_udpmsg : invalid cmd=" + String(cmd));
		break;
	}
}

bool validate_command_udpmsg(String msg) {
	bool out = false;
	int i = get_char_occurrences_string(msg, '<');
	int y = get_char_occurrences_string(msg, '>');

	if (i != y || i < 3 || y < 3)
	{
		debug_print("validate_command_udpmsg : error i=" + String(i) + " y=" + String(y));
		out = true;
	}
	return out;
}

int get_char_occurrences_string(String search_string, char charcter) {
	int i = 0;
	const char * str = search_string.c_str();
	for (i = 0; str[i]; str[i] == charcter ? i++ : *str++);
	return i;
}

float get_next_element_udp_msg(String &msg) {
	int first_pos_val = 0;
	int last_pos_val = 0;
	String val = "";
	String msg_old = msg;
	float out = 0.0;
	first_pos_val = msg.indexOf("<");
	last_pos_val = msg.indexOf(">");
	val = msg.substring(first_pos_val + 1, last_pos_val);
	msg = msg.substring(last_pos_val + 1);
	out = val.toFloat();
	return out;
}

//input 0-1
float get_dimval(float lightness) {
	float luminance = 0.0;
	float _lightness = lightness * 100.0;
	if (_lightness >= 0.0 && _lightness <= 100.0) {
		if (_lightness <= 8.0) {luminance = _lightness / 902.3;}
		else {luminance = pow(((_lightness + 16.0) / 116.0),3.0);}
	}
	else {luminance = _lightness;}
	return luminance;
}

struct rgb convert_hsv2rgb(struct hsv in){
	float      hh, p, q, t, ff;
	int        i;
	rgb         out;

	in.v = get_dimval(in.v);

	if (in.s <= 0.0) {
		out.r = in.v;
		out.g = in.v;
		out.b = in.v;
		return out;
	}
	hh = in.h;
	if (hh >= 360.0) hh = 0.0;
	hh /= 60.0;
	i = (int)hh;
	ff = hh - i;
	p = in.v * (1.0 - in.s);
	q = in.v * (1.0 - (in.s * ff));
	t = in.v * (1.0 - (in.s * (1.0 - ff)));
	
	switch (i) {
	case 0:
		out.r = in.v;
		out.g = t;
		out.b = p;
		break;
	case 1:
		out.r = q;
		out.g = in.v;
		out.b = p;
		break;
	case 2:
		out.r = p;
		out.g = in.v;
		out.b = t;
		break;

	case 3:
		out.r = p;
		out.g = q;
		out.b = in.v;
		break;
	case 4:
		out.r = t;
		out.g = p;
		out.b = in.v;
		break;
	case 5:
	default:
		out.r = in.v;
		out.g = p;
		out.b = q;
		break;
	}
	
	out.r = 1.0 - out.r;
	out.g = 1.0 - out.g;
	out.b = 1.0 - out.b;
	return out;
}

struct hsv convert_rgb2hsv(struct rgb in)
{
	hsv         out;
	float      min, max, delta;

	min = in.r < in.g ? in.r : in.g;
	min = min  < in.b ? min : in.b;

	max = in.r > in.g ? in.r : in.g;
	max = max  > in.b ? max : in.b;

	out.v = max;                          
	delta = max - min;
	if (delta < 0.00001)
	{
		out.s = 0;
		out.h = 0;
		return out;
	}
	if (max > 0.0) { 
		out.s = (delta / max);                
	}
	else {
		out.s = 0.0;
		out.h = NAN;                           
		return out;
	}
	if (in.r >= max)                          
		out.h = (in.g - in.b) / delta;        
	else
		if (in.g >= max)
			out.h = 2.0 + (in.b - in.r) / delta;  
		else
			out.h = 4.0 + (in.r - in.g) / delta;  

	out.h *= 60.0;                             

	if (out.h < 0.0)
		out.h += 360.0;
	
	return out;
}

void get_rgb(float hue, float sat, float val, int colors[3]) {
	struct rgb _rgb;
	struct hsv _hsv;
	_hsv.h = check_hsv(hue, e_udpmsg_parname::hue);
	_hsv.s = check_hsv(sat, e_udpmsg_parname::saturation);
	_hsv.v = check_hsv(val, e_udpmsg_parname::value);
	_rgb = convert_hsv2rgb(_hsv);
	if (_rgb.r > 0.0) { colors[0] = _rgb.r * max_val; } else { colors[0] = 0; }
	if (_rgb.g > 0.0) { colors[1] = _rgb.g * max_val; } else { colors[1] = 0; }
	if (_rgb.b > 0.0) { colors[2] = _rgb.b * max_val; }	else { colors[2] = 0; }
}

void refresh_watchdog() {
	if (watchdog_refresh_last_change <= (millis() - 1000)) {
		if (watchdog_refresh_state) {
			pwm.setPWM(watchdog_pin_pwmboard, 0, 0);
			watchdog_refresh_state = false;
		}
		else {
			pwm.setPWM(watchdog_pin_pwmboard, 0, max_val);
			watchdog_refresh_state = true;
		}
		watchdog_refresh_last_change = millis();
	}
}

void check_watchdog() {
	if (watchdog_last_change <= (millis() - 1000)) {
		int state_watchdog_pin = digitalRead(watchdog_pin_esp32);
		if (state_watchdog_pin != watchdog_state_check) {
			watchdog_fail_counts = 0;
			set_statusled_on();
			watchdog_state_check = state_watchdog_pin;
			debug_print("check_watchdog : ok");

		}
		else {
			debug_print("check_watchdog : fail");
			watchdog_fail_counts++;
		}
		watchdog_last_change = millis();
	}
	if (watchdog_fail_counts>10) {
		debug_print("check_watchdog : watchog triggered -> wait 5s -> reset!");
		set_statusled_off();
		delay(5000);
		ESP.restart();
	}
}

void clear_outputs() {
	digitalWrite(LED_BUILTIN, LOW);
	pwm.setPin(watchdog_pin_pwmboard, max_val);
	pwm.setPin(led1_pin_r, max_val);
	pwm.setPin(led1_pin_g, max_val);
	pwm.setPin(led1_pin_b, max_val);
	pwm.setPin(led2_pin_r, max_val);
	pwm.setPin(led2_pin_g, max_val);
	pwm.setPin(led2_pin_b, max_val);
}

void set_statusled_on() {
	digitalWrite(LED_BUILTIN, HIGH);
}

void set_statusled_off() {
	digitalWrite(LED_BUILTIN, LOW);
}

void set_rgbled(int led_nmbr, int rgb_color[3]) {
	if (led_nmbr == 1) {
		pwm.setPin(led1_pin_r, rgb_color[0]);
		pwm.setPin(led1_pin_g, rgb_color[1]);
		pwm.setPin(led1_pin_b, rgb_color[2]);
	}
	else if (led_nmbr == 2) {
		pwm.setPin(led2_pin_r, rgb_color[0]);
		pwm.setPin(led2_pin_g, rgb_color[1]);
		pwm.setPin(led2_pin_b, rgb_color[2]);
	}
}

void prog1() {
	debug_print("prog1 : run");
	int hue_1 = random(0, 360);
	int hue_2 = random(0, 360);
	while (true)
	{
		get_rgb(hue_1, 1.0, 1.0, rgb_colors);
		set_rgbled(1, rgb_colors);
		get_rgb(hue_2, 1.0, 1.0, rgb_colors);
		set_rgbled(2, rgb_colors);
		own_delay(1);
	}
}

void prog2() {
	debug_print("prog2 : run");
	int prog2_spd = 3000;
	float var = 0.0;
	while (true)
	{
		if (var <= 360) { var += 10; }
		else var = 0;
		for (float prog2_hue = var; prog2_hue <= 359.0; prog2_hue += 60.0) {
			get_rgb(prog2_hue, 1.0, 1.0, rgb_colors);
			set_rgbled(1, rgb_colors);
			set_rgbled(2, rgb_colors);
			own_delay(prog2_spd);
		}
		for (float prog2_hue = 359.0 - var; prog2_hue >= 0.0; prog2_hue -= 60.0) {
			get_rgb(prog2_hue, 1.0, 1.0, rgb_colors);
			set_rgbled(1, rgb_colors);
			set_rgbled(2, rgb_colors);
			own_delay(prog2_spd);
		}
	}
}

void prog3() {
	debug_print("prog3 : run");
	int prog3_spd = 500;
	while (true)
	{
		for (float prog3_hue = 0.0; prog3_hue <= 359.0; prog3_hue += 0.5) {
			get_rgb(prog3_hue, 1.0, 1.0, rgb_colors);
			set_rgbled(1, rgb_colors);
			set_rgbled(2, rgb_colors);
			own_delay(prog3_spd);
		}
		for (float prog3_hue = 359.0; prog3_hue >= 0.0; prog3_hue = -0.5) {
			get_rgb(prog3_hue, 1.0, 1.0, rgb_colors);
			set_rgbled(1, rgb_colors);
			set_rgbled(2, rgb_colors);
			own_delay(prog3_spd);
		}
	}
}

void prog4() {
	debug_print("prog4 : run");
	int prog4_speed = 20;
	while (true)
	{
		for (float h = 0; h <= 359; h += 20) {
			for (float v = 0; v <= 1; v += 0.0025) {
				get_rgb(h, 1.0, v, rgb_colors);
				set_rgbled(1, rgb_colors);
				set_rgbled(2, rgb_colors);
				own_delay(prog4_speed);
			}
			own_delay(1500);
			for (float v = 1; v >= 0; v -= 0.0025) {
				get_rgb(h, 1.0, v, rgb_colors);
				set_rgbled(1, rgb_colors);
				set_rgbled(2, rgb_colors);
				own_delay(prog4_speed);
			}
		}
	}
}

void prog5() {
	debug_print("prog5 : run");
	int prog5_speed = 40;
	int time;
	int hue;
	while (true)
	{
		hue = random(1, 360);
		for (float v = 0.00001; v <= 1; v += 0.0025) {
			get_rgb(hue, 1.0, v, rgb_colors);
			set_rgbled(1, rgb_colors);
			set_rgbled(2, rgb_colors);
			time = map(v * 254, 0, 254, 1, 30);
			own_delay(prog5_speed - time);
		}
		own_delay(1500);
		for (float v = 1; v >= 0.00001; v -= 0.0025) {
			get_rgb(hue, 1.0, v, rgb_colors);
			set_rgbled(1, rgb_colors);
			set_rgbled(2, rgb_colors);
			time = map(v * 254, 0, 254, 30, 1);
			own_delay(prog5_speed - time);
		}
	}
}

void prog6() {
	debug_print("prog6 : run");
	int hue = 0;
	while (true)
	{
		if (hue > 360) {hue = hue - 360;}
		for (float s = 0.0; s <= 1.0; s += 0.001) {
			get_rgb(hue, s, 1.0, rgb_colors);
			set_rgbled(1, rgb_colors);
			get_rgb(hue, 1.0 - s, 1.0, rgb_colors);
			set_rgbled(2, rgb_colors);
			own_delay(5);
		}
		for (float s = 1.0; s >= 0.0; s -= 0.001) {
			get_rgb(hue, s, 1.0, rgb_colors);
			set_rgbled(1, rgb_colors);
			get_rgb(hue, 1.0 - s, 1.0, rgb_colors);
			set_rgbled(2, rgb_colors);
			own_delay(5);
		}
		hue += 50;
	}
}

void prog7() {
	debug_print("prog7 : run");
	int hue_offset;
	int prog7_speed = 20;
	while (true)
	{
		for (float h = 0; h <= 359; h += hue_offset) {
			hue_offset = random(0, 45);
			for (float v = 0; v <= 1; v += 0.0025) {
				get_rgb(h, 1.0, v, rgb_colors);
				set_rgbled(1, rgb_colors);
				get_rgb(h, 1.0, 1.0 - v, rgb_colors);
				set_rgbled(2, rgb_colors);
				own_delay(prog7_speed);
			}
			own_delay(1500);
			for (float v = 1; v >= 0; v -= 0.0025) {
				get_rgb(h, 1.0, v, rgb_colors);
				set_rgbled(1, rgb_colors);
				get_rgb(h, 1.0, 1.0 - v, rgb_colors);
				set_rgbled(2, rgb_colors);
				own_delay(prog7_speed);
			}
		}
	}
}

void prog8() {
	debug_print("prog8 : run");
	int prog8_speed = 50;
	float h = 1.0;
	while (true)
	{
		for (float v = 0.0; v <= 1.0; v+=0.0028) {
			h++;
			get_rgb(h, 1.0, v, rgb_colors);
			set_rgbled(1, rgb_colors);
			set_rgbled(2, rgb_colors);
			own_delay(prog8_speed);
		}
		own_delay(1500);
		for (float v = 1.0; v >= 0.0; v-=0.0028) {
			h--;
			get_rgb(h, 1.0, v, rgb_colors);
			set_rgbled(1, rgb_colors);
			set_rgbled(2, rgb_colors);
			own_delay(prog8_speed);
		}
	}
}

void prog9() {
	debug_print("prog9 : run");
	float min_val = 0.0;
	float max_val = 1.0;
	float hue = 0.0;
	int delay = 0;
	while (true)
	{
		for (float val = min_val; val <= max_val; val += 0.01) {
			get_rgb(hue, 1.0, val, rgb_colors);
			set_rgbled(1, rgb_colors);
			set_rgbled(2, rgb_colors);
			delay = 50 - map(pow(val, 3), 0, 1, 1, 50);
			own_delay(delay);
		}
		for (float val = max_val; val >= min_val; val -= 0.01) {
			get_rgb(hue, 1.0, val, rgb_colors);
			set_rgbled(1, rgb_colors);
			set_rgbled(2, rgb_colors);
			delay = map(pow(val, 3), 0, 1, 1, 50);
			own_delay(delay);
		}
	}
}

void prog_man_blynk() {
	debug_print("prog_man_blynk : run");
	while (true)
	{
		set_rgbled(1, rgb_colors_man);
		set_rgbled(2, rgb_colors_man);
		own_delay(1);
	}
}

void prog_man_pc_rgb() {
	debug_print("prog_man_pc_rgb : run");
	while (true)
	{
		set_rgbled(1, rgb_colors_man);
		set_rgbled(2, rgb_colors_man);
		own_delay(1);
	}
}

void prog_man_pc_hsv() {
	debug_print("prog_man_pc_hsv : run");
	while (true)
	{
		get_rgb(hsv_colors_man[0], hsv_colors_man[1], hsv_colors_man[2], rgb_colors_man);
		set_rgbled(1, rgb_colors_man);
		set_rgbled(2, rgb_colors_man);
		own_delay(1);
	}
}

void set_program() {
	if (prog_nmbr != last_prog_nmbr) {
		debug_print("set_program : prog=" + String(prog_nmbr));
		last_prog_nmbr = prog_nmbr;
		switch (prog_nmbr)
		{
		case auto1:
			prog1();
			break;
		case auto2:
			prog2();
			break;
		case auto3:
			prog3();
			break;
		case auto4:
			prog4();
			break;
		case auto5:
			prog5();
			break;
		case auto6:
			prog6();
			break;
		case auto7:
			prog7();
			break;
		case auto8:
			prog8();
			break;
		case auto9:
			prog9();
			break;
		case man_blynk:
			prog_man_blynk();
			break;
		case man_pc_rgb:
			prog_man_pc_rgb();
			break;
		case man_pc_hsv:
			prog_man_pc_hsv();
			break;
		default:
			break;
		}
	}	
}

BLYNK_WRITE(V1) {
	debug_print("BLYNK_WRITE(V1) : run");
	int ledValue = param.asInt();
	prog_nmbr = (e_prog_nmbr)ledValue;
	debug_print("BLYNK_WRITE(V1) : prog_nmbr=" + String(prog_nmbr));
	cyclic_tasks();
}

BLYNK_WRITE(V2) {
	debug_print("BLYNK_WRITE(V2) : run");
	rgb_colors_man[0] = check_rgb(max_val - param[0].asInt());
	rgb_colors_man[1] = check_rgb(max_val - param[1].asInt());
	rgb_colors_man[2] = check_rgb(max_val - param[2].asInt());
	debug_print("BLYNK_WRITE(V2) : r=" + String(rgb_colors_man[0]) + " g=" + String(rgb_colors_man[1]) + " b=" + String(rgb_colors_man[2]));
}

void cyclic_tasks() {
	if (cyclictask_last_run <= (millis() - 500)) {
		debug_print("cyclic_tasks : run");
		refresh_watchdog();
		check_watchdog();
		ArduinoOTA.handle();
		set_program();
		cyclictask_last_run = millis();
	}
	Blynk.run();
	recv_udp_msg();
}

void debug_print(String msg){  
  if (DEBUG_SERIAL) {
    Serial.println(msg);
  }
  if (DEBUG_UDP) {
    send_udp_debug_msg(msg);
  }
}

void loop() {
	debug_print("loop : started!");
	cyclic_tasks();
}
