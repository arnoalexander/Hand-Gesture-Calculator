#include "calculator.h"

Calculator::Calculator(cv::Point upper_left_point, cv::Point lower_right_point, clock_t delay_ms)
{
	int width = lower_right_point.x - upper_left_point.x;
	int height = lower_right_point.y - upper_left_point.y;
	for (int i = 0; i <= 4; i++) {
		x_grid.push_back(upper_left_point.x + width * i / 4);
	}
	for (int i = 0; i <= 5; i++) {
		y_grid.push_back(upper_left_point.y + height * i / 5);
	}
	pressed_key = CALC_KEY_NONE;
	next_value = "0";
	delay = delay_ms;
	last_key_change_time = clock();
	is_operated = false;
	is_typing = true;
	is_point_in_next_value = false;
}

void Calculator::press(cv::Point pressed_point)
{
	int left_x_grid = -1, upper_y_grid = -1;
	bool found = false;
	int i = 0;
	while (i < 4 && !found) {
		found = pressed_point.x > x_grid[i] && pressed_point.x < x_grid[i + 1];
		if (found) {
			left_x_grid = i;
		}
		i++;
	}
	found = false;
	i = 0;
	while (i < 5 && !found) {
		found = pressed_point.y > y_grid[i] && pressed_point.y < y_grid[i + 1];
		if (found) {
			upper_y_grid = i;
		}
		i++;
	}
	int newest_pressed_key = CALC_KEY_NONE;
	if (left_x_grid != -1 && upper_y_grid != -1) {
		switch (left_x_grid) {
			case 0:
			{
				switch (upper_y_grid) {
					case 0: newest_pressed_key = CALC_KEY_CE; break;
					case 1: newest_pressed_key = CALC_KEY_7; break;
					case 2: newest_pressed_key = CALC_KEY_4; break;
					case 3: newest_pressed_key = CALC_KEY_1; break;
					case 4: newest_pressed_key = CALC_KEY_0; break;
				}
				break;
			}
			case 1:
			{
				switch (upper_y_grid) {
					case 1: newest_pressed_key = CALC_KEY_8; break;
					case 2: newest_pressed_key = CALC_KEY_5; break;
					case 3: newest_pressed_key = CALC_KEY_2; break;
					case 4: newest_pressed_key = CALC_KEY_POINT; break;
				}
				break;
			}
			case 2:
			{
				switch (upper_y_grid) {
					case 1: newest_pressed_key = CALC_KEY_9; break;
					case 2: newest_pressed_key = CALC_KEY_6; break;
					case 3: newest_pressed_key = CALC_KEY_3; break;
					case 4: newest_pressed_key = CALC_KEY_RESULT; break;
				}
				break;
			}
			case 3:
			{
				switch (upper_y_grid) {
					case 0: newest_pressed_key = CALC_KEY_DEL; break;
					case 1: newest_pressed_key = CALC_KEY_DIV; break;
					case 2: newest_pressed_key = CALC_KEY_MUL; break;
					case 3: newest_pressed_key = CALC_KEY_MINUS; break;
					case 4: newest_pressed_key = CALC_KEY_PLUS; break;
				}
				break;
			}
		}
	}
	if (newest_pressed_key != pressed_key) {
		pressed_key = newest_pressed_key;
		last_key_change_time = clock();
		is_operated = false;
	}
	else if (clock() - last_key_change_time > delay && !is_operated) {
		operate(pressed_key);
		is_operated = true;
	}
	
}

void Calculator::unpress()
{
	pressed_key = CALC_KEY_NONE;
	last_key_change_time = clock();
}

int Calculator::get_pressed_key()
{
	return pressed_key;
}

void Calculator::draw(cv::InputOutputArray dst)
{
	draw_component(dst, 1, 3, 0, 1, CALC_DISPLAY);
	draw_component(dst, 0, 1, 0, 1, CALC_KEY_CE);
	draw_component(dst, 3, 4, 0, 1, CALC_KEY_DEL);
	draw_component(dst, 0, 1, 1, 2, CALC_KEY_7);
	draw_component(dst, 1, 2, 1, 2, CALC_KEY_8);
	draw_component(dst, 2, 3, 1, 2, CALC_KEY_9);
	draw_component(dst, 3, 4, 1, 2, CALC_KEY_DIV);
	draw_component(dst, 0, 1, 2, 3, CALC_KEY_4);
	draw_component(dst, 1, 2, 2, 3, CALC_KEY_5);
	draw_component(dst, 2, 3, 2, 3, CALC_KEY_6);
	draw_component(dst, 3, 4, 2, 3, CALC_KEY_MUL);
	draw_component(dst, 0, 1, 3, 4, CALC_KEY_1);
	draw_component(dst, 1, 2, 3, 4, CALC_KEY_2);
	draw_component(dst, 2, 3, 3, 4, CALC_KEY_3);
	draw_component(dst, 3, 4, 3, 4, CALC_KEY_MINUS);
	draw_component(dst, 0, 1, 4, 5, CALC_KEY_0);
	draw_component(dst, 1, 2, 4, 5, CALC_KEY_POINT);
	draw_component(dst, 2, 3, 4, 5, CALC_KEY_RESULT);
	draw_component(dst, 3, 4, 4, 5, CALC_KEY_PLUS);
}

void Calculator::draw_component(cv::InputOutputArray dst, int x_grid_start, int x_grid_end, int y_grid_start, int y_grid_end, int key)
{
	int width = x_grid[x_grid_end] - x_grid[x_grid_start];
	int height = y_grid[y_grid_end] - y_grid[y_grid_start];

	cv::Scalar color(0, 31, 127);
	int thickness = std::min(width, height) / 16 + 1;
	cv::rectangle(dst, cv::Point(x_grid[x_grid_start], y_grid[y_grid_start]), cv::Point(x_grid[x_grid_end], y_grid[y_grid_end]), color, thickness);
	
	if (pressed_key == key) {
		color = cv::Scalar(0, 63, 255);
		thickness = thickness * 3 / 2;
	}
	else if (key == CALC_DISPLAY) {
		color = cv::Scalar(0, 127, 255);
	}
	std::string label;
	switch (key) {
		case CALC_KEY_0: label = "0"; break;
		case CALC_KEY_1: label = "1"; break;
		case CALC_KEY_2: label = "2"; break;
		case CALC_KEY_3: label = "3"; break;
		case CALC_KEY_4: label = "4"; break;
		case CALC_KEY_5: label = "5"; break;
		case CALC_KEY_6: label = "6"; break;
		case CALC_KEY_7: label = "7"; break;
		case CALC_KEY_8: label = "8"; break;
		case CALC_KEY_9: label = "9"; break;
		case CALC_KEY_PLUS: label = "+"; break;
		case CALC_KEY_MINUS: label = "-"; break;
		case CALC_KEY_MUL: label = "*"; break;
		case CALC_KEY_DIV: label = "/"; break;
		case CALC_KEY_RESULT: label = "="; break;
		case CALC_KEY_POINT: label = "."; break;
		case CALC_KEY_DEL: label = "DEL"; break;
		case CALC_KEY_CE: label = "CE"; break;
		case CALC_DISPLAY: label = is_typing ? next_value : values[0]; break;
	}
	int label_length = label.length();
	double font_scale = std::min((double)(height) / 18.0, (double)(width) / 14.4);
	double adaptive_font_scale = std::min((double)(height) / 18.0, (double)(width) / ((double)(label_length) * 14.4));
	double adaptive_thickness = adaptive_font_scale * thickness / font_scale;
	int label_width_approx = adaptive_font_scale * label_length * 8;
	int label_height_approx = adaptive_font_scale * 10;
	int horizontal_margin = (width - label_width_approx) / 2;
	int vertical_margin = (height - label_height_approx) / 2;
	cv::Point org(x_grid[x_grid_start] + horizontal_margin, y_grid[y_grid_end] - vertical_margin);
	cv::putText(dst, label, org, cv::FONT_HERSHEY_PLAIN, adaptive_font_scale, color, adaptive_thickness);
}

void Calculator::operate(int key)
{
	std::cout << "Pressed key id : " << key << " (" << clock() << " ms)" << std::endl;
		switch (key) {
		case CALC_KEY_0: add_next_value("0"); break;
		case CALC_KEY_1: add_next_value("1"); break;
		case CALC_KEY_2: add_next_value("2"); break;
		case CALC_KEY_3: add_next_value("3"); break;
		case CALC_KEY_4: add_next_value("4"); break;
		case CALC_KEY_5: add_next_value("5"); break;
		case CALC_KEY_6: add_next_value("6"); break;
		case CALC_KEY_7: add_next_value("7"); break;
		case CALC_KEY_8: add_next_value("8"); break;
		case CALC_KEY_9: add_next_value("9"); break;
		case CALC_KEY_POINT:
		{
			if (!is_typing) {
				next_value = "0.";
			}
			else if (!is_point_in_next_value && next_value.length() > 0) {
				next_value += ".";
			}
			break;
		}
		case CALC_KEY_CE:
		{
			if (!is_typing) {
				if (values.size() == 1) {
					is_typing = values[0].length() > 0;
					if (is_typing) next_value = values[0];
				}
			}
			if (next_value.length() > 0) {
				if (next_value[0] == '-') {
					next_value = next_value.substr(1);
				}
				else {
					next_value = "-" + next_value;
				}
			}
			break;
		}
		case CALC_KEY_DEL:
		{
			if (!is_typing) {
				if (values.size() > 0) {
					is_typing = values[0].length() > 0;
					if (is_typing) next_value = values[0];
				}
			}
			if (next_value.length() > 0) {
				is_point_in_next_value = is_point_in_next_value && next_value.back() != '.';
				next_value = next_value.substr(0, next_value.length() - 1);
				if (next_value.length() == 0) {
					next_value = "0";
				}
				else if (next_value == "-") {
					next_value = "-0";
				}
			}
			break;
		}
		case CALC_KEY_RESULT: operate_values(); break;
		case CALC_KEY_PLUS: push_operator("+"); break;
		case CALC_KEY_MINUS: push_operator("-"); break;
		case CALC_KEY_MUL: push_operator("*"); break;
		case CALC_KEY_DIV: push_operator("/"); break;
	}
}

void Calculator::add_next_value(const std::string& next_digit)
{
	if (next_value == "0" || !is_typing) {
		next_value = next_digit;
		is_typing = true;
	}
	else if (next_value == "-0") {
		next_value = "-" + next_digit;
	}
	else {
		next_value += next_digit;
	}
}

void Calculator::operate_values()
{
	if (is_typing) {
		is_typing = false;
		if (values.size() == 1) {
			values.pop_front();
		}
		values.push_back(next_value);
	}
	if (values.size() == 3) {
		if (values[1] == "+") {
			values[0] = normalize_form(std::to_string(std::stold(values[0]) + std::stold(values[2])));
		}
		else if (values[1] == "-") {
			values[0] = normalize_form(std::to_string(std::stold(values[0]) - std::stold(values[2])));
		}
		else if (values[1] == "*") {
			values[0] = normalize_form(std::to_string(std::stold(values[0]) * std::stold(values[2])));
		}
		else if (values[1] == "/") {
			values[0] = normalize_form(std::to_string(std::stold(values[0]) / std::stold(values[2])));
		}
		values.pop_back();
		values.pop_back();
	}
}

void Calculator::push_operator(const std::string & op)
{
	operate_values();
	if (values.size() == 1) {
		values.push_back(op);
	}
	else if (values.size() == 2) {
		values[1] = op;
	}
}

std::string Calculator::normalize_form(std::string val)
{
	bool found_point = false;
	int i = 0;
	while (i < val.length() && !found_point) {
		found_point = val[i] == '.';
		i++;
	}
	if (found_point) {
		i = val.length() - 1;
		bool delete_this;
		do {
			delete_this = val[i] == '0';
			found_point = val[i] == '.';
			if (delete_this || found_point) i--;
		} while (i >= 0 && delete_this && !found_point);
		val = val.substr(0, i + 1);
	}
	if (val == "-0" || val.length() == 0) {
		return "0";
	}
	else {
		return val;
	}
}
