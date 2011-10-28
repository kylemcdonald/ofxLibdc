#include "testApp.h"

void testApp::setup() {
    ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetVerticalSync(true);
    
    cam.setFormat7(true, 3);
    cam.setup();
    
    // these could be ofxCvImage instead, to avoid copying buffers to ofImage and then to ofxCvImage
    img1.allocate(cam.getWidth(), cam.getHeight(), OF_IMAGE_GRAYSCALE);
    img2.allocate(cam.getWidth(), cam.getHeight(), OF_IMAGE_GRAYSCALE);
    
    // Set the window size to the camera size.
	ofSetWindowShape(cam.getWidth()*2, cam.getHeight());

}

void testApp::update() {
    if(cam.grabStereo(img1.getPixels(), img2.getPixels())) {
        img1.update();
        img2.update();
    }
}

void testApp::draw() {
    if(cam.isReady()) {
        img1.draw(0, 0);
        img2.draw(img1.getWidth(), 0);
    }
}
