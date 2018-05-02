#include "stdafx.h"
#include "JAEncoder.h"
#include <stdio.h>
#include <iostream>
#include <thread>

#define TIME_SCALE 9000

void printError(std::string message) {
	std::cout << "[JAEncoder Error] " << message << std::endl;
}

void printHex(const unsigned char* arr, int len) {
	for (int i = 0; i < len; i++) {
		if (arr[i] < 16) {
			std::cout << "0";
		}
		std::cout << std::hex << (int)arr[i] << " ";
	}
	std::cout << std::endl;
}

JAEncoder::JAEncoder(const char * filename, JAEncoderConfig * config) {
	isValid = true;
	this->config = config;
	mp4Handle = MP4Create(filename, 0);
	if (mp4Handle == MP4_INVALID_FILE_HANDLE) {
		isValid = false;
		std::cout << "[JAEncoder Error] Could not create mp4 file: " << filename << std::endl;
		return;
	}
	MP4SetTimeScale(mp4Handle, TIME_SCALE);

	WelsCreateSVCEncoder(&encoder);

	SEncParamExt param;
	encoder->GetDefaultParams(&param);
	param.iUsageType = CAMERA_VIDEO_REAL_TIME;
	param.fMaxFrameRate = config->framerate;
	param.iLtrMarkPeriod = config->framerate;
	param.iPicWidth = config->width;
	param.iPicHeight = config->height;
	param.iTargetBitrate = config->targetBitrate;
	param.iMaxBitrate = 100000000;
	param.iRCMode = RC_BITRATE_MODE;
	param.bEnableDenoise = config->enableImageOptimisation;
	param.iSpatialLayerNum = 1;
	param.bUseLoadBalancing = false;
	param.bEnableSceneChangeDetect = config->enableImageOptimisation;
	param.bEnableBackgroundDetection = config->enableImageOptimisation;
	param.bEnableAdaptiveQuant = config->enableImageOptimisation;
	param.bEnableFrameSkip = config->enableImageOptimisation;
	param.iMultipleThreadIdc = std::thread::hardware_concurrency();
	param.uiIntraPeriod = config->keyframeInterval;

	param.iSpatialLayerNum = 1;
	param.sSpatialLayers[0].iVideoWidth = config->width;
	param.sSpatialLayers[0].iVideoHeight = config->height;
	param.sSpatialLayers[0].fFrameRate = config->framerate;
	param.sSpatialLayers[0].iSpatialBitrate = param.iTargetBitrate;
	param.sSpatialLayers[0].uiProfileIdc = PRO_BASELINE;
	param.sSpatialLayers[0].uiLevelIdc = LEVEL_4_2;
	param.sSpatialLayers[0].iDLayerQp = 42;

	SSliceArgument sliceArg;
	sliceArg.uiSliceMode = SM_FIXEDSLCNUM_SLICE;
	sliceArg.uiSliceNum = std::thread::hardware_concurrency();
	param.sSpatialLayers[0].sSliceArgument = sliceArg;

	param.uiMaxNalSize = 15000000;
	encoder->InitializeExt(&param);

	int videoFormat = config->videoFormat;
	encoder->SetOption(ENCODER_OPTION_DATAFORMAT, &videoFormat);

	mp4Track = MP4AddH264VideoTrack(mp4Handle, TIME_SCALE, TIME_SCALE / config->framerate, config->width,
		config->height, 66, 192, 42, 3);
	MP4SetVideoProfileLevel(mp4Handle, 0x7f);

	encodedFrame = new SFrameBSInfo;
	inputFrame = new SSourcePicture;

	inputFrame->iPicWidth = config->width;
	inputFrame->iPicHeight = config->height;
	inputFrame->iColorFormat = config->videoFormat;
	inputFrame->iStride[0] = config->width;
	inputFrame->iStride[1] = inputFrame->iStride[2] = config->width >> 1;

}

int JAEncoder::encodeFrame(unsigned char * frame) {
	if (!isValid) {
		std::cout << "[JAEncoder Error] Invalid Encoder, check previous error messages" << std::endl;
		return 0;
	}
	inputFrame->pData[0] = frame;
	inputFrame->pData[1] = inputFrame->pData[0] + config->width * config->height;
	inputFrame->pData[2] = inputFrame->pData[1] + (config->width * config->height >> 2);
	

	if (encoder->EncodeFrame(inputFrame, encodedFrame)) {
		std::cout << "[JAEncoder Error] Could not encode frame, check encoder settings and the passed frame" << std::endl;
		return 0;
	}
	int len = 0;
	for (int i = 0; i < encodedFrame->iLayerNum; ++i) {
		const SLayerBSInfo& layerInfo = encodedFrame->sLayerInfo[i];
		for (int j = 0; j < layerInfo.iNalCount; ++j) {
			len += layerInfo.pNalLengthInByte[j];
		}
	}
	MP4WriteSample(mp4Handle, mp4Track, encodedFrame->sLayerInfo[0].pBsBuf, len, MP4_INVALID_DURATION, MP4_INVALID_DURATION, 1);
	//for (int i = 0; i < encodedFrame->iLayerNum; ++i) {
	//	int len = 0;
	//	const SLayerBSInfo& layerInfo = encodedFrame->sLayerInfo[i];
	//	unsigned char* bitstream = encodedFrame->sLayerInfo[i].pBsBuf;
	//	std::cout << "Frametype: " << encodedFrame->eFrameType << std::endl;
	//	//if (encodedFrame->eFrameType == videoFrameTypeIDR && i == 0) {
	//	//	MP4AddH264SequenceParameterSet(mp4Handle, mp4Track, bitstream + 4, layerInfo.pNalLengthInByte[0] - 4);
	//	//	MP4AddH264PictureParameterSet(mp4Handle, mp4Track, bitstream + layerInfo.pNalLengthInByte[0] + 4, layerInfo.pNalLengthInByte[1] - 4);
	//	//}
	//	//else {
	//	//	for (int j = 0; j < layerInfo.iNalCount; ++j) {
	//	//		len += layerInfo.pNalLengthInByte[j];
	//	//	}

	//	//	//bitstream[0] = (len - 4) >> 24;
	//	//	//bitstream[1] = (len - 4) >> 16;
	//	//	//bitstream[2] = (len - 4) >> 8;
	//	//	//bitstream[3] = (len - 4) & 0xff;

	//	//	if (!MP4WriteSample(mp4Handle, mp4Track, bitstream, len, MP4_INVALID_DURATION, MP4_INVALID_DURATION, 1)) {
	//	//		std::cout << "Error when writing sample" << std::endl;
	//	//		system("pause");
	//	//		exit(1);
	//	//	}
	//	//}
	//}
}

void JAEncoder::close() {
	if (!isValid) {
		std::cout << "[JAEncoder Error] Invalid Encoder, check previous error messages" << std::endl;
		return;
	}
	isValid = false;
	encoder->Uninitialize();
	WelsDestroySVCEncoder(encoder);
	MP4Close(mp4Handle);
}
