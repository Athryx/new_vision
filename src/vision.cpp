#include "types.h"
#include "vision.h"
#include "util.h"
#include <math.h>

Vision::Vision(cv::Mat template_img, bool display)
: display(display)
{
	process_template(template_img);
}

Vision::~Vision() {
}

void Vision::process_template(cv::Mat img) {
	auto img_template = img.clone();
	cv::cvtColor(img_template, img_template, cv::COLOR_BGR2HSV, 8);
	cv::inRange(img_template, thresh_min, thresh_max, img_template);
	// TODO: pass kernel into morphologyEx instead of plain cv::Mat()
	cv::morphologyEx(img_template, img_template, cv::MORPH_OPEN, cv::Mat());

	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(img_template, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

	show_wait("Template", img_template);

	// find largest contour
	usize index = 0;
	double max_area = 0;
	for (usize i = 0; i < contours.size(); i++) {
		double area = cv::contourArea(contours[i]);
		if (area > max_area) {
			max_area = area;
			index = i;
		}
	}

	template_contour.swap(contours[index]);
	template_area_frac = max_area / cv::boundingRect(template_contour).area();
}

std::optional<Target> Vision::process(cv::Mat img) {
	cv::Size size(img.rows, img.cols);

	show("Input", img);

	// TODO: figure out type of BGR mat
	cv::Mat img_hsv(size, img.type());
	time("HSV conversion", [&]() { cv::cvtColor(img, img_hsv, cv::COLOR_BGR2HSV, 8); });

	cv::Mat img_thresh(size, CV_8U);
	time("Threshold", [&]() { cv::inRange(img_hsv, thresh_min, thresh_max, img_thresh); });
	show("Threshold", img_thresh);

	cv::Mat img_morph(size, CV_8U);
	// TODO: pass kernel into morphologyEx instead of plain cv::Mat()
	time("Morphology", [&]() { cv::morphologyEx(img_thresh, img_morph, cv::MORPH_OPEN, cv::Mat()); });
	show("Morphology", img_morph);

	// TODO: reserve eneough space in vector to prevent reallocations
	std::vector<std::vector<cv::Point>> contours;
	time("Contours", [&]() { cv::findContours(img_morph, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE); });

	usize match_index = 0;
	double best_match = INFINITY;
	double best_area = 0.0;

	time("Contour Matching", [&]() {
		for (usize i = 0; i < contours.size(); i++) {
			auto contour = contours[i];
			double match = cv::matchShapes(template_contour, contour, cv::CONTOURS_MATCH_I3, 0.0);

			double area = cv::contourArea(contour);
			double area_frac = area / cv::boundingRect(contour).area();

			if (match < best_match && match < 1.5 && abs(area_frac - template_area_frac) / template_area_frac < 0.2 && area > best_area) {
				best_match = match;
				best_area = area;
				match_index = i;
			}
		}
	});

	// TODO: maybe display matched contour and bounding box
	if (best_match == INFINITY) return {};

	auto rect = cv::boundingRect(contours[match_index]);
	Target out;
	out.distance = 11386.95362494479 * (1.0 / rect.width);
	auto xpos = rect.x + rect.width / 2;
	out.angle = atan((xpos - 320) / 530.47) * (180.0 / M_PI) + 16;

	return out;
}

void Vision::show(const std::string &name, cv::Mat &img) {
	if (display) {
		cv::namedWindow(name, cv::WINDOW_AUTOSIZE);
		cv::imshow(name, img);
	}
}

void Vision::show_wait(const std::string &name, cv::Mat &img) {
	if (display) {
		cv::namedWindow(name, cv::WINDOW_AUTOSIZE);
		cv::imshow(name, img);
		cv::waitKey();
	}
}
