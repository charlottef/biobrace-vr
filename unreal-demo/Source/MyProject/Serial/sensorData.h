#pragma once

#include "MyProject.h"
#include "Serial/hidapi.h"
#include <string>
#include <vector>
#include <sstream>

class BioBrace
{

public:

	struct Data {
		FVector Accelerometer;
		FVector Gyroscope;
		FVector Magnetometer;
		float GSR;
		float Myoware;
		uint8_t Flags;
	}

	BioBrace();
	~BioBrace();

	Data Update();

private:
	std::string bytes_to_string(std::vector<uint8_t> v);
	hid_device *handle;
	std::vector<uint8_t> buf;
	std::string serial_str;
	Data data;

};