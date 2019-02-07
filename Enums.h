#pragma once

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

enum e_udpmsg_type {
	undefined_udpmsg_type,
	debug,
	data,
	control,
	element_count_udpmsg_type,
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
	element_count_udpmsg_parname,
};

const int LED_MAX_RGB_VALUE = 4095;
const int MAX_UDP_CLIENTS = 4;


