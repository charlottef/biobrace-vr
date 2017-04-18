#pragma once

#include "MyProject.h"
#include "Serial/hidapi.h"
#include <string>



class BioBrace
{	
public:

struct Data {
		FVector Accelerometer;
		FVector Gyroscope;
		FVector Magnetometer;
		float GSR;
		float Myoware;
		int Flags;
	}

	BioBrace();

	Data Update();
};