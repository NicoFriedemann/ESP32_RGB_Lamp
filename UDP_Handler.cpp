#include "UDP_Connection.h"


UDP_Connection::UDP_Connection(void(*debug_print_fncptr)(String), WiFiClass &wifi)
{
	_MAX_UDP_CLIENTS = 4;
	debug_print = debug_print_fncptr;
	_wifi = wifi;
	debug_print("UDP_Connection::UDP_Connection - constructor");
}

void UDP_Connection::add_udp_msg_receiver(udp_receiver udp_rec)
{
	debug_print("UDP_Connection::add_udp_msg_receiver - run");
	for (int i = 0; i <= (_MAX_UDP_CLIENTS - 1); i++)
	{
		//receiver already registered?
		if (_receiver_list[i].udp_ipaddress_partner != udp_rec.udp_ipaddress_partner) {
			//receiver entry is not active (empty)
			if (!_receiver_list[i].is_active)
			{
				_receiver_list[i].is_active = true;
				_receiver_list[i].udp_ipaddress_partner = udp_rec.udp_ipaddress_partner;
				_receiver_list[i].udp_port_partner = udp_rec.udp_port_partner;
				int receive_port = _receiver_list[i].udp_port_partner + 1;
				_receiver_list[i].wifi_udp.begin(receive_port);
				debug_print("UDP_Connection::add_udp_msg_receiver - receiver added!");
				debug_print("UDP_Connection::add_udp_msg_receiver - sender-ip: " + _receiver_list[i].udp_ipaddress_partner);
				debug_print("UDP_Connection::add_udp_msg_receiver - sender-port: " + (String)_receiver_list[i].udp_port_partner);
				debug_print("UDP_Connection::add_udp_msg_receiver - listener-port: " + (String)receive_port);
				break;
			}
		}
	}
}

udp_message UDP_Connection::receive_udp_msg(int client_number)
{
	//array starts at 0 but client number at 1
	if (client_number > 0)
	{
		client_number--;
	}
	if (client_number > _MAX_UDP_CLIENTS) {
		debug_print("UDP_Connection::receive_udp_msg - client number > MAX_UDP_CLIENTS");
		return udp_message();
	}
	char packetBuffer[255];
	udp_message udp_msg = udp_message();
	if (_wifi.isConnected())
	{
		int packetSize = _receiver_list[client_number].wifi_udp.parsePacket();
		udp_msg.msg = "";
		if (packetSize)
		{
			int len = _receiver_list[client_number].wifi_udp.read(packetBuffer, sizeof(packetBuffer));
			if (len > 0) {
				packetBuffer[len] = 0;
			}
			udp_msg.msg = String(packetBuffer);
			debug_print("UDP_Connection::receive_udp_msg - msg received:" + udp_msg.msg);
		}
		udp_msg.udp_rec.udp_ipaddress_partner = _receiver_list[client_number].wifi_udp.remoteIP().toString();
		udp_msg.udp_rec.udp_port_partner = _receiver_list[client_number].wifi_udp.remotePort();
		return udp_msg;
	}
}

void UDP_Connection::send_udp_msg(String header, String msg)
{
	if (_wifi.isConnected())
	{
		String _msg = "";
		_msg = "|" + header + " | " + msg + " |";
		const char * c = _msg.c_str();
		for (int i = 0; i <= (_MAX_UDP_CLIENTS - 1); i++)
		{
			if (_receiver_list[i].is_active) {
				const char* udp_address_partner = _receiver_list[i].udp_ipaddress_partner.c_str();
				_udp_sender.beginPacket(udp_address_partner, _receiver_list[i].udp_port_partner);
				_udp_sender.printf(c);
				_udp_sender.endPacket();
			}
		}
	}
}

