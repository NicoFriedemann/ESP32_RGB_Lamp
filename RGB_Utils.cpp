#include <math.h>
#include "RGB_Utils.h"

RGB_Utils::RGB_Utils(int LED_MAX_RGB_VALUE)
{
	_LED_MAX_RGB_VALUE = LED_MAX_RGB_VALUE;
}

int RGB_Utils::check_rgb(int val)
{
	if (val > _LED_MAX_RGB_VALUE) { return _LED_MAX_RGB_VALUE; }
	else if (val < 0) { return 0; }
	else { return val; }
}

float RGB_Utils::check_hsv(float val, e_udpmsg_parname par_name)
{
	if (val > 1.0 && (par_name == e_udpmsg_parname::saturation || par_name == e_udpmsg_parname::value)) { return 1.0; }
	else if (val > 360.0 && (par_name == e_udpmsg_parname::hue)) { return 360.0; }
	else if (val < 0.0) { return 0.0; }
	else { return val; }
}

float RGB_Utils::get_dimval(float lightness)
{
	float luminance = 0.0;
	float _lightness = lightness * 100.0;
	if (_lightness >= 0.0 && _lightness <= 100.0) {
		if (_lightness <= 8.0) { luminance = _lightness / 902.3; }
		else { luminance = pow(((_lightness + 16.0) / 116.0), 3.0); }
	}
	else { luminance = _lightness; }
	return luminance;
}

RGB_Utils::s_rgb RGB_Utils::convert_hsv2rgb(s_hsv in)
{
	float      hh, p, q, t, ff;
	int        i;
	s_rgb         out;

	in.v = get_dimval(in.v);

	if (in.s <= 0.0) {
		out.r = in.v;
		out.g = in.v;
		out.b = in.v;
		return out;
	}
	hh = in.h;
	if (hh >= 360.0) hh = 0.0;
	hh /= 60.0;
	i = (int)hh;
	ff = hh - i;
	p = in.v * (1.0 - in.s);
	q = in.v * (1.0 - (in.s * ff));
	t = in.v * (1.0 - (in.s * (1.0 - ff)));

	switch (i) {
	case 0:
		out.r = in.v;
		out.g = t;
		out.b = p;
		break;
	case 1:
		out.r = q;
		out.g = in.v;
		out.b = p;
		break;
	case 2:
		out.r = p;
		out.g = in.v;
		out.b = t;
		break;

	case 3:
		out.r = p;
		out.g = q;
		out.b = in.v;
		break;
	case 4:
		out.r = t;
		out.g = p;
		out.b = in.v;
		break;
	case 5:
	default:
		out.r = in.v;
		out.g = p;
		out.b = q;
		break;
	}

	out.r = 1.0 - out.r;
	out.g = 1.0 - out.g;
	out.b = 1.0 - out.b;
	return out;
}

RGB_Utils::s_hsv RGB_Utils::convert_rgb2hsv(s_rgb in)
{
	s_hsv         out;
	float      min, max, delta;

	min = in.r < in.g ? in.r : in.g;
	min = min < in.b ? min : in.b;

	max = in.r > in.g ? in.r : in.g;
	max = max > in.b ? max : in.b;

	out.v = max;
	delta = max - min;
	if (delta < 0.00001)
	{
		out.s = 0;
		out.h = 0;
		return out;
	}
	if (max > 0.0) {
		out.s = (delta / max);
	}
	else {
		out.s = 0.0;
		out.h = NAN;
		return out;
	}
	if (in.r >= max)
		out.h = (in.g - in.b) / delta;
	else
		if (in.g >= max)
			out.h = 2.0 + (in.b - in.r) / delta;
		else
			out.h = 4.0 + (in.r - in.g) / delta;

	out.h *= 60.0;

	if (out.h < 0.0)
		out.h += 360.0;

	return out;
}

void RGB_Utils::get_rgb(float hue, float sat, float val, int colors[3])
{
	struct s_rgb _rgb;
	struct s_hsv _hsv;
	_hsv.h = check_hsv(hue, e_udpmsg_parname::hue);
	_hsv.s = check_hsv(sat, e_udpmsg_parname::saturation);
	_hsv.v = check_hsv(val, e_udpmsg_parname::value);
	_rgb = convert_hsv2rgb(_hsv);
	if (_rgb.r > 0.0) { colors[0] = _rgb.r * _LED_MAX_RGB_VALUE; }
	else { colors[0] = 0; }
	if (_rgb.g > 0.0) { colors[1] = _rgb.g * _LED_MAX_RGB_VALUE; }
	else { colors[1] = 0; }
	if (_rgb.b > 0.0) { colors[2] = _rgb.b * _LED_MAX_RGB_VALUE; }
	else { colors[2] = 0; }
}
