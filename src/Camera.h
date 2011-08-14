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
	
	// normalized values
	void setBrightness(float brightness);
	void setGamma(float gamma);
	void setGain(float gain);
	void setExposure(float exposure);
	void setShutter(float shutter);
	void setFeature(dc1394feature_t feature, float value);
	
	// absolute values
	void setBrightnessAbs(float brightness);
	void setGammaAbs(float gamma);
	void setGainAbs(float gain);
	void setExposureAbs(float exposure);
	void setShutterAbs(float shutter);
	void setFeatureAbs(dc1394feature_t feature, float value);
	
	// raw values
	void setBrightnessRaw(unsigned int brightness);
	void setGammaRaw(unsigned int gamma);
	void setGainRaw(unsigned int gain);
	void setExposureRaw(unsigned int exposure);
	void setShutterRaw(unsigned int shutter);
	void setFeatureRaw(dc1394feature_t feature, unsigned int value);
	
	// normalized values
	float getBrightness() const;
	float getGamma() const;
	float getGain() const;
	float getExposure() const;
	float getShutter() const;
	float getFeature(dc1394feature_t feature) const;
	
	// absolute values
	float getBrightnessAbs() const;
	float getGammaAbs() const;
	float getGainAbs() const;
	float getExposureAbs() const;
	float getShutterAbs() const;
	float getFeatureAbs(dc1394feature_t feature) const;
	
	// raw values
	unsigned int getBrightnessRaw() const;
	unsigned int getGammaRaw() const;
	unsigned int getGainRaw() const;
	unsigned int getExposureRaw() const;
	unsigned int getShutterRaw() const;
	unsigned int getFeatureRaw(dc1394feature_t feature) const;
	
	// raw value ranges
	void getBrightnessRange(unsigned int* min, unsigned int* max) const;
	void getGammaRange(unsigned int* min, unsigned int* max) const;
	void getGainRange(unsigned int* min, unsigned int* max) const;
	void getExposureRange(unsigned int* min, unsigned int* max) const;
	void getShutterRange(unsigned int* min, unsigned int* max) const;
	void getFeatureRange(dc1394feature_t feature, unsigned int* min, unsigned int* max) const;
	
	void printFeatures() const;
	
	// image grabbing
	
	bool grabStill(ofImage& img);
	bool grabVideo(ofImage& img, bool dropFrames = true);
	
	void flushBuffer();
		
	unsigned int getWidth() const;
	unsigned int getHeight() const;
	ofImageType getImageType() const;
	
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