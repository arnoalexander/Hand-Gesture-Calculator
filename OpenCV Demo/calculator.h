#pragma once

#include <iostream>
#include <cstdlib>
#include <opencv2/opencv.hpp>

#define CALC_KEY_NONE -1
#define CALC_KEY_0 0
#define CALC_KEY_1 1
#define CALC_KEY_2 2
#define CALC_KEY_3 3
#define CALC_KEY_4 4
#define CALC_KEY_5 5
#define CALC_KEY_6 6
#define CALC_KEY_7 7
#define CALC_KEY_8 8
#define CALC_KEY_9 9
#define CALC_KEY_PLUS 10
#define CALC_KEY_MINUS 11
#define CALC_KEY_MUL 12
#define CALC_KEY_DIV 13
#define CALC_KEY_RESULT 14
#define CALC_KEY_POINT 15
#define CALC_KEY_DEL 16
#define CALC_KEY_CE 17
#define CALC_DISPLAY 18

class Calculator {
public:
	Calculator(cv::Point upper_left_point = cv::Point(0, 0), cv::Point lower_right_point = cv::Point(480, 360), clock_t delay_ms = 500);
	void press(cv::Point pressed_point);
	void unpress();
	int get_pressed_key();
	void draw(cv::InputOutputArray dst);
private:
	std::vector<int> x_grid;
	std::vector<int> y_grid;
	int pressed_key;
	std::string next_value;
	std::deque <std::string> values;
	clock_t delay;
	clock_t last_key_change_time;
	bool is_operated;
	bool is_typing;
	bool is_point_in_next_value;

	void draw_component(cv::InputOutputArray dst, int x_grid_start, int x_grid_end, int y_grid_start, int y_grid_end, int key);
	void operate(int key);
	void add_next_value(const std::string& next_digit);
	void operate_values();
	void push_operator(const std::string& op);
	std::string normalize_form(std::string val);
};