#include "UDP_Handler.h"
#include "UDP_Message_Handler.h"

UDP_Handler::UDP_Handler(void(*debug_print_fncptr)(String,e_debug_level), WiFiClass &wifi, int udp_receive_port)
{
	debug_print = debug_print_fncptr;
	_wifi = wifi;
	_udp_port_receive = udp_receive_port;
	_udp_receiver_started = false;
	debug_print("UDP_Connection::UDP_Connection - constructor", e_debug_level::dl_info);
}

void UDP_Handler::add_udp_msg_receiver(udp_receiver udp_rec)
{
	debug_print("UDP_Connection::add_udp_msg_receiver - run", e_debug_level::dl_error);
	for (int i = 0; i <= (MAX_UDP_CLIENTS - 1); i++)
	{
		//receiver already registered?
		if (_receiver_list[i].udp_ipaddress_partner != udp_rec.udp_ipaddress_partner) {
			//receiver entry is not active (empty)
			if (!_receiver_list[i].is_active)
			{
				_receiver_list[i].is_active = true;
				_receiver_list[i].udp_ipaddress_partner = udp_rec.udp_ipaddress_partner;
				_receiver_list[i].udp_port_partner = udp_rec.udp_port_partner;
				_receiver_list[i].udpmsg_type = udp_rec.udpmsg_type;
				debug_print("UDP_Connection::add_udp_msg_receiver - message receiver added!", 
					e_debug_level::dl_info);
				debug_print("UDP_Connection::add_udp_msg_receiver - receiver-ip: " + _receiver_list[i].udp_ipaddress_partner,
					e_debug_level::dl_info);
				debug_print("UDP_Connection::add_udp_msg_receiver - receiver-port: " + (String)_receiver_list[i].udp_port_partner, 
					e_debug_level::dl_info);
				debug_print("UDP_Connection::add_udp_msg_receiver - msg-type " + (String)_receiver_list[i].udpmsg_type, 
					e_debug_level::dl_info);
				break;
			}
		}
	}
}

e_udpmsg_type UDP_Handler::get_udp_msg_format(String msg)
{
	const int BUFFER_SIZE = JSON_OBJECT_SIZE(10);
	StaticJsonBuffer<BUFFER_SIZE> jb;
	JsonObject& obj = jb.parseObject(msg);
	if (obj.success()) {
		return e_udpmsg_type::json;
	}
	else {
		if (msg.indexOf("<") >= 0 && msg.indexOf(">") >= 0)
		{
			return e_udpmsg_type::pos_based_format;
		}
		else {
			return e_udpmsg_type::undefined_udpmsg_type;
		}

	}
}

udp_message UDP_Handler::receive_udp_msg()
{
	char packetBuffer[255];
	udp_message udp_msg = udp_message();
	if (_wifi.isConnected())
	{
		int packetSize = _udp_receiver.parsePacket();
		udp_msg.msg = "";
		if (packetSize)
		{
			int len = _udp_receiver.read(packetBuffer, sizeof(packetBuffer));
			if (len > 0) {
				packetBuffer[len] = 0;
			}
			udp_msg.msg = String(packetBuffer);
			debug_print("UDP_Connection::receive_udp_msg - msg received:" + udp_msg.msg, e_debug_level::dl_debug);
		}
		udp_msg.udp_rec.udp_ipaddress_partner = _udp_receiver.remoteIP().toString();
		udp_msg.udp_rec.udp_port_partner = _udp_receiver.remotePort();
		udp_msg.udp_rec.udpmsg_type = get_udp_msg_format(udp_msg.msg);
		return udp_msg;
	}
}

void UDP_Handler::send_udp_msg(String header, String msg)
{
	if (_wifi.isConnected())
	{
		for (int i = 0; i <= (MAX_UDP_CLIENTS - 1); i++)
		{
			if (_receiver_list[i].is_active) {
				String _msg = UDP_Message_Handler::generate_udp_debug_msg(header, msg, _receiver_list[i].udpmsg_type);
				const char *c = _msg.c_str();
				const char *udp_address_partner = _receiver_list[i].udp_ipaddress_partner.c_str();
				_udp_sender.beginPacket(udp_address_partner, _receiver_list[i].udp_port_partner);
				_udp_sender.printf(c);
				_udp_sender.endPacket();
			}
		}
	}
}

void UDP_Handler::start_udp_receiver()
{
	if (!_udp_receiver_started && _wifi.isConnected())
	{
		_udp_receiver.begin(_udp_port_receive);
		_udp_receiver_started = true;
	}
}

void UDP_Handler::stop_udp_receiver()
{
	if (_udp_receiver_started)
	{
		_udp_receiver.stop();
		_udp_receiver_started = false;
	}
}

