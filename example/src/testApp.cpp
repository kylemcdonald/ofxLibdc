#include "testApp.h"

void testApp::setup() {
	camera.setup();
}

void testApp::update() {
	if(camera.grabVideo(curFrame)) {
		curFrame.update();
	}
}

void testApp::draw() {
	curFrame.draw(0, 0);
}
