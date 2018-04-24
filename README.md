# JAEncoder
Just Another Encoder is a license friendly h264 encoder and muxer for Windows. It is based on [openh264](https://github.com/cisco/openh264)
provided by Cisco Systems which is based on BSD license and [libmp4v2](https://github.com/sergiomb2/libmp4v2) which is based on MPL license.  
JAEncoder has a super simple API and some build in performance tuning.

## How to use
First create a encoder configuration, the parameter should be self explanatory:
```
JAEncoderConfig config; 
config.height = HEIGHT; 
config.width = WIDTH;
config.targetBitrate = 40000000;
config.videoFormat = YUV420;  //Only YUV420 as input format is supported atm.
config.framerate = 25;
config.enableImageOptimisation = false;  //If off the encoder is faster but requires a higher bitrate
```
Now create the Encoder using the configuration:
```
JAEncoder encoder("JAtest.mp4", &config);
```
Encode Frames:
```
char* yuvFrame;
for (int i = 0; i < 75 * 5; i++) {
	//prepare you frame here
	encoder.encodeFrame(frame);
}
```
Finaly don't forget to close the encoder:
```
encoder.close();
```

## How to build

### Windows 64bit
Open the Visual Studio Solution under `vs/JAEncoder` and just build the main Project. The test project has some not pushed dependencies
yet to support other image formats in the future. If the build is successful the binarys should be located inside the `bin` folder. 
Don't forget to copy the 3rd party dlls from the ext folder, otherwise the application can't launch.

### Windows 32bit
You can just follow the steps above, but there are no precompiled 3rd party dlls yet, you have to compile/download them yourself

### Other Plattforms
The code itself is not Plattform dependent, but there is no building system for Linux or Android yet, feel free to add it yourself.

## Why should I use this libary over FFmpeg?  
You shouldn't use this project if you don't mind to use the LGPL:  
- libx264 is faster than openh264
- FFmpeg has tons of features
- FFmpeg comes with broad support
If you are just looking for an easy and simple way to create a .mp4 file out of some image data than this libary is the way to go.
