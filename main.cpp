#include "types.h"
#include "argparse.hpp"
#include <opencv2/opencv.hpp>
#include <mosquitto.h>
#include <stdio.h>
#include <unistd.h>

#define MQTT_HOST "roborio-2358-frc.local"
#define MQTT_PORT 1183

int main(int argc, char **argv) {
	argparse::ArgumentParser program("vision");
	
	program.add_argument("-d")
		.help("display processing frames")
		.default_value(false)
		.implicit_value(true);

	program.add_argument("-c")
		.help("display final processed frame")
		.default_value(false)
		.implicit_value(true);

	program.add_argument("-p")
		.help("process frames in parrallel")
		.default_value(false)
		.implicit_value(true);

	program.add_argument("-m")
		.help("publish distance and angle to mqtt broker")
		.default_value(std::string {MQTT_HOST});

	program.add_argument("file")
		.help("file name to process, if no file name is given, use camera 0");

	try {
		program.parse_args (argc, argv);
	} catch (const std::runtime_error& err) {
		std::cout << err.what() << std::endl;
		std::cout << program;
		exit(1);
	}

	const bool display_flag = program.get<bool>("-d");
	const bool final_flag = program.get<bool>("-c");
	const bool parallel_flag = program.get<bool>("-p");

	const bool mqtt_flag = program.is_used("-m");
	struct mosquitto *mqtt_client = nullptr;
	int conn_id;
	if (mqtt_flag) {
		auto mqtt_host = program.get("-m");
		auto client_name = std::string {"vision_"} + std::to_string(getpid());

		mosquitto_lib_init();
		mqtt_client = mosquitto_new(client_name.c_str(), true, nullptr);
		if (mqtt_client == nullptr) {
			std::cout << "couldn't create MQTT client" << std::endl;
			exit(2);
		}

		conn_id = mosquitto_connect(mqtt_client, mqtt_host.c_str(), MQTT_PORT, 60);
	}

	cv::VideoCapture cap;
	auto file_name = program.present<std::string>("file");
	if (file_name.has_value()) {
		cap.open(*file_name);
	} else {
		cap.open(0);
		cap.set(cv::CAP_PROP_FRAME_WIDTH, 320);
		cap.set(cv::CAP_PROP_FRAME_HEIGHT, 240);
		cap.set(cv::CAP_PROP_FPS, 120);
	}
}
