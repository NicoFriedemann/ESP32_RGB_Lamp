#pragma once
#include "Enums.h"

class RGB_Utils {
public:
	RGB_Utils(int LED_MAX_RGB_VALUE);
	RGB_Utils() { _LED_MAX_RGB_VALUE = 4095; };

	struct s_hsv {
		float h;       // angle in degrees
		float s;       // a fraction between 0 and 1
		float v;       // a fraction between 0 and 1
	};

	struct s_rgb {
		float r;       // a fraction between 0 and 1
		float g;       // a fraction between 0 and 1
		float b;       // a fraction between 0 and 1
	};

	int check_rgb(int val);
	float check_hsv(float val, e_udpmsg_parname par_name);
	float get_dimval(float lightness);
	struct s_rgb convert_hsv2rgb(struct s_hsv in);
	struct s_hsv convert_rgb2hsv(struct s_rgb in);
	void get_rgb(float hue, float sat, float val, int colors[3]);
private:
	int _LED_MAX_RGB_VALUE;
};

