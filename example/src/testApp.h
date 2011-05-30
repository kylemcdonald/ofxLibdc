#pragma once

#include "ofMain.h"
#include "ofxLibdc.h"

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	
	ofxLibdc::Camera camera;
	ofxLibdc::Grabber grabber;
	ofImage curFrame;
};
