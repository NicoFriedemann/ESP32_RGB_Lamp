#include "UDP_Message_Handler.h"

UDP_Message_Handler::UDP_Message_Handler(void(*debug_print_fncptr)(String), void(*set_program_fncptr)(e_prog_nmbr, int[3], float[3]),
	void(*add_udp_msg_receiver_fncptr)(udp_receiver),int LED_MAX_RGB_VALUE)
{
	debug_print = debug_print_fncptr;
	set_program = set_program_fncptr;
	add_udp_msg_receiver = add_udp_msg_receiver_fncptr;
	_prog_nmbr = auto1;
	_rgb_util = RGB_Utils(LED_MAX_RGB_VALUE);
	_LED_MAX_RGB_VALUE = LED_MAX_RGB_VALUE;
	debug_print("UDP_Message_Handler::UDP_Message_Handler - constructor");
}

bool UDP_Message_Handler::validate_command_udpmsg(String msg) {
	bool out = false;
	int i = get_char_occurrences_string(msg, '<');
	int y = get_char_occurrences_string(msg, '>');

	if (i != y || i < 3 || y < 3)
	{
		debug_print("UDP_Message_Handler::validate_command_udpmsg - validate_command_udpmsg : error i=" 
			+ String(i) + " y=" + String(y));
		out = true;
	}
	return out;
}

int UDP_Message_Handler::get_char_occurrences_string(String search_string, char charcter)
{
	int i = 0;
	const char * str = search_string.c_str();
	for (i = 0; str[i]; str[i] == charcter ? i++ : *str++);
	return i;
}

float UDP_Message_Handler::get_next_element_udp_msg(String & msg)
{
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

void UDP_Message_Handler::parse_udp_cmd_msg(udp_message udp_msg)
{
	debug_print("UDP_Message_Handler::parse_udp_cmd_msg - run");
	e_udpmsg_cmd cmd;
	e_udpmsg_parname par_name;
	float par_val = 0;
	if (validate_command_udpmsg(udp_msg.msg))
	{
		debug_print("UDP_Message_Handler::parse_udp_cmd_msg - invalid udpmsg:" + udp_msg.msg);
		return;
	}
	cmd = (e_udpmsg_cmd)int(get_next_element_udp_msg(udp_msg.msg));

	switch (cmd)
	{
	case change_prog:
		debug_print("UDP_Message_Handler::parse_udp_cmd_msg - change prog");
		par_name = (e_udpmsg_parname)int(get_next_element_udp_msg(udp_msg.msg));
		if (par_name == program_number)
		{
			par_val = get_next_element_udp_msg(udp_msg.msg);
			_prog_nmbr = (e_prog_nmbr)int(par_val);
		}
		break;
	case change_color:
		debug_print("UDP_Message_Handler::parse_udp_cmd_msg - change color");
		do
		{
			par_name = (e_udpmsg_parname)int(get_next_element_udp_msg(udp_msg.msg));
			par_val = get_next_element_udp_msg(udp_msg.msg);
			switch (par_name)
			{
			case undefined_udpmsg_parname:
				break;
			case red:
				if (_prog_nmbr != e_prog_nmbr::man_pc_rgb)
				{ 
					debug_print("UDP_Message_Handler::parse_udp_cmd_msg - error -> changed red -> prog:" + String(_prog_nmbr)); 
				}
				_rgb_colors_man[0] = _rgb_util.check_rgb(_LED_MAX_RGB_VALUE - int(par_val));
				break;
			case green:
				if (_prog_nmbr != e_prog_nmbr::man_pc_rgb)
				{ 
					debug_print("UDP_Message_Handler::parse_udp_cmd_msg - error -> changed green -> prog:" + String(_prog_nmbr)); 
				}
				_rgb_colors_man[1] = _rgb_util.check_rgb(_LED_MAX_RGB_VALUE - int(par_val));
				break;
			case blue:
				if (_prog_nmbr != e_prog_nmbr::man_pc_rgb)
				{
					debug_print("UDP_Message_Handler::parse_udp_cmd_msg - error -> changed blue -> prog:" + String(_prog_nmbr));
				}
				_rgb_colors_man[2] = _rgb_util.check_rgb(_LED_MAX_RGB_VALUE - int(par_val));
				break;
			case hue:
				if (_prog_nmbr != e_prog_nmbr::man_pc_hsv) { 
					debug_print("UDP_Message_Handler::parse_udp_cmd_msg - error -> changed hue -> prog:" + String(_prog_nmbr));
				}
				_hsv_colors_man[0] = _rgb_util.check_hsv(par_val, e_udpmsg_parname::hue);
				break;
			case saturation:
				if (_prog_nmbr != e_prog_nmbr::man_pc_hsv)
				{
					debug_print("UDP_Message_Handler::parse_udp_cmd_msg - error -> changed saturation -> prog:" + String(_prog_nmbr));
				}
				_hsv_colors_man[1] = _rgb_util.check_hsv(par_val, e_udpmsg_parname::saturation);
				break;
			case value:
				if (_prog_nmbr != e_prog_nmbr::man_pc_hsv)
				{
					debug_print("UDP_Message_Handler::parse_udp_cmd_msg - error -> changed value -> prog:" + String(_prog_nmbr));
				}
				_hsv_colors_man[2] = _rgb_util.check_hsv(par_val, e_udpmsg_parname::value);
				break;
			default:
				break;
			}
		} while (udp_msg.msg.length() > 0);
		break;
	case add_msg_receiver:
		add_udp_msg_receiver(udp_msg.udp_rec);
	default:
		debug_print("UDP_Message_Handler::parse_udp_cmd_msg - invalid cmd:" + String(cmd));
		break;
	}
	set_program(_prog_nmbr,_rgb_colors_man,_hsv_colors_man);
}

String UDP_Message_Handler::generate_udp_debug_msg(String header, String msg)
{
	return "|" + header + " | " + msg + " |";
}
