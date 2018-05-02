#pragma once

#ifdef JAENCODER_EXPORTS
#define JAENCODER_API __declspec(dllexport) 
#else
#define JAENCODER_API __declspec(dllimport) 
#endif

#include <string>
#include "mp4v2/mp4v2.h"
#include "codec_api.h"

typedef enum {
	YUV420 = 23,
} VideoFormat;

typedef struct {
	int width, height;				//Of the source images in pixels
	int framerate;
	int keyframeInterval;           //Set to framerate if unsure
	int targetBitrate;				//Try to play around with this value
	bool enableImageOptimisation;	//If true techniques to improve image quality are enabled
									//This costs a lot performance, rather increase Bitrate if filesize does not matter
	VideoFormat videoFormat;		//Input Video Format YUV420 is always the fastest
} JAEncoderConfig;

class JAEncoder{
	private:
		JAEncoderConfig * config;
		bool isValid;
		ISVCEncoder* encoder;
		SFrameBSInfo* encodedFrame;
		SSourcePicture* inputFrame;
		MP4FileHandle mp4Handle;
		MP4TrackId mp4Track;
	public:
		JAENCODER_API JAEncoder(const char* filename, JAEncoderConfig* config);

		int JAENCODER_API encodeFrame(unsigned char* frame);

		void JAENCODER_API close();
	
};