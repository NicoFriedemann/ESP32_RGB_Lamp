#pragma once
#include <WString.h>

enum e_watchdog_state {
	undefined_watchdog_state,
	triggered,
	ok,
	element_count_watchdog_state,
};

enum e_prog_nmbr {
	undefined_prog_nmbr,
	auto1,
	auto2,
	auto3,
	auto4,
	auto5,
	auto6,
	auto7,
	auto8,
	auto9,
	man_blynk,
	man_pc_rgb,
	man_pc_hsv,
	element_count_prog_nmbr,
};

enum e_udpmsg_cmd {
	undefined_udpmsg_cmd,
	change_prog,
	change_color,
	add_msg_receiver,
	element_count_udpmsg_cmd,
};

enum e_udpmsg_parname {
	undefined_udpmsg_parname,
	program_number,
	red,
	green,
	blue,
	hue,
	saturation,
	value,
	debug_level,
	element_count_udpmsg_parname,
};

enum e_udpmsg_type {
	undefined_udpmsg_type,
	json,
	pos_based_format,
	element_count_udpmsg_type,
};

enum e_rgb {
	rgb_red,
	rgb_green,
	rgb_blue,
};

enum e_hsv {
	hsv_hue,
	hsv_saturation,
	hsv_value,
};

enum e_err {
	undefined_err_type,
	no_error,
	error,
	element_count_err,
};

enum e_debug_level {
	dl_debug,
	dl_info,
	dl_error,
};

const int LED_MAX_RGB_VALUE = 4095;
const int MAX_UDP_CLIENTS = 4;
const int MAX_WLAN_CONNECT_RETRY = 3;
const String C_JSON_CMD = "cmd";
const String C_JSON_PARNAME = "par_name_";
const String C_JSON_PARVAL = "par_val_";

