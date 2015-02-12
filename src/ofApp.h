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
        class Output {
            friend class ofApp;
        public:
            Output(std::string name);
            ~Output();
            Output& operator = (const Output& b);
            Output(const Output& p);
            void update(float image_width, float image_height);
            void draw(ofTexture& texture, bool show_stats);
            void doOSCEvent(const std::string& local_address, const ofxOscMessage& message, bool& frame_was_updated, bool& missed_frames_need_checked);
        private:
            void updatePointParameter(std::string name, int index, float value);
            void parameterChanged(ofAbstractParameter & parameter);
            std::string name;

            bool dimensions_changed;
            bool parameters_changed;
            ofRectangle bounding_box;
            ofRectangle crop_box;

            std::vector<ofMesh> blends;
            ofxGLWarper warper;

            std::shared_ptr<ofParameterGroup> parameters;
        };
        void doClientOSCEvent(const std::string& local_address,
                              const ofxOscMessage& message,
                              bool& frame_was_updated,
                              bool& missed_frames_need_checked,
                              bool& outputs_were_reconfigured);
        void parameterChanged(ofAbstractParameter & parameter);

		ofxOscReceiver receiver;
		std::list<int> frame_numbers;

        bool source_changed;
		bool in_error;
		int current_frame_number;
        std::map<std::string, Output> outputs;

		ofTrueTypeFont font;

		ofParameterGroup client_parameters;
		bool client_parameters_changed;

		ofxHapPlayer player;
        ofxHapImageSequence sequence;
        ofxHapImage image;

        bool use_sequence;
        std::string frame_number_errors;

        ofPoint image_dimensions;

};
