#pragma once

#include "ofMain.h"
#include <ofxOsc.h>
#include <ofxHapPlayer.h>

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

		ofxOscReceiver receiver;
		std::list<int> frame_numbers;
		bool in_error;
		int current_frame_number;
		bool show_stats;

		ofTrueTypeFont font;

		ofParameterGroup parameters;
		ofParameter<std::string> movie_file;
		
		ofxHapPlayer player;

        std::string frame_number_errors;
};
