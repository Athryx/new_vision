#include "types.h"
#include "argparse.hpp"
#include "vision.h"
#include "util.h"
#include <opencv2/opencv.hpp>
#include <mosquitto.h>
#include <stdio.h>
#include <unistd.h>

#define MQTT_HOST "roborio-2358-frc.local"
#define MQTT_PORT 1183

int main(int argc, char **argv) {
	argparse::ArgumentParser program("vision", "0.1.0");
	
	program.add_argument("-d")
		.help("display processing frames")
		.default_value(false)
		.implicit_value(true);

	program.add_argument("-m")
		.help("publish distance and angle to mqtt broker")
		.default_value(std::string {MQTT_HOST});

	program.add_argument("-c")
		.help("camera device file name to process, if no file name is given, use camera 0")
		.default_value(std::optional<std::string> {})
		.action([] (const std::string& str) -> std::optional<std::string> {
				return str;
		});

	program.add_argument("template")
		.help("template image file to process");

	try {
		program.parse_args (argc, argv);
	} catch (const std::runtime_error& err) {
		std::cout << err.what() << std::endl;
		std::cout << program;
		exit(1);
	}

	const bool display_flag = program.get<bool>("-d");

	// TODO: maybe it is ugly to have a boolean and mqtt_client, maybe use an optional?
	const bool mqtt_flag = program.is_used("-m");
	// XXX: if mqtt_flag is set, this is guaranteed to be a valid pointer
	struct mosquitto *mqtt_client = nullptr;
	if (mqtt_flag) {
		auto host_name = program.get("-m");
		auto client_name = std::string {"vision_"} + std::to_string(getpid());

		mosquitto_lib_init();
		mqtt_client = mosquitto_new(client_name.c_str(), true, nullptr);
		if (mqtt_client == nullptr) {
			printf("couldn't create MQTT client\n");
			exit(2);
		}

		// mosquitto_connect returns a non zero value on failure
		if (mosquitto_connect(mqtt_client, host_name.c_str(), MQTT_PORT, 60)) {
			printf("warning: could not connect to mqtt_host %s\n", host_name.c_str());
		}
	}

	cv::VideoCapture cap;
	auto file_name = program.get<std::optional<std::string>>("-c");
	if (file_name.has_value()) {
		cap.open(*file_name);
	} else {
		cap.open(0);
		cap.set(cv::CAP_PROP_FRAME_WIDTH, 320);
		cap.set(cv::CAP_PROP_FRAME_HEIGHT, 240);
		cap.set(cv::CAP_PROP_FPS, 120);
	}

	Vision vis(display_flag);

	auto template_file = program.get("template");
	auto template_img = cv::imread(template_file, -1);
	if (template_img.empty()) {
		printf("template file '%s' empty or missing\n", template_file.c_str());
		exit(3);
	}
	vis.process_template(template_img);

	const int msg_len = 32;
	char msg[msg_len];
	memset(msg, 0, msg_len);

	for (;;) {
		cv::Mat frame;
		cap >> frame;
		if (frame.empty()) break;

		auto target = time<std::optional<Target>>("frame", [&]() {
			return vis.process(frame);
		});

		if (mqtt_flag) {
			if (target.has_value()) {
				snprintf(msg, msg_len, "1 %6.2f %6.2f", target->distance, target->angle);
			}
			else {
				snprintf(msg, msg_len, "0 %6.2f %6.2f", 0.0f, 0.0f);
			}

			mosquitto_publish(mqtt_client, 0, "PI/CV/SHOOT/DATA", strlen(msg), msg, 0, false);
			int ret = mosquitto_loop(mqtt_client, 0, 1);
			printf("message sent: %s\n", msg);
			if (ret) {
				printf("connection lost, reconnecting...\n");
				mosquitto_reconnect(mqtt_client);
			}
		}
	}

	if (mqtt_flag) {
		mosquitto_destroy(mqtt_client);
		mosquitto_lib_cleanup();
	}
}
