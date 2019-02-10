#include "UDP_Message_Handler.h"
#include <ArduinoJson.h>


UDP_Message_Handler::UDP_Message_Handler(void(*debug_print_fncptr)(String), void(*set_program_fncptr)(e_prog_nmbr, int[3], float[3]),
	void(*add_udp_msg_receiver_fncptr)(udp_receiver))
{
	debug_print = debug_print_fncptr;
	set_program = set_program_fncptr;
	add_udp_msg_receiver = add_udp_msg_receiver_fncptr;
	_prog_nmbr = auto1;
	_rgb_util = RGB_Utils();
	debug_print("UDP_Message_Handler::UDP_Message_Handler - constructor");
}

bool UDP_Message_Handler::validate_command_udpmsg(String msg) {
	int i = get_char_occurrences_string(msg, '<');
	int y = get_char_occurrences_string(msg, '>');

	if (i != y || i < 3 || y < 3 || i > 7 || y > 7)
	{
		debug_print("UDP_Message_Handler::validate_command_udpmsg - validate_command_udpmsg : error i=" 
			+ String(i) + " y=" + String(y));
		return true;
	}
	return false;
}

//todo: error handling
bool UDP_Message_Handler::get_par_name(e_udpmsg_parname & par_name, String & udp_msg)
{
	float f_temp_par_name = 0.0;
	int i_temp_par_name = 0;
	bool err = false;
	err = get_next_element_udp_msg(udp_msg, f_temp_par_name);

	if (!err && i_temp_par_name <= e_udpmsg_parname::element_count_udpmsg_parname)
	{
		i_temp_par_name = int(f_temp_par_name);
		par_name = e_udpmsg_parname(i_temp_par_name);
		return false;
	}
	else
	{
		debug_print("UDP_Message_Handler::get_par_name - cant cast par_name");
		return true;
	}
}

//todo: error handling
bool UDP_Message_Handler::get_par_value(float & par_val, String & udp_msg)
{
	if (!get_next_element_udp_msg(udp_msg, par_val))
	{
		return false;
	}
	else
	{
		debug_print("UDP_Message_Handler::get_par_value - cant cast par_val");
		return true;
	}
}

//todo: error handling
bool UDP_Message_Handler::get_cmd(e_udpmsg_cmd & cmd, String & udp_msg)
{
	float temp = 0.0;
	if (!get_next_element_udp_msg(udp_msg, temp))
	{
		cmd = (e_udpmsg_cmd)int(temp);
		return false;
	}
	else
	{
		debug_print("UDP_Message_Handler::get_cmd - cant cast cmd");
		return true;
	}
}

int UDP_Message_Handler::get_char_occurrences_string(String search_string, char charcter)
{
	int i = 0;
	const char * str = search_string.c_str();
	for (i = 0; str[i]; str[i] == charcter ? i++ : *str++);
	return i;
}

//todo: error handling
boolean UDP_Message_Handler::get_next_element_udp_msg(String &msg, float &out)
{
	int first_pos_val = 0;
	int last_pos_val = 0;
	String val = "";
	String msg_old = msg;
	first_pos_val = msg.indexOf("<");
	last_pos_val = msg.indexOf(">");
	val = msg.substring(first_pos_val + 1, last_pos_val);
	msg = msg.substring(last_pos_val + 1);
	try
	{
		out = val.toFloat();
		return false;
	}
	catch (const std::exception& e)
	{
		debug_print("UDP_Message_Handler::get_next_element_udp_msg - parameter is no number, err=" + String(e.what()));
		return true;
	}
}

void UDP_Message_Handler::parse_udp_cmd_msg(udp_message udp_msg, e_udpmsg_type udpmsg_type)
{
	debug_print("UDP_Message_Handler::parse_udp_cmd_msg - run");
	boolean err_occured = false;
	e_udpmsg_cmd cmd;
	e_udpmsg_parname par_name;
	float par_val = 0.0;

	//different handling of format
	switch (udpmsg_type)
	{
	case undefined_udpmsg_type:
		break;
	case json:
		break;
	case pos_based_format:
		//position based formar like <cmd_type><par_name1><par_val1><....><par_name3>,<par_val3>
		//step1: get cmd_type
		//step2: get par_name
		//step3: get par_value
		//repeat step 2-3 till parameter pair 3
		if (validate_command_udpmsg(udp_msg.msg))
		{
			debug_print("UDP_Message_Handler::parse_udp_cmd_msg(pos_based_format) - invalid udpmsg:" + udp_msg.msg);
			return;
		}

		//step1: get cmd_type
		get_cmd(cmd, udp_msg.msg);

		switch (cmd)
		{
		case change_prog:
			debug_print("UDP_Message_Handler::parse_udp_cmd_msg(pos_based_format) - change prog");
			get_par_name(par_name, udp_msg.msg);
			if (par_name == program_number)
			{
				get_par_value(par_val, udp_msg.msg);
				_prog_nmbr = (e_prog_nmbr)int(par_val);
			}
			else
			{
				debug_print("UDP_Message_Handler::parse_udp_cmd_msg(pos_based_format) - invalid par_name with cmd change_prog");
			}
			break;
		case change_color:
			debug_print("UDP_Message_Handler::parse_udp_cmd_msg(pos_based_format) - change color");
			do
			{			
				get_par_name(par_name, udp_msg.msg);
				get_par_value(par_val, udp_msg.msg);
				switch (par_name)
				{
				case undefined_udpmsg_parname:
					break;
				case red:
					_rgb_colors_man[rgb_red] = _rgb_util.check_rgb(LED_MAX_RGB_VALUE - int(par_val));
					break;
				case green:
					_rgb_colors_man[rgb_green] = _rgb_util.check_rgb(LED_MAX_RGB_VALUE - int(par_val));
					break;
				case blue:
					_rgb_colors_man[rgb_blue] = _rgb_util.check_rgb(LED_MAX_RGB_VALUE - int(par_val));
					break;
				case hue:
					_hsv_colors_man[hsv_hue] = _rgb_util.check_hsv(par_val, hue);
					break;
				case saturation:
					_hsv_colors_man[hsv_saturation] = _rgb_util.check_hsv(par_val, saturation);
					break;
				case value:
					_hsv_colors_man[hsv_value] = _rgb_util.check_hsv(par_val, value);
					break;
				default:
					break;
				}
			} while (udp_msg.msg.length() > 0);
			break;
		case add_msg_receiver:
			add_udp_msg_receiver(udp_msg.udp_rec);
		default:
			debug_print("UDP_Message_Handler::parse_udp_cmd_msg(pos_based_format) - invalid cmd:" + String(cmd));
			break;
		}
		break;
	default:
		break;
	}
	set_program(_prog_nmbr,_rgb_colors_man,_hsv_colors_man);
}

String UDP_Message_Handler::generate_udp_debug_msg(String header, String msg, e_udpmsg_type udpmsg_type)
{
	String debug_msg = "";
	const size_t capacity = JSON_OBJECT_SIZE(2) + String("header").length() + header.length() + String("message").length() + msg.length();
	StaticJsonBuffer<300> jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();
	//json buffer capacity is 300, check if string is bigger with some threshold
	if (capacity > (300 - 25)) {
		msg =  "UDP_Message_Handler::generate_udp_debug_msg - message txt to long";
	}

	switch (udpmsg_type)
	{
	case undefined_udpmsg_type:
		debug_msg = "UDP_Message_Handler::generate_udp_debug_msg - invalid udpmsg_type";
		break;
	case json:
		root["header"] = header;
		root["message"] = msg;
		root.printTo(debug_msg);
		break;
	case pos_based_format:
		debug_msg = "|" + header + " | " + msg + " |";
		break;
	default:
		break;
	}
	return debug_msg;
}
