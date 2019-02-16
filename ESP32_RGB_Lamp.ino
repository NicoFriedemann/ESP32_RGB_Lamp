#include <Adafruit_PWMServoDriver.h>
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include "Enums.h"
#include "RGB_Controller.h"
#include "UDP_Handler.h"
#include "login_credentials.h"
#include "UDP_Message_Handler.h"

void debug_print(String msg, e_debug_level e_dl);
void add_udp_msg_receiver(udp_receiver udp_rec);
void set_program(e_prog_nmbr program_number, int rgb_colors_man[3], float hsv_colors_man[3]);
int ota_progress = 0;
int ota_progress_old = 0;
bool setup_finished = false;
const boolean DEBUG_SERIAL = true;
const boolean DEBUG_UDP = true;
const e_debug_level DEBUG_DL = e_debug_level::dl_debug;
unsigned long cyclictask_last_run = 0;
UDP_Handler *udp_handler;
RGB_Controller *rgb;
UDP_Message_Handler *udp_msg_handler;

void setup() {
	debug_print("Main::setup - setup started", e_debug_level::dl_info);
	if (DEBUG_SERIAL) {
		Serial.begin(115200);
		delay(1000);
	}
	udp_handler = new UDP_Handler(&debug_print, WiFi,11001);
	rgb = new RGB_Controller(&debug_print, &own_delay, 4, 0, 2, 1, 2, 3, 4, 5, 6);
	udp_msg_handler = new UDP_Message_Handler(&debug_print, &set_program, &add_udp_msg_receiver);
	
	if (connect_wlan()==wl_status_t::WL_CONNECTED)
	{
		start_esp32_ota();
		udp_handler->start_udp_receiver();
		setup_finished = true;
	}

	rgb->set_program((e_prog_nmbr)random(1, 9));
	rgb->set_esp32_statusled_on();
}

wl_status_t connect_wlan()
{
	int wlan_restart_count = 0;
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	while (WiFi.waitForConnectResult() != WL_CONNECTED) {
		debug_print("Main::connect_wlan - wlan connection failed!", e_debug_level::dl_error);
		delay(5000);
		ESP.restart();
		wlan_restart_count += 1;
		if (wlan_restart_count >= MAX_WLAN_CONNECT_RETRY) {
			return wl_status_t::WL_CONNECT_FAILED;
			break;
		}
	}
	debug_print("Main::connect_wlan - connection established!", e_debug_level::dl_info);
	return wl_status_t::WL_CONNECTED;
}

void start_esp32_ota()
{
	if (WiFi.isConnected()) {
		ArduinoOTA.setPassword("esP32");
		ArduinoOTA.onStart([]() {
			debug_print("Main::OTA - start updating", e_debug_level::dl_info);
		});
		ArduinoOTA.onEnd([]() {
			debug_print("Main::OTA - end updating", e_debug_level::dl_info);
		});
		ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
			ota_progress = (progress / (total / 100));
			if (ota_progress > ota_progress_old)
			{
				ota_progress_old = ota_progress;
				debug_print("Main::OTA - progress:" + String(ota_progress) + "%", e_debug_level::dl_info);
			}
		});
		ArduinoOTA.onError([](ota_error_t error) {
			debug_print("Main::OTA - Error:" + String(error), e_debug_level::dl_error);
			if (error == OTA_AUTH_ERROR) debug_print("Main::OTA - auth failed", e_debug_level::dl_error);
			else if (error == OTA_BEGIN_ERROR) debug_print("Main::OTA - begin failed", e_debug_level::dl_error);
			else if (error == OTA_CONNECT_ERROR) debug_print("Main::OTA - connect failed", e_debug_level::dl_error);
			else if (error == OTA_RECEIVE_ERROR) debug_print("Main::OTA - receive failed", e_debug_level::dl_error);
			else if (error == OTA_END_ERROR) debug_print("Main::OTA - end failed", e_debug_level::dl_error);
		});
		ArduinoOTA.begin();
	}
}

void own_delay(int milli) {
	unsigned long start_time = millis();
	unsigned long end_time = start_time + milli;
	while (millis() < end_time) {
		cyclic_tasks();
	}
}

void cyclic_tasks() {
	if (cyclictask_last_run <= (millis() - 500)) {
		debug_print("Main::cyclic_tasks - run", e_debug_level::dl_info);
		rgb->cyclic_tasks();
		cyclictask_last_run = millis();
	}
	udp_message udp_msg;
	udp_msg = udp_handler->receive_udp_msg();
	if (udp_msg.msg != "")
	{
		udp_msg_handler->parse_udp_cmd_msg(udp_msg);
	}
	ArduinoOTA.handle();
}

void debug_print(String msg, e_debug_level dl)
{
	String header;
	if (DEBUG_SERIAL) {
		Serial.println(msg);
	}
	if (DEBUG_UDP && setup_finished) {
		udp_handler->send_debug_msg(msg,dl);
	}
}

void add_udp_msg_receiver(udp_receiver udp_rec)
{
	udp_handler->add_udp_msg_receiver(udp_rec);
}

void set_program(e_prog_nmbr program_number, int rgb_colors_man[3], float hsv_colors_man[3]) {
	rgb->set_program(program_number, rgb_colors_man, hsv_colors_man);
}

void loop() {
	cyclic_tasks();
}
