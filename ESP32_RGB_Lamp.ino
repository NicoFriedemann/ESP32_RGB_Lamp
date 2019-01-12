#include <Adafruit_PWMServoDriver.h>
#include "UDP_Connection.h"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include "Enums.h"
#include "RGB_Controller.h"
#include "UDP_Connection.h"
#include "login_credentials.h"
#include "UDP_Message_Handler.h"

void debug_print(String msg);
void add_udp_msg_receiver(udp_receiver udp_rec);
void set_program(e_prog_nmbr program_number, int rgb_colors_man[3], float hsv_colors_man[3]);
const boolean DEBUG_SERIAL = true;
const boolean DEBUG_UDP = true;
unsigned long cyclictask_last_run = 0;
UDP_Connection *udp_connection;
RGB_Controller *rgb;
UDP_Message_Handler *udp_handler;

void setup() {
	debug_print("Main::setup - setup started");
	if (DEBUG_SERIAL) {
		Serial.begin(115200);
		delay(1000);
	}
	udp_connection = new UDP_Connection(&debug_print, WiFi);
	rgb = new RGB_Controller(&debug_print, &own_delay, 4, 0, 2, 1, 2, 3, 4, 5, 6, 4095);
	udp_handler = new UDP_Message_Handler(&debug_print, &set_program, &add_udp_msg_receiver, 4095);
	
	connect_wlan();
	start_esp32_ota();
	add_command_sender();

	rgb->set_program((e_prog_nmbr)random(1, 9));
	rgb->set_esp32_statusled_on();
}

void connect_wlan()
{
	int wlan_restart_count = 0;
	int MAX_WLAN_CONNECT_RETRY = 3;
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	while (WiFi.waitForConnectResult() != WL_CONNECTED) {
		debug_print("Main::connect_wlan - wlan connection failed!");
		delay(5000);
		ESP.restart();
		wlan_restart_count += 1;
		if (wlan_restart_count >= MAX_WLAN_CONNECT_RETRY) {
			break;
		}
	}
	debug_print("Main::connect_wlan - connection established!");
}

//add first remote command sender
void add_command_sender() {
	udp_receiver rec;
	rec.is_active = true;
	rec.udp_ipaddress_partner = "192.168.178.20";
	rec.udp_port_partner = 11000;
	udp_connection->add_udp_msg_receiver(rec);
}

void start_esp32_ota()
{
	if (WiFi.isConnected()) {
		ArduinoOTA.setPassword("esP32");
		ArduinoOTA.onStart([]() {
			debug_print("Main::OTA - start updating");
		});
		ArduinoOTA.onEnd([]() {
			debug_print("Main::OTA - end updating");
		});
		ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
			int progress_percent = (progress / (total / 100));
			debug_print("Main::OTA - progress:" + String(progress_percent) + "%");
		});
		ArduinoOTA.onError([](ota_error_t error) {
			debug_print("Main::OTA - Error:" + String(error));
			if (error == OTA_AUTH_ERROR) debug_print("Main::OTA - auth failed");
			else if (error == OTA_BEGIN_ERROR) debug_print("Main::OTA - begin failed");
			else if (error == OTA_CONNECT_ERROR) debug_print("Main::OTA - connect failed");
			else if (error == OTA_RECEIVE_ERROR) debug_print("Main::OTA - receive failed");
			else if (error == OTA_END_ERROR) debug_print("Main::OTA - end failed");
		});
		ArduinoOTA.begin();
	}
}

void own_delay(int milli) {
	unsigned long start_time = millis();
	unsigned long end_time = start_time + milli;
	while (millis() < end_time) {
		cyclic_tasks();
		delay(1);
	}
}

void cyclic_tasks() {
	if (cyclictask_last_run <= (millis() - 500)) {
		debug_print("Main::cyclic_tasks - run");
		rgb->cyclic_tasks();
		cyclictask_last_run = millis();
	}
	udp_message udp_msg;
	udp_msg = udp_connection->receive_udp_msg(1);
	if (udp_msg.msg != "")
	{
		udp_handler->handle_udp_cmd_msg(udp_msg);
	}
	ArduinoOTA.handle();
}

void debug_print(String msg)
{
	if (DEBUG_SERIAL) {
		Serial.println(msg);
	}
	if (DEBUG_UDP) {
		udp_connection->send_udp_msg("DEBUG", msg);
	}
}

void add_udp_msg_receiver(udp_receiver udp_rec)
{
	udp_connection->add_udp_msg_receiver(udp_rec);
}

void set_program(e_prog_nmbr program_number, int rgb_colors_man[3], float hsv_colors_man[3]) {
	rgb->set_program(program_number, rgb_colors_man, hsv_colors_man);
}

void loop() {
	cyclic_tasks();
}
