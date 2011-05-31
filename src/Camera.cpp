#include "Camera.h"

namespace ofxLibdc {

dc1394_t* Camera::libdcContext = NULL;
int Camera::libdcCameras = 0;

Camera::Camera() :
		camera(NULL),
		imageType(OF_IMAGE_GRAYSCALE),
		useFormat7(false),
		use1394b(false),
		width(640),
		height(480),
		left(0),
		top(0),
		useBayer(false),
		bayerMode(DC1394_COLOR_FILTER_RGGB),
		format7Mode(0),
		capturePolicy(DC1394_CAPTURE_POLICY_POLL) {
	startLibdcContext();
}

Camera::~Camera() {
	if(camera != NULL) {
		dc1394_capture_stop(camera);
		setTransmit(false);
		dc1394_camera_free(camera);
	}
	stopLibdcContext();
}

void Camera::setBayerMode(dc1394color_filter_t bayerMode){
	setImageType(OF_IMAGE_COLOR);
	this->bayerMode = bayerMode;
	useBayer = true;
}

void Camera::setBlocking(bool blocking) {
	capturePolicy = blocking ? DC1394_CAPTURE_POLICY_WAIT : DC1394_CAPTURE_POLICY_POLL;
}

void Camera::startLibdcContext() {
	if(libdcCameras == 0) {
		ofLog(OF_LOG_VERBOSE, "Creating libdc1394 context with dc1394_new().");
		libdcContext = dc1394_new();
	}
	libdcCameras++;
}

void Camera::stopLibdcContext() {
	libdcCameras--;
	if(libdcCameras == 0) {
		ofLog(OF_LOG_VERBOSE, "No more cameras, destroying libdc1394 context.");
		dc1394_free(libdcContext);
	}
}

ofImageType Camera::getOfImageType(dc1394color_coding_t imageType) {
	switch(imageType) {
		case DC1394_COLOR_CODING_MONO8: return OF_IMAGE_GRAYSCALE;
		case DC1394_COLOR_CODING_RGB8: return OF_IMAGE_COLOR;
		default: return OF_IMAGE_COLOR;
	}
}

dc1394color_coding_t Camera::getLibdcType(ofImageType imageType) {
	switch(imageType) {
		case OF_IMAGE_GRAYSCALE: return DC1394_COLOR_CODING_MONO8;
		case OF_IMAGE_COLOR: return DC1394_COLOR_CODING_RGB8;
		default: return DC1394_COLOR_CODING_RGB8;
	}
}

int Camera::getCameraCount() {
	dc1394camera_list_t* list;
	dc1394_camera_enumerate(libdcContext, &list);
	int cameraCount = list->num;
	dc1394_camera_free_list(list);
	return cameraCount;
}

void Camera::setFormat7(bool useFormat7, int mode) {
	bool changed = useFormat7 != this->useFormat7 || mode != this->format7Mode;
	this->useFormat7 = useFormat7;
	this->format7Mode = mode;
	if(camera && changed)
		applySettings();
}

void Camera::set1394b(bool use1394b) {
	bool changed = use1394b != this->use1394b; 
	this->use1394b = use1394b;
	if(camera && changed)
		applySettings();
}

// todo: add an error message if you set an invalid (too big) size
void Camera::setSize(unsigned int width, unsigned int height) {
	bool changed = width != this->width || height != this->height; 
	this->width = width;
	this->height = height;
	if(camera && changed)
		applySettings();
}

// todo: add an error message if you set an invalid (out of bounds) position
void Camera::setPosition(unsigned int left, unsigned int top) {
	bool changed = left != this->left || top != this->top; 
	this->left = left;
	this->top = top;
	if(camera && changed) {
		quantizePosition();
		dc1394_format7_set_image_position(camera, videoMode, this->left, this->top);
	}
}

bool Camera::setup(int cameraNumber) {
	dc1394camera_list_t* list;
	dc1394_camera_enumerate(libdcContext, &list);
	if(list->num == 0) {
		ofLog(OF_LOG_ERROR, "No cameras found.");
		return false;
	}
	uint64_t cameraGuid = list->ids[cameraNumber].guid;
	dc1394_camera_free_list(list);
	return initCamera(cameraGuid) && applySettings();
}

bool Camera::setup(string cameraGuid) {
	uint64_t cameraGuidInt;
	istringstream ss(cameraGuid);
	ss >> hex >> cameraGuidInt;
	return initCamera(cameraGuidInt) && applySettings();
}

bool Camera::initCamera(uint64_t cameraGuid) {
	// create camera struct
	camera = dc1394_camera_new(libdcContext, cameraGuid);
	if (!camera) {
		stringstream error;
		error << "Failed to initialize camera with GUID " << hex << cameraGuid;
		ofLog(OF_LOG_ERROR, error.str());
		return false;
	} else {
		stringstream msg;
		msg << "Using camera with GUID " << hex << camera->guid;
		ofLog(OF_LOG_VERBOSE, msg.str());
	}
	
	#ifdef TARGET_OSX
	dc1394_iso_release_bandwidth(camera, INT_MAX);
	for (int channel = 0; channel < 64; channel++)
		dc1394_iso_release_channel(camera, channel);
	#endif
	
	#ifdef TARGET_LINUX
	dc1394_reset_bus(camera);
	#endif
	
	return true;
}

bool Camera::applySettings() {
	if(camera)
		dc1394_capture_stop(camera);
		
	if(use1394b) {
		// assumes you want to run your 1394b camera at 800 Mbps
		dc1394_video_set_operation_mode(camera, DC1394_OPERATION_MODE_1394B);
		dc1394_video_set_iso_speed(camera, DC1394_ISO_SPEED_800);
	} else {
		dc1394_video_set_operation_mode(camera, DC1394_OPERATION_MODE_LEGACY);
		dc1394_video_set_iso_speed(camera, DC1394_ISO_SPEED_400);
	}
		
	dc1394framerate_t framerate;
	if(useFormat7) {
		videoMode = (dc1394video_mode_t) ((int) DC1394_VIDEO_MODE_FORMAT7_0 + format7Mode);
		unsigned int maxWidth, maxHeight;
		dc1394_format7_get_max_image_size(camera, videoMode, &maxWidth, &maxHeight);
		stringstream msg;
		msg << "Maximum size for current Format7 mode is " << maxWidth << "x" << maxHeight;
		ofLog(OF_LOG_VERBOSE, msg.str());
	} else {
		dc1394video_modes_t video_modes;
		dc1394_video_get_supported_modes(camera, &video_modes);
		dc1394color_coding_t targetCoding = getLibdcType(imageType);
		
		if( useBayer ){
			targetCoding = DC1394_COLOR_CODING_MONO8;
		}
		
		float bestDistance = 0;
		dc1394video_mode_t bestMode;
		bool found = false;
		for(int i = 0; i < video_modes.num; i++) {
			if (!dc1394_is_video_mode_scalable(video_modes.modes[i])) {
				dc1394video_mode_t curMode = video_modes.modes[i];
				unsigned int curWidth, curHeight;
				dc1394_get_image_size_from_video_mode(camera, curMode, &curWidth, &curHeight);

				dc1394color_coding_t curCoding;
				dc1394_get_color_coding_from_video_mode(camera, curMode, &curCoding);

				ofLog(OF_LOG_VERBOSE,
					  "Camera mode " + ofToString(i) + ": " + makeString(curCoding) + " " +
					  ofToString((int) curWidth) + "x" + ofToString((int) curHeight));
				if(curCoding == targetCoding) {
					float curDistance = ofDist(curWidth, curHeight, width, height);
					if(!found || curDistance < bestDistance) {
						bestMode = curMode;
						bestDistance = curDistance;
					}
					found = true;
				}
			}
		}
		
		if(!found) {
			ofLog(OF_LOG_ERROR, "Camera does not support target color coding.");
			camera = NULL;
			return false;
		} else {
			unsigned int bestWidth, bestHeight;
			dc1394_get_image_size_from_video_mode(camera, bestMode, &bestWidth, &bestHeight);
			width = bestWidth;
			height = bestHeight;
			videoMode = bestMode;
		}
		
		// get fastest framerate
		// todo: make this settable
		dc1394framerates_t framerates;
		dc1394_video_get_supported_framerates(camera, videoMode, &framerates);
		framerate = framerates.framerates[framerates.num - 1];
	}
	
	if(useFormat7) {
		quantizePosition();
		quantizeSize();
		dc1394_format7_set_roi(camera, videoMode, getLibdcType(imageType), DC1394_USE_MAX_AVAIL, left, top, width, height);
		unsigned int curWidth, curHeight;
		dc1394_format7_get_image_size(camera, videoMode, &curWidth, &curHeight);
		ofLog(OF_LOG_VERBOSE, "Using mode: " + ofToString((int) width) + "x" + ofToString((int) height));
	} else {
		dc1394_video_set_framerate(camera, framerate);
		ofLog(OF_LOG_VERBOSE, "Using mode: " + ofToString((int) width) + "x" + ofToString((int) height) + " " + makeString(framerate) + "fps");
	}
	
	// contrary to the libdc1394 format7 demo, this should go after the roi setting
	dc1394_video_set_mode(camera, videoMode);
		
	dc1394_capture_setup(camera, OFXLIBDC_BUFFER_SIZE, DC1394_CAPTURE_FLAGS_DEFAULT);
	
	return true;
}

void Camera::quantizePosition() {
	if(camera) {
		unsigned int hunit, vunit;
		dc1394_format7_get_unit_position(camera, videoMode, &hunit, &vunit);
		left = (left / hunit) * hunit;
		top = (top / vunit) * vunit;
	}
}

void Camera::quantizeSize() {
	if(camera) {
		unsigned int hunit, vunit;
		dc1394_format7_get_unit_size(camera, videoMode, &hunit, &vunit);
		width = (width / hunit) * hunit;
		height = (height / vunit) * vunit;
	}
}

void Camera::printFeatures() const {
	dc1394featureset_t features;
	if(camera) {
		dc1394_feature_get_all(camera, &features);
		dc1394_feature_print_all(&features, stdout);
	}
}

void Camera::setBrightness(unsigned int brightness) {
	setFeature(DC1394_FEATURE_BRIGHTNESS, brightness);
}

void Camera::setGamma(unsigned int gamma) {
	setFeature(DC1394_FEATURE_GAMMA, gamma);
}

void Camera::setGain(unsigned int gain) {
	setFeature(DC1394_FEATURE_GAIN, gain);
}

void Camera::setExposure(unsigned int exposure) {
	setFeature(DC1394_FEATURE_EXPOSURE, exposure);
}

void Camera::setShutter(unsigned int shutter) {
	setFeature(DC1394_FEATURE_SHUTTER, shutter);
}

void Camera::setBrightnessNorm(float brightness) {
	setFeatureNorm(DC1394_FEATURE_BRIGHTNESS, brightness);
}

void Camera::setGammaNorm(float gamma) {
	setFeatureNorm(DC1394_FEATURE_GAMMA, gamma);
}

void Camera::setGainNorm(float gain) {
	setFeatureNorm(DC1394_FEATURE_GAIN, gain);
}

void Camera::setExposureNorm(float exposure) {
	setFeatureNorm(DC1394_FEATURE_EXPOSURE, exposure);
}

void Camera::setShutterNorm(float shutter) {
	setFeatureNorm(DC1394_FEATURE_SHUTTER, shutter);
}

inline void Camera::setFeature(dc1394feature_t feature, unsigned int value) {
	if(camera) {
		dc1394_feature_set_power(camera, feature, DC1394_ON);
		dc1394_feature_set_mode(camera, feature, DC1394_FEATURE_MODE_MANUAL);
		dc1394_feature_set_value(camera, feature, value);
	}
}

void Camera::setFeatureNorm(dc1394feature_t feature, float value) {
	if(camera) {
		unsigned int min, max;
		getFeatureRange(feature, &min, &max);
		dc1394_feature_set_power(camera, feature, DC1394_ON);
		dc1394_feature_set_mode(camera, feature, DC1394_FEATURE_MODE_MANUAL);
		dc1394_feature_set_value(camera, feature, value * (max - min) + min);
	}
}

void Camera::getBrightnessRange(unsigned int* min, unsigned int* max) const {
	getFeatureRange(DC1394_FEATURE_BRIGHTNESS, min, max);
}

void Camera::getGammaRange(unsigned int* min, unsigned int* max) const {
	getFeatureRange(DC1394_FEATURE_GAMMA, min, max);
}

void Camera::getGainRange(unsigned int* min, unsigned int* max) const {
	getFeatureRange(DC1394_FEATURE_GAIN, min, max);
}

void Camera::getExposureRange(unsigned int* min, unsigned int* max) const {
	getFeatureRange(DC1394_FEATURE_EXPOSURE, min, max);
}

void Camera::getShutterRange(unsigned int* min, unsigned int* max) const {
	getFeatureRange(DC1394_FEATURE_SHUTTER, min, max);
}

void Camera::getFeatureRange(dc1394feature_t feature, unsigned int* min, unsigned int* max) const {
	if(camera) {
		dc1394_feature_get_boundaries(camera, feature, min, max);
	}
}

unsigned int Camera::getBrightness() {
	return getFeature(DC1394_FEATURE_BRIGHTNESS);
}

unsigned int Camera::getGamma() {
	return getFeature(DC1394_FEATURE_GAMMA);
}

unsigned int Camera::getGain() {
	return getFeature(DC1394_FEATURE_GAIN);
}

unsigned int Camera::getExposure() {
	return getFeature(DC1394_FEATURE_EXPOSURE);
}

unsigned int Camera::getShutter() {
	return getFeature(DC1394_FEATURE_SHUTTER);
}

unsigned int Camera::getFeature(dc1394feature_t feature) {
	unsigned int value = 0;
	if(camera) {
		dc1394_feature_get_value(camera, feature, &value);
	}
	return value;
}

float Camera::getBrightnessNorm() {
	return getFeatureNorm(DC1394_FEATURE_BRIGHTNESS);
}

float Camera::getGammaNorm() {
	return getFeatureNorm(DC1394_FEATURE_GAMMA);
}

float Camera::getGainNorm() {
	return getFeatureNorm(DC1394_FEATURE_GAIN);
}

float Camera::getExposureNorm() {
	return getFeatureNorm(DC1394_FEATURE_EXPOSURE);
}

float Camera::getShutterNorm() {
	return getFeatureNorm(DC1394_FEATURE_SHUTTER);
}

float Camera::getFeatureNorm(dc1394feature_t feature) {
	unsigned int value = getFeature(feature);
	unsigned int min, max;
	getFeatureRange(feature, &min, &max);
	return ((float) value - min) / (max - min);
}

float Camera::getShutterAbs() const {
	return getFeatureAbs(DC1394_FEATURE_SHUTTER);
}

float Camera::getFeatureAbs(dc1394feature_t feature) const {
	float value = 0;
	if(camera) {
		dc1394_feature_get_absolute_value(camera, feature, &value);
	}
	return value;
}

unsigned int Camera::getWidth() const {
	return width;
}

unsigned int Camera::getHeight() const {
	return height;
}

ofImageType Camera::getImageType() const {
	return imageType;
}

void Camera::setImageType(ofImageType imageType) {
	this->imageType = imageType;
}

void Camera::setTransmit(bool transmit) {
	if(camera) {
		dc1394switch_t cur, target;
		dc1394_video_get_transmission(camera, &cur);
		target = transmit ? DC1394_ON : DC1394_OFF;
		if(cur != target)
			dc1394_video_set_transmission(camera, target);
	}
}

void Camera::grabStill(ofImage& img) {
	if(camera) {
		setTransmit(false);
		flushBuffer();
		dc1394_video_set_one_shot(camera, DC1394_ON);
		grabFrame(img);
	}
}

bool Camera::grabVideo(ofImage& img, bool dropFrames) {
	if(camera) {
		setTransmit(true);
		if(dropFrames) {
			bool remaining;
			int i = 0;
			do {
				remaining = grabFrame(img);
				if(!remaining && i == 0)
					return false;
				i++;
			} while (remaining);
			return true;
		} else {
			return grabFrame(img);
		}
	} else {
		return false;
	}
}

bool Camera::grabFrame(ofImage& img) {
	if(camera) {
		dc1394video_frame_t *frame;
		dc1394_capture_dequeue(camera, capturePolicy, &frame);
		if(frame != NULL) {
			// don't trust allocate() to be smart. should also check for imageType change.
			if(img.getWidth() != width || img.getHeight() != height) {
				img.allocate(width, height, imageType);
			}
			unsigned char* src = frame->image;
			unsigned char* dst = img.getPixels();
			if(imageType == OF_IMAGE_GRAYSCALE) {
				memcpy(dst, src, width * height);
			} else if(imageType == OF_IMAGE_COLOR) {
				unsigned int bits = width * height * img.getPixelsRef().getBitsPerPixel();
				
				if( useBayer ){
					dc1394_bayer_decoding_8bit(src, dst, width, height, bayerMode, DC1394_BAYER_METHOD_BILINEAR);
				}else{
					dc1394_convert_to_RGB8(src, dst, width, height, 0, getLibdcType(imageType), bits);
				}
			}
			dc1394_capture_enqueue(camera, frame);
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

void Camera::flushBuffer() {
	if(camera) {
		dc1394video_frame_t *frame;
		do {
			dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_POLL, &frame);
			if(frame != NULL)
				dc1394_capture_enqueue(camera, frame);
		} while (frame != NULL);
	}
}

dc1394camera_t* Camera::getLibdcCamera() {
	return camera;
}

bool Camera::ready() const {
	return camera != NULL;
}

/*
 This macro is used for converting an enum like DC1394_COLOR_CODING_MONO8 to the code:
 
 case DC1394_COLOR_CODING_MONO8: return "DC1394_COLOR_CODING_MONO8"; break;
 
 Which can be used as part of a big switch statement for printing enum names dynamically.
 The slightly unusual "#" in the macro is responsible for surrounding the enum in quotes.
 */
#define enumCase( name ) {case name: return # name; break;}

string Camera::makeString(int name) {
	switch(name) {
		enumCase(DC1394_COLOR_CODING_MONO8)
		enumCase(DC1394_COLOR_CODING_YUV411)
		enumCase(DC1394_COLOR_CODING_YUV422)
		enumCase(DC1394_COLOR_CODING_RGB8)
		enumCase(DC1394_COLOR_CODING_MONO16)
		enumCase(DC1394_COLOR_CODING_RGB16)
		enumCase(DC1394_COLOR_CODING_MONO16S)
		enumCase(DC1394_COLOR_CODING_RGB16S)
		enumCase(DC1394_COLOR_CODING_RAW8)
		enumCase(DC1394_COLOR_CODING_RAW16)
		enumCase(DC1394_FRAMERATE_1_875)
		enumCase(DC1394_FRAMERATE_3_75)
		enumCase(DC1394_FRAMERATE_7_5)
		enumCase(DC1394_FRAMERATE_15)
		enumCase(DC1394_FRAMERATE_30)
		enumCase(DC1394_FRAMERATE_60)
		enumCase(DC1394_FRAMERATE_120)
		enumCase(DC1394_FRAMERATE_240)
	}
}

}
