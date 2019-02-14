#pragma once
#include "Enums.h"
#include "UDP_Handler.h"
#include "RGB_Utils.h"

class UDP_Message_Handler {
public:
	UDP_Message_Handler(void (*debug_print_fncptr)(String, e_debug_level), void (*set_program_fncptr)(e_prog_nmbr, int[3], float[3]),
		void(*add_udp_msg_receiver_fncptr)(udp_receiver));
	void parse_udp_cmd_msg(udp_message msg);
	static String generate_udp_debug_msg(String header, String msg, e_udpmsg_type udpmsg_type);

private:
	e_err verify_command_msg(udp_message udp_msg);
	e_err get_par(e_udpmsg_parname &par_name, float &par_val, udp_message &udp_msg, int par_pair_nmbr);
	e_err get_cmd(e_udpmsg_cmd &cmd, udp_message &udp_msg);
	e_err get_element_json(String msg, String json_key, String &out);
	e_err get_element_msg(udp_message &udp_msg, String &out, String opt_json_key);
	e_err get_element_pos_based_format(String &msg, String &out);
	e_err get_float_from_string(String msg, float &out);
	int get_char_occurrences_string(String search_string, char charcter);
	bool is_number(String msg);
	void (*debug_print)(String msg, e_debug_level e_dl);
	void (*set_program)(e_prog_nmbr program_number, int rgb_colors_man[3], float hsv_colors_man[3]);
	void (*add_udp_msg_receiver)(udp_receiver udp_rec);

	e_prog_nmbr _prog_nmbr;
	RGB_Utils _rgb_util;
	int _rgb_colors_man[3] = { LED_MAX_RGB_VALUE,LED_MAX_RGB_VALUE,LED_MAX_RGB_VALUE };
	float _hsv_colors_man[3] = {240.0, 1.0, 1.0};
};