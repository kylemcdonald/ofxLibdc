#include "ofxLibdcPtGrey.h"

#define readBits(x, pos, len) ((x >> (pos - len)) & ((1 << len) - 1))

/*
 This code sets GPIO 0 and 1 to output alternating pulses.
 Information describing these registers can be found in that following documents:
 http://www.ptgrey.com/support/downloads/documents/TAN2005003_Output_variable_pattern_strobe_pulse.pdf
 http://www.ptgrey.com/support/downloads/documents/TAN2005002_Output_strobe_signal_pulse.pdf
 */
void ofxLibdcPtGrey::setupAlternatingStrobe() {	
	// put GPIO 0 and 1 in output mode
	dc1394_set_control_register(camera, PTGREY_PIO_DIRECTION, 0xc0000000);
	
	// set GPIO 0 and 1 delay to 0, and duration to the integration (shutter) time
	dc1394_set_control_register(camera, PTGREY_STROBE_0_CNT, 0x82000000);
	dc1394_set_control_register(camera, PTGREY_STROBE_1_CNT, 0x82000000);
	
	// set strobe to a period of 2 frames
	dc1394_set_control_register(camera, PTGREY_GPIO_STRPAT_CTRL, 0x80000200);
	
	// put GPIO 0 and 1 in strobe mode
	dc1394_set_control_register(camera, PTGREY_GPIO_CTRL_PIN_0, 0x80030003);
	dc1394_set_control_register(camera, PTGREY_GPIO_CTRL_PIN_1, 0x80030003);
	
	// set GPIO 0 and 1 patterns to (1,0) and (0,1) respectively
	dc1394_set_control_register(camera, PTGREY_GPIO_STRPAT_MASK_PIN_0, 0x8000bfff);
	dc1394_set_control_register(camera, PTGREY_GPIO_STRPAT_MASK_PIN_1, 0x80007fff);
	
	setEmbeddedInfo(PTGREY_EMBED_STROBE_PATTERN);
}

void ofxLibdcPtGrey::clearEmbeddedInfo() {
	dc1394_set_control_register(camera, PTGREY_FRAME_INFO, 0x80000000);
}

void ofxLibdcPtGrey::setEmbeddedInfo(int embeddedInfo, bool enable) {
	unsigned int reg;
	dc1394_get_control_register(camera, PTGREY_FRAME_INFO, &reg);
	if(enable)
		reg |= 1 << embeddedInfo;
	else
		reg &= ~(1 << embeddedInfo);
	dc1394_set_control_register(camera, PTGREY_FRAME_INFO, reg);
}

void ofxLibdcPtGrey::setMaxFramerate() {
	if(useFormat7) {
		unsigned int framerateInq;
		dc1394_get_control_register(camera, PTGREY_FRAME_RATE_INQ, &framerateInq);
		unsigned int minValue = readBits(framerateInq, 24, 12);
		minValue |= 0x82000000;
		dc1394_set_control_register(camera, PTGREY_FRAME_RATE, minValue);
	}
}

unsigned int ofxLibdcPtGrey::getEmbeddedInfoOffset(int embeddedInfo) const {
	unsigned int reg;
	dc1394_get_control_register(camera, PTGREY_FRAME_INFO, &reg);
	int total = 0;
	for(int i = 0; i < embeddedInfo; i++)
		if(reg & (1 << i))
			total++;
	return total;
}

unsigned int ofxLibdcPtGrey::getEmbeddedInfo(ofImage& img, int embeddedInfo) const {
	unsigned int offset = getEmbeddedInfoOffset(embeddedInfo);
	unsigned char* pixels = img.getPixels();
	return ((unsigned int*) pixels)[offset];
}

void ofxLibdcPtGrey::getEmbeddedPosition(ofImage& img, unsigned short* left, unsigned short* top) const {
	unsigned int value = getEmbeddedInfo(img, PTGREY_EMBED_ROI);
	unsigned char* pv = (unsigned char*) &value;
	*left = ((unsigned short) pv[0] << 8) | pv[1];
	*top = ((unsigned short) pv[2] << 8) | pv[3];
}

unsigned int ofxLibdcPtGrey::getEmbeddedStrobeCounter(ofImage& img) const {
	unsigned int value = getEmbeddedInfo(img, PTGREY_EMBED_STROBE_PATTERN);
	unsigned char* pv = (unsigned char*) &value;
	return (unsigned int) (pv[3] & 0x0f);
}