# ofxLibdc wraps libdc1394 for openFrameworks

http://damien.douxchamps.net/ieee1394/libdc1394/  
http://openframeworks.cc/

It avoids unnecessary threading by using libdc1394's non-blocking image grabbing, which means you can call grabVideo without worrying about it slowing down your application.

Instead of copying the image to an internal buffer before giving it to you, ofxLibdc will fill the image you pass to it. This avoids unnecessary copying.

A minimal example of grabbing with ofxLibdc looks like:

	#include "ofxLibdc.h"
	class testApp : public ofBaseApp {
	public:
		ofxLibdc::Camera camera;
		ofImage curFrame;
		void setup() {
			camera.setup();
		}
		void update() {
			if(camera.grabVideo(curFrame))
				curFrame.update();
		}
		void draw() {
			curFrame.draw(0, 0);
		}
	};

Because there is no separate capture thread, there is no overhead from copying images you don't need.

The only parameter you may pass to setup() is the camera number or a camera GUID string. Any other camera parameters are handled by setter functions.

ofxLibdc can dynamically change a number of parameters. setPosition() can be used to change the ROI position without restarting the camera. Other changes can be made, but will cause slight delays. Format 7 can be switched on and off, or between modes, 1394b can be switched on and off, and the ROI can be resized.

For an example of interfacing to the color USB Firefly MV on OSX, see [this example project](http://phd.lewissykes.info/webdisk/FireflyMV-USB/) from Lewis Sykes and Elliot Woods.