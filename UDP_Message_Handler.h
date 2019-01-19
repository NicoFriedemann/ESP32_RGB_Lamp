#pragma once
#include "Enums.h"
#include "UDP_Handler.h"
#include "RGB_Utils.h"

class UDP_Message_Handler {
public:
	UDP_Message_Handler(void (*debug_print_fncptr)(String), void (*set_program_fncptr)(e_prog_nmbr, int[3], float[3]),
		void(*add_udp_msg_receiver_fncptr)(udp_receiver), int LED_MAX_RGB_VALUE);
	void parse_udp_cmd_msg(udp_message msg);
	static String generate_udp_debug_msg(String header, String msg);

private:
	bool validate_command_udpmsg(String msg);
	float get_next_element_udp_msg(String &msg);
	int get_char_occurrences_string(String search_string, char charcter);
	void (*debug_print)(String msg);
	void (*set_program)(e_prog_nmbr program_number, int rgb_colors_man[3], float hsv_colors_man[3]);
	void (*add_udp_msg_receiver)(udp_receiver udp_rec);
	
	e_prog_nmbr _prog_nmbr;
	RGB_Utils _rgb_util;
	int _LED_MAX_RGB_VALUE;
	int _rgb_colors_man[3] = { _LED_MAX_RGB_VALUE,_LED_MAX_RGB_VALUE,_LED_MAX_RGB_VALUE };
	float _hsv_colors_man[3] = {240.0, 1.0, 1.0};
};