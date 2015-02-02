#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(30);
	receiver.setup(6666);
	in_error = false;
	ofBackground(76, 153, 0);
	font.loadFont(OF_TTF_MONO, 72);
}

//--------------------------------------------------------------
void ofApp::update(){
	while (receiver.hasWaitingMessages())
	{
		ofxOscMessage message;
		receiver.getNextMessage(&message);
		if (message.getAddress() == "/frame_number")
		{
			bool previous_error_state = in_error;

			current_frame_number = message.getArgAsInt64(0);

			frame_numbers.push_back(current_frame_number);

			while (frame_numbers.size() > 100)
			{
				frame_numbers.pop_front();
			}

			bool in_error = false;

			int previous = 0;
			for (std::list<int>::const_iterator it = frame_numbers.cbegin(); it != frame_numbers.cend(); ++it) {
				if (it != frame_numbers.cbegin())
				{
					if (previous != *it - 1)
					{
						in_error = true;
					}
				}
				previous = *it;
			}

			if (in_error != previous_error_state)
			{
				if (in_error)
				{
					ofBackground(204, 0, 0);
				}
				else
				{
					ofBackground(76, 153, 0);
				}
			}

			previous_error_state = in_error;
		}

	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	if (in_error)
	{
		ofDrawBitmapString("Frame discontinuity", 100, 100);
	}

	std::string message = ofToString(current_frame_number);

	font.drawString(message, (ofGetWindowWidth() / 2) - (font.stringWidth(message) / 2), (ofGetWindowHeight() / 2) - (font.stringHeight(message) / 2));
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
