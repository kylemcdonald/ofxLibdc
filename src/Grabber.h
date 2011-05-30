/*
 ofxLibdc::Grabber has all the features of ofxLibdc::Camera with the
 interface of ofBaseVideo. It uses the more traditional approach to
 capturing, which has the disadvantage of being slightly slower and
 requiring more memory -- but the advantage of being easy to swap out.
 
	ofxLibdc::Grabber camera;
	ofImage currentFrame;
	camera.grabFrame();
	if(camera.isFrameNew()) {
		currentFrame.setFromPixels(camera.getPixels() ... );
		// do something with currentFrame
	}
*/

#pragma once

#include "Camera.h"

namespace ofxLibdc {

class Grabber : public Camera, public ofBaseVideo {
public:
	Grabber();
	ofTexture& getTextureReference();
	void setUseTexture(bool useTexture);
	unsigned char* getPixels();
	ofImage& getBuffer();
	void update();
	void grabFrame();
	void draw(float x, float y);
	void draw(float x, float y, float w, float h);
	float getWidth();
	float getHeight();
	bool isFrameNew();
	void close();
	ofPixels& getPixelsRef();
protected:
	ofImage buffer;
	bool newFrame;
};

}