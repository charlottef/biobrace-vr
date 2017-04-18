#pragma once

#include "MyProject.h"
#include "Serial/sensorData.h"


BioBrace::BioBrace()
{
	// Spider Sensor Init
	handle = hid_open(0x4d8, 0x3f, NULL); // create handle to open device
	buf.resize(255, 0);
}

BioBrace::~BioBrace() {
	hid_close(handle);
}

BioBrace::Data BioBrace::Update()
{
	// amount of bytes read
	int res = 0;

	if (!handle)
	{
		return;
	}

	while (res == 0)
	{
		// amount of bytes received
		res = hid_read(handle, buf.data(), 255);

		// error
		if (res < 0)
			return;
	}

	auto s = bytes_to_string(buf);

	// Split string where we find first \n
	auto spl = split(s, '\n');

	// Tokenize string with whitespace and place in this.data
	auto tokens = split(s, ' ');

	// Copy previous data buffer, fill in with new data
	auto d = data;

	for (uint32_t i = 0; i < tokens.size(); i++)
	{

		d.Accelerometer.X = (i == 0) ? std::stof(tokens[i]) : d.Accelerometer.X;
		d.Accelerometer.Y = (i == 1) ? std::stof(tokens[i]) : d.Accelerometer.Y;
		d.Accelerometer.Z = (i == 2) ? std::stof(tokens[i]) : d.Accelerometer.Z;

		d.Gyroscope.X = (i == 3) ? std::stof(tokens[i]) : d.Gyroscope.X;
		d.Gyroscope.Y = (i == 4) ? std::stof(tokens[i]) : d.Gyroscope.Y;
		d.Gyroscope.Z = (i == 5) ? std::stof(tokens[i]) : d.Gyroscope.Z;

		d.Magnetometer.X = (i == 6) ? std::stof(tokens[i]) : d.Magnetometer.X;
		d.Magnetometer.Y = (i == 7) ? std::stof(tokens[i]) : d.Magnetometer.Y;
		d.Magnetometer.Z = (i == 8) ? std::stof(tokens[i]) : d.Magnetometer.Z;

		d.GSR = (i == 9) ? std::stof(tokens[i]) : d.GSR;
		d.Myoware = (i == 10) ? std::stof(tokens[i]) : d.Myoware;
		d.Flags = (i == 11) ? std::stoi(tokens[i]) : d.Flags;

	}

	data = d;

	// Empty buffer
	buf.clear();

	return data;
}

/// Converts a vector of u8 to a std::string
std::string BioBrace::bytes_to_string(std::vector<uint8_t> v)
{
	std::string s(reinterpret_cast<char const*>(v.data()), v.size());
	return s;
}

std::vector<std::string> split(std::string s, char delim) {

	std::stringstream ss(s);
	std::string item;
	std::vector<std::string> tokens;

	while (std::getline(ss, item, delim)) {
		tokens.push_back(item);
	}
	return tokens;
}