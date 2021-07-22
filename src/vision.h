#pragma once

#include <opencv2/opencv.hpp>
#include <optional>
#include <vector>

// represents a detected target
struct Target {
	double distance;
	double angle;
};

// TODO: come up with better class name
class Vision {
	public:
		Vision(cv::Mat template_img, bool display);
		~Vision();

		void process_template(cv::Mat img);
		std::optional<Target> process(cv::Mat img) const;

	private:
		void show(const std::string& name, cv::Mat& img) const;
		void show_wait(const std::string& name, cv::Mat& img) const;

		bool display;

		cv::Scalar thresh_min { cv::Scalar(10, 70, 70) };
		cv::Scalar thresh_max { cv::Scalar(40, 255, 255) };

		std::vector<cv::Point> template_contour {};
		double template_area_frac { 0.0 };
};
