#pragma once
#include <WiFiUdp.h>
#include <WiFi.h>

struct udp_receiver {
public:
	int udp_port_partner;
	String udp_ipaddress_partner;
	boolean is_active;
	udp_receiver() {
		udp_port_partner = 0;
		udp_ipaddress_partner = "0.0.0.0";
		is_active = false;
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
	UDP_Handler(void(*debug_print_fncptr)(String), WiFiClass &wifi, int udp_receive_port);
	void send_udp_msg(String header, String msg);
	void start_udp_receiver();
	void stop_udp_receiver();
	udp_message receive_udp_msg();
	void add_udp_msg_receiver(udp_receiver udp_rec);
private:
	void(*debug_print)(String msg);
	WiFiUDP _udp_sender;
	WiFiUDP _udp_receiver;
	WiFiClass _wifi;
	int _udp_port_receive;
	boolean _udp_receiver_started;
	udp_receiver _receiver_list[4] = { udp_receiver(), udp_receiver(), udp_receiver(), udp_receiver() };
};