#pragma once

#include "ofMain.h"
#include "Camera.h"

class testApp : public ofBaseApp {
public:
    
    ofImage img1;
    ofImage img2;
    
    ofxLibdc::Camera cam;
    
	void setup();
	void update();
	void draw();
};
