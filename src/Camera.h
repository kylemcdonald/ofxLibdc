#pragma once

#include "ofMain.h"
#include "dc1394.h"

// This sets the number of images in the DMA buffer,
// where libdc stores images until you grab them.
#define OFXLIBDC_BUFFER_SIZE 4

namespace ofxLibdc {

class Camera {
public:
	Camera();
	static int getCameraCount();
	
	// pre-setup settings
	void setSize(unsigned int width, unsigned int height);
	void setPosition(unsigned int roiLeft, unsigned int roiTop);
	void setImageType(ofImageType imageType);
	void setFormat7(bool useFormat7, int mode = 0);
	void set1394b(bool use1394b);
	void setBlocking(bool blocking);
	void setBayerMode(dc1394color_filter_t bayerMode); 
	
	virtual bool setup(int cameraNumber = 0);
	virtual bool setup(string cameraGuid);
	virtual ~Camera();
	
	// post-setup settings
	void setBrightness(unsigned int brightness);
	void setGamma(unsigned int gamma);
	void setGain(unsigned int gain);
	void setExposure(unsigned int exposure);
	void setShutter(unsigned int shutter);
	void setFeature(dc1394feature_t feature, unsigned int value);
	
	void setBrightnessNorm(float brightness);
	void setGammaNorm(float gamma);
	void setGainNorm(float gain);
	void setExposureNorm(float exposure);
	void setShutterNorm(float shutter);
	void setFeatureNorm(dc1394feature_t feature, float value);
	
	void getBrightnessRange(unsigned int* min, unsigned int* max) const;
	void getGammaRange(unsigned int* min, unsigned int* max) const;
	void getGainRange(unsigned int* min, unsigned int* max) const;
	void getExposureRange(unsigned int* min, unsigned int* max) const;
	void getShutterRange(unsigned int* min, unsigned int* max) const;
	void getFeatureRange(dc1394feature_t feature, unsigned int* min, unsigned int* max) const;
	
	unsigned int getBrightness();
	unsigned int getGamma();
	unsigned int getGain();
	unsigned int getExposure();
	unsigned int getShutter();
	unsigned int getFeature(dc1394feature_t feature);
	
	float getBrightnessNorm();
	float getGammaNorm();
	float getGainNorm();
	float getExposureNorm();
	float getShutterNorm();
	float getFeatureNorm(dc1394feature_t feature);
	
	float getShutterAbs() const;
	float getFeatureAbs(dc1394feature_t feature) const;
	
	void grabStill(ofImage& img);
	bool grabVideo(ofImage& img, bool dropFrames = true);
	
	void flushBuffer();
		
	unsigned int getWidth() const;
	unsigned int getHeight() const;
	ofImageType getImageType() const;
	void printFeatures() const;
	
	dc1394camera_t* getLibdcCamera();
	bool ready() const;

protected:
	static dc1394_t* libdcContext;
	static int libdcCameras;
	static void startLibdcContext();
	static void stopLibdcContext();
	
	static ofImageType getOfImageType(dc1394color_coding_t imageType);
	static dc1394color_coding_t getLibdcType(ofImageType imageType);
		
	dc1394camera_t* camera;
	dc1394video_mode_t videoMode;
	dc1394capture_policy_t capturePolicy;
	unsigned int width, height, left, top;
	ofImageType imageType;
	
	bool useBayer;
	dc1394color_filter_t bayerMode;
	
	bool useFormat7;
	int format7Mode;
	bool use1394b;
	
	bool grabFrame(ofImage& img);
	bool initCamera(uint64_t cameraGuid);
	bool applySettings();
	
	void quantizeSize();
	void quantizePosition();
	
	void setTransmit(bool transmit);
	
	string makeString(int name);
};

}