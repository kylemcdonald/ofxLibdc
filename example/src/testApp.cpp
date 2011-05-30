#include "testApp.h"

void testApp::setup() {
	// If you want to set any non-default parameters like size, format7, blocking
	// capture, etc., you can do it here before setup. They'll be applied to the
	// camera during setup().
	
	// setup() will pick the first camera. Or if there are multiple cameras you
	// can use the number, or the GUID of the camera.
	camera.setup();
	
	// After setup it's still possible to change a lot of parameters. If you want
	// to change a pre-setup parameter, the camera will auto-restart
}

void testApp::update() {
	// grabVideo() will place the most recent frame into curFrame. If it's a new
	// frame, grabFrame returns true. If there are multiple frames available, it
	// will drop old frames and only give you the newest. This guarantees the
	// lowest latency. If you prefer to not drop frames, set the second argument
	// (dropFrames) to false. By default, capture is non-blocking.
	if(camera.grabVideo(curFrame)) {
		curFrame.update();
	}
}

void testApp::draw() {
	// If the camera isn't ready, the curFrame will be empty.
	if(camera.ready()) {
		// Camera doesn't draw itself, curFrame does.
		curFrame.draw(0, 0);
	}
}
