#include <iostream>
#include <cstring>
#include <ctime>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <opencv2/opencv.hpp>

#include "calculator.h"

int main(int argc, char** argv) {
	//open video source
	std::string videoAddress = "http://localhost:4747/mjpegfeed";
	cv::VideoCapture video(videoAddress);
	if (!video.isOpened()) {
		std::cerr << "[ERROR] Camera not found" << std::endl;
		return 1;
	}
	video.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	video.set(CV_CAP_PROP_FRAME_HEIGHT, 480);

	//learning background
	int iterationNumber = 30;
	cv::Mat frame, blur;
	cv::namedWindow("Hand Gesture Calculator");
	cv::Ptr<cv::BackgroundSubtractor> bgsubtractor = cv::createBackgroundSubtractorMOG2(100, 120.0, true);
	while (iterationNumber--) {
		std::cout << "Learning background " << iterationNumber << std::endl;
		video >> frame;
		cv::flip(frame, frame, 1);
		cv::cvtColor(frame, blur, cv::COLOR_BGR2GRAY);
		cv::GaussianBlur(blur, blur, cv::Size(9, 9), 0);
		bgsubtractor->apply(blur, blur);
		cv::putText(frame, "Learning background", cv::Point(0, 10), cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar(0.0, 0.0, 255.0));
		cv::imshow("Hand Gesture Calculator", frame);
		if (cv::waitKey(15) == 27) break;
	}

	//states & initiations
	Calculator calculator(cv::Point(120, 90), cv::Point(520, 390));
	bool is_click = false;

	//image processing
	cv::namedWindow("Foreground");
	cv::Mat foreground, hsv, hand;
	while (true)
	{
		//filtering background
		video >> frame;
		cv::flip(frame, frame, 1);
		cv::cvtColor(frame, blur, cv::COLOR_BGR2GRAY);
		cv::GaussianBlur(blur, blur, cv::Size(9, 9), 0);
		bgsubtractor->apply(blur, foreground, 0);
		cv::GaussianBlur(foreground, foreground, cv::Size(29, 29), 0);
		cv::threshold(foreground, foreground, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
		cv::erode(foreground, foreground, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
		cv::dilate(foreground, foreground, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));

		//find hand contour
		std::vector < std::vector <cv::Point> > contours, hand_contours, hand_hulls;
		std::vector < std::vector < cv::Vec4i > > finger_defects;
		cv::findContours(foreground, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_TC89_L1);
		for (int i = 0; i < contours.size(); i++) {
			if (cv::contourArea(contours[i]) > 5555) {
				std::vector <int> hull_idx;
				cv::convexHull(contours[i], hull_idx);
				std::vector < cv::Point > hull;
				for (int j = 0; j < hull_idx.size(); j++) {
					hull.push_back(contours[i][hull_idx[j]]);
				}

				std::vector < cv::Vec4i > defect, finger_defect;
				cv::convexityDefects(contours[i], hull_idx, defect);
				for (int j = 0; j < defect.size(); j++) {
					int defect_vector1_x = contours[i][defect[j][0]].x - contours[i][defect[j][2]].x;
					int defect_vector1_y = contours[i][defect[j][0]].y - contours[i][defect[j][2]].y;
					int defect_vector2_x = contours[i][defect[j][1]].x - contours[i][defect[j][2]].x;
					int defect_vector2_y = contours[i][defect[j][1]].y - contours[i][defect[j][2]].y;
					double vector1_length = sqrt((double)(defect_vector1_x * defect_vector1_x) + (double)(defect_vector1_y * defect_vector1_y));
					double vector2_length = sqrt((double)(defect_vector2_x * defect_vector2_x) + (double)(defect_vector2_y * defect_vector2_y));
					double dot_product = (double)(defect_vector1_x * defect_vector2_x + defect_vector1_y * defect_vector2_y);
					double cos_value = dot_product / (vector1_length * vector2_length);
					double angle_value = acos(cos_value);
					double defect_width_x = contours[i][defect[j][0]].x - contours[i][defect[j][1]].x;
					double defect_width_y = contours[i][defect[j][0]].y - contours[i][defect[j][1]].y;
					double defect_width = sqrt(defect_width_x * defect_width_x + defect_width_y * defect_width_y);
					if (angle_value < 1.8 && (double) (defect[j][3]) > std::min(2017.8, defect_width * 100.0)) {
						finger_defect.push_back(defect[j]);
					}
				}
				if (finger_defect.size() == 4) {
					hand_contours.push_back(contours[i]);
					hand_hulls.push_back(hull);
					finger_defects.push_back(finger_defect);
				}
			}
		}

		//finding largest hand and its center
		int largest_hand_idx;
		cv::Point largest_hand_center;
		bool is_hand_exist = hand_contours.size() > 0;
		if (is_hand_exist) {
			largest_hand_idx = 0;
			for (int i = 1; i < hand_contours.size(); i++) {
				if (cv::contourArea(hand_contours[i]) > cv::contourArea(hand_contours[largest_hand_idx])) {
					largest_hand_idx = i;
				}
			}
			for (int i = 0; i < finger_defects[largest_hand_idx].size(); i++) {
				largest_hand_center += hand_contours[largest_hand_idx][finger_defects[largest_hand_idx][i][2]];
			}
			largest_hand_center /= (int) (finger_defects[largest_hand_idx].size());
		}

		//processing hand state
		if (is_hand_exist) {
			double average_defect_distance = 0;
			double average_fingertip_distance = 0;
			for (int i = 0; i < finger_defects[largest_hand_idx].size(); i++) {
				int fingertip_vector1_x = largest_hand_center.x - hand_contours[largest_hand_idx][finger_defects[largest_hand_idx][i][0]].x;
				int fingertip_vector1_y = largest_hand_center.y - hand_contours[largest_hand_idx][finger_defects[largest_hand_idx][i][0]].y;
				int fingertip_vector2_x = largest_hand_center.x - hand_contours[largest_hand_idx][finger_defects[largest_hand_idx][i][1]].x;
				int fingertip_vector2_y = largest_hand_center.y - hand_contours[largest_hand_idx][finger_defects[largest_hand_idx][i][1]].y;
				int defect_vector_x = largest_hand_center.x - hand_contours[largest_hand_idx][finger_defects[largest_hand_idx][i][2]].x;
				int defect_vector_y = largest_hand_center.y - hand_contours[largest_hand_idx][finger_defects[largest_hand_idx][i][2]].y;
				average_fingertip_distance += sqrt((double)(fingertip_vector1_x * fingertip_vector1_x) + (double)(fingertip_vector1_y * fingertip_vector1_y))
					+ sqrt((double)(fingertip_vector2_x * fingertip_vector2_x) + (double)(fingertip_vector2_y * fingertip_vector2_y));
				average_defect_distance += sqrt((double)(defect_vector_x * defect_vector_x) + (double)(defect_vector_y * defect_vector_y));
			}
			average_fingertip_distance /= 2 * finger_defects[largest_hand_idx].size();
			average_defect_distance /= finger_defects[largest_hand_idx].size();
			double distance_ratio = average_fingertip_distance / average_defect_distance;
			//if (distance_ratio < 1.95 ^ is_click) {
			//	state_transition_time = clock();
			//}
			is_click = distance_ratio < 2.1;
		}
		else {
			//if (is_click) {
			//	state_transition_time = clock();
			//}
			is_click = false;
		}
		if (is_click) {
			calculator.press(largest_hand_center);
		}
		else {
			calculator.unpress();
		}

		//draw result
		/*
		//<TOSHOW>
		cv::putText(frame, std::to_string(clock() - state_transition_time), cv::Point(0, 11), cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar(0.0, 0.0, 255.0));
		for (int i = 0; i < finger_defects.size(); i++) {
			for (int j = 0; j < finger_defects[i].size(); j++) {
				cv::circle(frame, hand_contours[i][finger_defects[i][j][2]], 5, cv::Scalar(0, 255, 255), -1);
			}
		}
		//</TOSHOW>
		*/
		calculator.draw(frame);
		if (is_hand_exist) {
			if (!is_click) {
				cv::circle(frame, largest_hand_center, 8, cv::Scalar(191, 63, 63), 2);
			}
			else {
				cv::circle(frame, largest_hand_center, 9, cv::Scalar(255, 0, 0), -1);
			}
		}
		cv::drawContours(frame, hand_contours, -1, cv::Scalar(127, 127, 255));
		cv::drawContours(frame, hand_hulls, -1, cv::Scalar(127, 255, 127));
		cv::imshow("Hand Gesture Calculator", frame);
		cv::imshow("Foreground", foreground);

		//keyboard input processing
		int key = cv::waitKey(15);
		if (key == 27) {
			break;
		}
	}

	//end of program
	cv::destroyAllWindows();
	return 0;
}