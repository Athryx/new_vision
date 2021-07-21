#include "vision.h"
#include "util.h"

Vision::Vision(bool display)
: display(display)
{
}

Vision::~Vision() {
}

void Vision::process_template(cv::Mat img) {
	auto img_template = img.clone();
	cv::cvtColor(img_template, img_template, cv::COLOR_BGR2HSV, 8);
	cv::inRange(img_template, thresh_min, thresh_max, img_template);
	// TODO: pass kernel into morphologyEx instead of plain cv::Mat()
	cv::morphologyEx(img_template, img_template, cv::MORPH_OPEN, cv::Mat());
	cv::findContours(img_template, template_contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
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

	contours.clear();
	time("Contours", [&]() { cv::findContours(img_morph, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE); });

	return {};
}

void Vision::show(const std::string &name, cv::Mat &img) {
	if (display) {
		cv::namedWindow(name, cv::WINDOW_AUTOSIZE);
		cv::imshow(name, img);
	}
}
