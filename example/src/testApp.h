#pragma once

#include "ofMain.h"
#include "ofxLibdc.h"

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	
	// ofxLibdc is a namespace, and Camera is the object
	ofxLibdc::Camera camera;	
	
	// ofImage is where we store the current frame we grabbed from the Camera 
	ofImage curFrame;
	
	// include these just to make sure they compile
	ofxLibdc::Grabber grabber;
	ofxLibdc::PointGrey pointGrey;
};
