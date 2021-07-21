#pragma once

#include <opencv2/opencv.hpp>
#include <optional>

// represents a detected target
struct Target {
	float distance;
	float angle;
};

// TODO: come up with better class name
class Vision {
	public:
		Vision(bool display);
		~Vision();

		void process_template(cv::Mat img);
		std::optional<Target> process(cv::Mat img);

	private:
		void show(const std::string &name, cv::Mat &img);

		bool display;

		cv::Scalar thresh_min { cv::Scalar(10, 70, 70) };
		cv::Scalar thresh_max { cv::Scalar(40, 255, 255) };

		std::vector<std::vector<cv::Point>> contours {};
		std::vector<std::vector<cv::Point>> template_contours {};
};
