#include "UDP_Message_Handler.h"
#include <ArduinoJson.h>

UDP_Message_Handler::UDP_Message_Handler(void(*debug_print_fncptr)(String, e_debug_level), void(*set_program_fncptr)(e_prog_nmbr, int[3], float[3]),
	void(*add_udp_msg_receiver_fncptr)(udp_receiver))
{
	debug_print = debug_print_fncptr;
	set_program = set_program_fncptr;
	add_udp_msg_receiver = add_udp_msg_receiver_fncptr;
	_prog_nmbr = auto1;
	_rgb_util = RGB_Utils();
	debug_print("UDP_Message_Handler::UDP_Message_Handler - constructor", e_debug_level::dl_info);
}

e_err UDP_Message_Handler::verify_command_msg(udp_message udp_msg) {
	
	const int BUFFER_SIZE = JSON_OBJECT_SIZE(10);
	StaticJsonBuffer<BUFFER_SIZE> jb;
	JsonObject& obj = jb.parseObject(udp_msg.msg);
	int i = 0;
	int y = 0;

	switch (udp_msg.udp_rec.udpmsg_type)
	{
	case json:
		if (obj.success()) return e_err::no_error;
		else return e_err::error;
	case pos_based_format:
		i = get_char_occurrences_string(udp_msg.msg, '<');
		y = get_char_occurrences_string(udp_msg.msg, '>');

		if (i != y || i < 3 || y < 3 || i > 7 || y > 7)
		{
			debug_print("UDP_Message_Handler::verify_command_msg - verify_command_msg : error i="
				+ String(i) + " y=" + String(y), e_debug_level::dl_error);
			return e_err::error;
		}
		return e_err::no_error;
	case undefined_udpmsg_type:
	default:
		debug_print("UDP_Message_Handler::verify_command_msg - unknown msg_type", e_debug_level::dl_error);
		return e_err::error;
	}
}

int UDP_Message_Handler::get_char_occurrences_string(String search_string, char charcter)
{
	int i = 0;
	const char * str = search_string.c_str();
	for (i = 0; str[i]; str[i] == charcter ? i++ : *str++);
	return i;
}

bool UDP_Message_Handler::is_number(String msg)
{
	int i = 0;
	const char * str = msg.c_str();
	for (i = 0; str[i]; !isDigit(str[i]) ? i++ : *str++);
	debug_print("UDP_Message_Handler::is_number - msg:" + msg + "," + (i > 0 ? "no":"is") + " number", e_debug_level::dl_debug);
	return i > 0 ? false : true;
}

//get cmd from string with pos_based format
e_err UDP_Message_Handler::get_element_pos_based_format(String &msg, String &out)
{
	e_err err = e_err::no_error;
	int first_pos_val = 0;
	int last_pos_val = 0;
	String val = "";
	first_pos_val = msg.indexOf("<");
	last_pos_val = msg.indexOf(">");
	//contains no start or end character
	if (first_pos_val < 0 || last_pos_val < 0)
	{
		debug_print("UDP_Message_Handler::get_element_pos_based_format - command string faulty", e_debug_level::dl_error);
		err = e_err::error;
	}
	val = msg.substring(first_pos_val + 1, last_pos_val);
	msg = msg.substring(last_pos_val + 1);
	out = val;
	return err;
}

//get cmd from json formated string
e_err UDP_Message_Handler::get_element_json(String msg, String json_key, String &out) {

	debug_print("UDP_Message_Handler::get_element_json - run",e_debug_level::dl_debug);
	e_err err = e_err::no_error;
	const int BUFFER_SIZE = JSON_OBJECT_SIZE(10);
	StaticJsonBuffer<BUFFER_SIZE> jb;
	JsonObject& obj = jb.parseObject(msg);
	if (!obj.success()) {
		debug_print("UDP_Message_Handler::get_element_json - json string faulty", e_debug_level::dl_error);
		err = e_err::error;
	}
	else
	{
		out = obj[json_key].asString();
		if (out=="")
		{
			debug_print("UDP_Message_Handler::get_element_json - element empty", e_debug_level::dl_error);
			err = e_err::error;
		}
	}
	return err;
}

e_err UDP_Message_Handler::get_element_msg(udp_message &udp_msg, String &out, String opt_json_key) {

	debug_print("UDP_Message_Handler::get_element_msg - udp_msg: " + udp_msg.msg, e_debug_level::dl_debug);
	e_err err;
	switch (udp_msg.udp_rec.udpmsg_type)
	{
	case json:
		err = get_element_json(udp_msg.msg, opt_json_key, out);
		break;
	case pos_based_format:
		err = get_element_pos_based_format(udp_msg.msg, out);
		break;
	default:
		debug_print("UDP_Message_Handler::get_element_msg - cmd type not defined", e_debug_level::dl_error);
		err = e_err::error;
		break;
	}
	if (err != e_err::no_error)
	{
		debug_print("UDP_Message_Handler::get_element_msg - can't get cmd", e_debug_level::dl_error);
	}
	debug_print("UDP_Message_Handler::get_element_msg - udp_msg: " + udp_msg.msg + ", out: " + out, 
		e_debug_level::dl_debug);
	return err;
}

e_err UDP_Message_Handler::get_float_from_string(String msg, float &out) {
	
	debug_print("UDP_Message_Handler::get_float_from_string - run", e_debug_level::dl_debug);
	e_err err = e_err::no_error;
	if (is_number(msg)) {
		out = msg.toFloat();
	}
	else {
		out = 0.0;
		debug_print("UDP_Message_Handler::get_float_from_string - parameter is no number", e_debug_level::dl_error);
		err = e_err::error;
	}
	return err;
}

e_err UDP_Message_Handler::get_cmd(e_udpmsg_cmd &cmd, udp_message &udp_msg)
{
	debug_print("UDP_Message_Handler::get_cmd - run", e_debug_level::dl_debug);
	String s_temp_cmd = "";
	e_err err;
	if (get_element_msg(udp_msg, s_temp_cmd, C_JSON_CMD) == e_err::no_error)
	{
		float f_temp_cmd;
		err = get_float_from_string(s_temp_cmd, f_temp_cmd);
		if (err == e_err::no_error && f_temp_cmd <= e_udpmsg_cmd::element_count_udpmsg_cmd)
		{
			cmd = e_udpmsg_cmd(f_temp_cmd);
		}
		else {
			debug_print("UDP_Message_Handler::get_cmd - can't cast element to float", e_debug_level::dl_error);
		}
	}
	if (err != e_err::no_error)
	{
		cmd = e_udpmsg_cmd::undefined_udpmsg_cmd;
		debug_print("UDP_Message_Handler::get_cmd - can't get cmd", e_debug_level::dl_error);
	}
	return err;
}

e_err UDP_Message_Handler::get_par(e_udpmsg_parname &par_name, float &par_val, udp_message &udp_msg, int par_pair_nmbr) {
	
	debug_print("UDP_Message_Handler::get_par - run", e_debug_level::dl_debug);
	String s_temp_cmd = "";
	float f_temp_cmd;
	e_err err = e_err::no_error;

	// currently only 3 key value pairs are defined
	if ((par_pair_nmbr <= 0 || par_pair_nmbr > 3))
	{
		debug_print("UDP_Message_Handler::get_par_name - par_pair_nmbr set false", e_debug_level::dl_error);
		return e_err::error;
	}

	if (get_element_msg(udp_msg, s_temp_cmd, C_JSON_PARNAME + par_pair_nmbr) == e_err::no_error)
	{
		err = get_float_from_string(s_temp_cmd, f_temp_cmd);
		if (err == e_err::no_error && f_temp_cmd <= e_udpmsg_parname::element_count_udpmsg_parname)
		{
			par_name = e_udpmsg_parname(f_temp_cmd);
			if (get_element_msg(udp_msg, s_temp_cmd, C_JSON_PARVAL + par_pair_nmbr) == e_err::no_error)
			{
				err = get_float_from_string(s_temp_cmd, f_temp_cmd);
				if (err == e_err::no_error)
				{
					par_val = f_temp_cmd;
				}
			}
		}
	}
	if (err != e_err::no_error)
	{
		par_val = 0.0;
		par_name = e_udpmsg_parname::undefined_udpmsg_parname;
		debug_print("UDP_Message_Handler::get_par_name - can't get par_name or par_val", e_debug_level::dl_error);
	}
	return err;
}

//position based format like <cmd_type><par_name1><par_val1><....><par_name3>,<par_val3>
void UDP_Message_Handler::parse_udp_cmd_msg(udp_message udp_msg) {
	
	debug_print("UDP_Message_Handler::parse_udp_cmd_msg - run", e_debug_level::dl_info);
	e_udpmsg_cmd cmd;
	e_udpmsg_parname par_name;
	float par_val;
	e_err err = e_err::no_error;

	if (verify_command_msg(udp_msg) == e_err::no_error)
	{
		//step1: get cmd_type
		if (get_cmd(cmd, udp_msg) == e_err::no_error)
		{
			switch (cmd)
			{
			case change_prog:
				debug_print("UDP_Message_Handler::parse_udp_cmd_msg - change prog",e_debug_level::dl_debug);
				err = get_par(par_name, par_val, udp_msg, 1);
				if (par_name == program_number && err == e_err::no_error)
				{
					_prog_nmbr = (e_prog_nmbr)par_val;
				}
				else
				{
					debug_print("UDP_Message_Handler::parse_udp_cmd_msg - invalid par_name with cmd change_prog", e_debug_level::dl_error);
				}
				break;
			case change_color:
				debug_print("UDP_Message_Handler::parse_udp_cmd_msg - change color", e_debug_level::dl_info);
				for (int i = 1; i <= 3; i++)
				{
					err = get_par(par_name, par_val, udp_msg, i);
					if (err == e_err::no_error)
					{
						switch (par_name)
						{
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
						case undefined_udpmsg_parname:
						default:
							debug_print("UDP_Message_Handler::parse_udp_cmd_msg - par_name not defined", e_debug_level::dl_error);
							break;
						}
					}
				}
				break;
			case add_msg_receiver:
				debug_print("UDP_Message_Handler::parse_udp_cmd_msg - add_msg_receiver", e_debug_level::dl_info);
				//get debug_level
				err = get_par(par_name, par_val, udp_msg, 1);
				if (err == e_err::no_error)
				{
					if (par_name==e_udpmsg_parname::debug_level)
					{
						udp_msg.udp_rec.udp_debug_level = (e_debug_level)par_val;
					}
				}
				//get remote listening port
				err = get_par(par_name, par_val, udp_msg, 2);
				if (err == e_err::no_error)
				{
					if (par_name == e_udpmsg_parname::port)
					{
						udp_msg.udp_rec.udp_port_partner = (int)par_val;
					}
				}
				add_udp_msg_receiver(udp_msg.udp_rec);
				break;
			default:
				debug_print("UDP_Message_Handler::parse_udp_cmd_msg - invalid cmd: " + String(cmd), 
					e_debug_level::dl_error);
				break;
			}
			if (err == e_err::no_error)
			{
				set_program(_prog_nmbr, _rgb_colors_man, _hsv_colors_man);
			}
		}
		else
		{
			debug_print("UDP_Message_Handler::parse_udp_cmd_msg - can't get cmd from udpmsg" + udp_msg.msg, e_debug_level::dl_error);
			return;
		}
	}
	else
	{
		debug_print("UDP_Message_Handler::parse_udp_cmd_msg - invalid udpmsg:" + udp_msg.msg, e_debug_level::dl_error);
		return;
	}
}

String UDP_Message_Handler::generate_udp_debug_msg(String header, String msg, e_udpmsg_type udpmsg_type)
{
	String debug_msg = "";
	const int capacity = JSON_OBJECT_SIZE(10);
	StaticJsonBuffer<capacity> jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

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
		debug_msg = header + "->" + msg;
		break;
	default:
		break;
	}
	return debug_msg;
}
