#pragma once
#include <WiFiUdp.h>
#include <WiFi.h>
#include "Enums.h"
#include <ArduinoJson.h>

struct udp_receiver {
public:
	int udp_port_partner;
	String udp_ipaddress_partner;
	e_debug_level udp_debug_level;
	e_udpmsg_type udpmsg_type;
	boolean is_active;
	udp_receiver() {
		udpmsg_type = e_udpmsg_type::pos_based_format;
		udp_port_partner = 0;
		udp_ipaddress_partner = "0.0.0.0";
		is_active = false;
		udp_debug_level = e_debug_level::dl_info;
	};
};

struct udp_message {
public:
	udp_receiver udp_rec;
	String msg;
	udp_message() {
		msg = "";
		udp_rec = udp_receiver();
	}
};

class UDP_Handler {
public:
	UDP_Handler(void(*debug_print_fncptr)(String,e_debug_level), WiFiClass &wifi, int udp_receive_port);
	void send_udp_msg(String msg, udp_receiver udp_rec);
	void send_debug_msg(String msg, e_debug_level dl);
	void start_udp_receiver();
	void stop_udp_receiver();
	udp_message receive_udp_msg();
	void add_udp_msg_receiver(udp_receiver udp_rec);
private:
	void(*debug_print)(String msg, e_debug_level e_dl);
	e_udpmsg_type get_udp_msg_format(String msg);
	WiFiUDP _udp_sender;
	WiFiUDP _udp_receiver;
	WiFiClass _wifi;
	int _udp_port_receive;
	boolean _udp_receiver_started;
	udp_receiver _receiver_list[4] = { udp_receiver(), udp_receiver(), udp_receiver(), udp_receiver() };
};