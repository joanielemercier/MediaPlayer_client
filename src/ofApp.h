#pragma once

#include "ofMain.h"
#include <ofxOsc.h>
#include <ofxHapPlayer.h>
#include <ofxHapImageSequence.h>
#include <ofxGLWarper.h>

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

private:
        void doOSCEvent(std::string local_address, const ofxOscMessage& message, bool& frame_was_updated, bool& missed_frames_need_checked);
        void parameterChanged(ofAbstractParameter & parameter);
        void updatePointParameter(std::string name, int index, float value);
		ofxOscReceiver receiver;
		std::list<int> frame_numbers;
		bool in_error;
		int current_frame_number;

		ofTrueTypeFont font;

		ofParameterGroup parameters;
		bool parameters_changed;
        bool source_changed;

		ofxHapPlayer player;
        ofxHapImageSequence sequence;
        ofxHapImage image;

        bool use_sequence;
        std::string frame_number_errors;

        ofxGLWarper warper;

        ofPoint image_dimensions;
        bool dimensions_changed;
        ofRectangle bounding_box;
        ofRectangle crop_box;

        std::vector<ofMesh> blends;
};
