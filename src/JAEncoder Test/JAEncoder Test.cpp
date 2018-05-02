#include "stdafx.h"
#include <chrono>
#include <iostream>
#include "JAEncoder.h"


#define HEIGHT 480
#define WIDTH 640

using namespace std;
using namespace chrono;

/* Just some dummy data to see artifacts ect */
void prepareFrameYUV(int i, unsigned char* pic) {
	for (int y = 0; y<HEIGHT; y++) {
		for (int x = 0; x<WIDTH; x++) {
			pic[y * WIDTH + x] = x + y + i * 3;
		}
	}

	for (int y = 0; y<HEIGHT / 2; y++) {
		for (int x = 0; x<WIDTH / 2; x++) {
			pic[(WIDTH * HEIGHT) + y * (WIDTH / 2) + x] = 128 + y + i * 2;
			pic[(WIDTH * HEIGHT) + (WIDTH * HEIGHT >> 2) + y * (WIDTH / 2) + x] = 64 + x + i * 5;
		}
	}
}

void prepareFrameRGB(int i, unsigned char* pic) {
	for (int y = 0; y<HEIGHT; y++) {
		for (int x = 0; x<WIDTH; x++) {
			pic[(y * WIDTH + x) * 3] = 255 + i;
			pic[(y * WIDTH + x) * 3 + 1] = i;
			pic[(y * WIDTH + x) * 3 + 2] = 128 + i;
		}
	}
}

int main() {
	high_resolution_clock::time_point time = high_resolution_clock::now();
	JAEncoderConfig config;
	config.height = HEIGHT;
	config.width = WIDTH;
	config.targetBitrate = 40000000;
	config.videoFormat = YUV420;
	config.framerate = 75;
	config.enableImageOptimisation = false;
	config.keyframeInterval = 15;
	JAEncoder encoder("JAtest.mp4", &config);
	unsigned char* rgbFrame = new unsigned char[WIDTH * HEIGHT * 3 / 2];
	/*Ipp8u* frame = new unsigned char[WIDTH * HEIGHT * 3];
	Ipp8u** yuvFrame = new Ipp8u*[3];
	yuvFrame[0] = frame;
	yuvFrame[1] = yuvFrame[0] + WIDTH * HEIGHT;
	yuvFrame[2] = yuvFrame[1] + (WIDTH * HEIGHT >> 2);
	IppiSize size;*/
	/*size.width = WIDTH;
	size.height = HEIGHT;*/
	for (int i = 0; i < 29*75; i++) {
		prepareFrameYUV(i, rgbFrame);
		//ippiRGBToYUV420_8u_C3P3(rgbFrame, yuvFrame, size);
		encoder.encodeFrame(rgbFrame);
	}
	encoder.close();
	cout << "done in: ";
	cout <<  duration_cast<milliseconds>(high_resolution_clock::now() - time).count() << endl;
	system("pause");
}

