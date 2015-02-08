#pragma once

#include "ofMain.h"
#include <ofxOsc.h>
#include <ofxHapPlayer.h>
#include <ofxHapImageSequence.h>

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

        void doOSCEvent(std::string local_address, const ofxOscMessage& message, bool& frame_was_updated, bool& missed_frames_need_checked);

		ofxOscReceiver receiver;
		std::list<int> frame_numbers;
		bool in_error;
		int current_frame_number;
		bool show_stats;

		ofTrueTypeFont font;

		ofParameterGroup parameters;
		
		ofxHapPlayer player;
        ofxHapImageSequence sequence;
        ofxHapImage image;

        bool use_sequence;
        std::string frame_number_errors;
};
