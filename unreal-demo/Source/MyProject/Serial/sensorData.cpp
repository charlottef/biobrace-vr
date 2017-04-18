#pragma once

#include "MyProject.h"
#include "Serial/sensorData.h"


using namespace std;

const int TOT_VALS = 30;
const int RANGE = 10;
int motion[TOT_VALS] = { 0 };
int lastVal = TOT_VALS - 1;
int n;
int start;
bool gamePaused = false;
chrono::time_point<chrono::high_resolution_clock> t1, t2;
double gyro_X = 0;
double gyro_Y = 0;
double gyro_Z = 0;

int XL_X_Label = 2;
int XL_Y_Label = 10;
int XL_Z_Label = 50;
int G_X_Label = 250;
int G_Y_Label = 1250;
int G_Z_Label = 6250;

BioBrace::BioBrace()
{
	rot[0] = 0;
	rot[1] = 0;
	omega[0] = 0;
	omega[1] = 0;
	angle[0] = 0;
	angle[1] = 0;
	
}

int BioBrace::getData(unsigned char buf[], int sensor_data[], float final_data[], hid_device *handle)
{
	int res; // amount of bytes read

	if (!handle)
	{
		printf("unable to open device\n");
		return 1;
	}

		// Get Sensor Data (cmd 0xA0)
		buf[0] = 0x0; // Report Number (0x0 for Windows)
		buf[1] = 0xA0;// Sensor Data Command
		res = hid_write(handle, buf, 65); // res = 2 (2 bytes sent)
		if (res < 0)
			printf("Unable to write() (2)\n");

		res = 0; // reset res
		while (res == 0)
		{
			res = hid_read(handle, buf, 65); // amount of bytes received
			if (res == 0) // wait for data
				printf("waiting...\n");
			if (res < 0) // error
				printf("Unable to read()\n");

		}

		t2 = chrono::high_resolution_clock::now();

		for (int i = 0; i < 19; i++)
		{
			sensor_data[i] = buf[i];

		}

		for (int a = 2; a < 8; a++)
		{
			if ((0x80 & sensor_data[a]) > 0)
			{
				sensor_data[a] -= 0x100; // sensor_data[2] -sensor_data[4] accelerometer sensor_data[5]-sensor_data[7] gyro
			}
		}

		sensor_data[7] += 1; //Adjust for offset

		// Find degrees moved
		gyro_X = getAngle(sensor_data[5], gyro_X);
		gyro_Y = getAngle(sensor_data[6], gyro_Y);
		gyro_Z = getAngle(sensor_data[7], gyro_Z);
		t1 = t2;

		// get data function ends
		final_data[0] = cleanData(sensor_data[2]); // XL X
		final_data[1] = cleanData(sensor_data[3]); // XL Y
		final_data[2] = cleanData(sensor_data[4]); // XL Z
		final_data[3] = cleanData(sensor_data[5]); // Gyro X
		final_data[4] = cleanData(sensor_data[6]); // Gyro Y
		final_data[5] = cleanData(sensor_data[7]); // Gyro Z
		return 0; // success
	
}
/*************************************************************************************/

float BioBrace::cleanData(float value)
{
	float normalize = value / 128.0f;
	float norm_dead = deadzone(normalize);
	return twoDecimal(norm_dead);
}


// Data must be normalized before applying this function.
void BioBrace::detectGesture(float data[])
{

	XL_X_Label = LabelXL(data[0], 1);
	XL_Y_Label = LabelXL(data[1], 5);
	XL_Z_Label = LabelXL(data[2], 25);
	G_X_Label = LabelG(data[3], 125);
	G_Y_Label = LabelG(data[4], 625);
	G_Z_Label = LabelG(data[5], 3125);

	int label = XL_X_Label + XL_Y_Label + XL_Z_Label + G_X_Label + G_Y_Label + G_Z_Label;


	//IF new value is found
	if (label != motion[lastVal])
	{
		//Push all values up
		for (int i = 0; i <= (lastVal - 1); i++)
		{
			motion[i] = motion[i + 1];
		}

		//Assign new value to last container
		motion[lastVal] = label;

		//cout << label << endl; // REMOVE**
	}


	n = lastVal;
	start = n;

	while (n > (start - RANGE) && (n >= 0)){
		if ((motion[n] == 7810) || (motion[n] == 7835)){
		n--;
		start = n;
		
		while (n > (start - RANGE) && (n >= 0)){
		if (motion[n] == 7862){
			gamePaused = !gamePaused;
			// Reset Values
			n = 0;
			for (int i = 0; i <= lastVal; i++)
			{
				motion[i] = 0;
			}

			//RETURN VALUE REPRESENTING FUNCTION
		}
		n--;
		} ///END WHILE 2
	}
	n--;
	}// END WHILE 1
		
}

float BioBrace::twoDecimal(float value)
{
	int newValue = value * 100;
	float returnValue = newValue / 100.00;

	return returnValue;
}

/***********************************************************************************/

float BioBrace::deadzone(float value)
{
	if ((value > -0.009) && (value < 0.009))
	{
		value = 0;
	}

	return value;
}

/**********************************************************************************/

int BioBrace::LabelXL(float axisValue, int multiplier)
{

	float thresh1 = -0.20f;
	float thresh2 = -0.05f;
	float thresh3 = 0.05f;
	float thresh4 = 0.20f;


	int Label = 2 * multiplier;

	if (axisValue <= thresh1)									//			   Value <= thresh1
	{
		Label = 0 * multiplier;
	}
	else if ((axisValue > thresh1) && (axisValue < thresh2))	//  thresh1 <  Value <  thresh2  
	{
		Label = 1 * multiplier;
	}
	else if ((axisValue >= thresh2) && (axisValue <= thresh3))	//  thresh2 <= Value <= thresh3  **NO MOVEMENT**
	{
		Label = 2 * multiplier;
	}
	else if ((axisValue > thresh3) && (axisValue < thresh4))	//  thresh3 <  Value <  thresh4
	{
		Label = 3 * multiplier;
	}
	else if (axisValue >= thresh4)								//  thresh4 <= Value 
	{
		Label = 4 * multiplier;
	}

	return Label;
}

/************************************************************************************************/

int BioBrace::LabelG(float axisValue, int multiplier)
{

	float thresh1 = -0.3f;
	float thresh2 = -0.1f;
	float thresh3 = 0.1f;
	float thresh4 = 0.3f;

	int Label = 2 * multiplier;

	if (axisValue <= thresh1)		//  thresh0 <= Value <= thresh1
	{
		Label = 0 * multiplier;
	}
	else if ((axisValue > thresh1) && (axisValue < thresh2))	//  thresh1 <  Value <  thresh2
	{
		Label = 1 * multiplier;
	}
	else if ((axisValue >= thresh2) && (axisValue <= thresh3))	//  thresh2 <= Value <= thresh3
	{
		Label = 2 * multiplier;
	}
	else if ((axisValue > thresh3) && (axisValue < thresh4))	//  thresh3 <  Value <  thresh4
	{
		Label = 3 * multiplier;
	}
	else if (axisValue > thresh4)	//  thresh4 <= Value <= thresh5
	{
		Label = 4 * multiplier;
	}

	return Label;
}


float BioBrace::getGyroDelta(float gyro)
{
	int midLevel = 0;
	int unitDegree = 1;
	float T = (1.000 / 250.000); // WHAT IS SAMPLING RATE?

	// gyroX = roll
	gyro = gyro - midLevel;
	rot[0] = rot[1];
	rot[1] = gyro;
	omega[0] = omega[1];
	omega[1] = gyro / unitDegree;
	angle[0] = angle[1];
	angle[1] = angle[0] + (T)*((omega[1] + omega[0]) / 2.000);
	return angle[1];
}

double BioBrace::getAngle(int dps, double gyro)
{
	chrono::duration<double, std::milli> duration = t2 - t1;

	gyro += ((dps * 4.48) / duration.count());
	
	//cout << gyro << endl;
	return gyro;
}