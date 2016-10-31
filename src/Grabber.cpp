#include "Grabber.h"

namespace ofxLibdc {

Grabber::Grabber() :
	newFrame(false) {
}

ofTexture& Grabber::getTextureReference() {
	return buffer.getTexture();
}

void Grabber::setUseTexture(bool useTexture) {
	buffer.setUseTexture(useTexture);
}

//unsigned char* Grabber::getPixels() {
//	return buffer.getPixels();
//}

ofImage& Grabber::getBuffer() {
	return buffer;
}

void Grabber::update() {
	grabFrame();
}

void Grabber::grabFrame() {
	newFrame = grabVideo(buffer);
    if (newFrame) buffer.update();
}

void Grabber::draw(float x, float y) {
	buffer.draw(x, y);
}

void Grabber::draw(float x, float y, float w, float h) {
	buffer.draw(x, y, w, h);
}

float Grabber::getWidth() {
	return Camera::getWidth();
}

float Grabber::getHeight() {
	return Camera::getHeight();
}

bool Grabber::isFrameNew() const {
	if(newFrame) {
		newFrame = false;
		return true;
	}
	return false;
}

void Grabber::close() {
	// handled automatically by ~Camera
}

ofPixels& Grabber::getPixels() {
	return buffer.getPixels();
}
    
const ofPixels& Grabber::getPixels() const {
    return buffer.getPixels();
}

}