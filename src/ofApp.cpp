#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(30);
	receiver.setup(6666);
	in_error = false;
	ofBackground(76, 153, 0);
	font.loadFont(OF_TTF_MONO, 72);

	parameters.setName("settings");
	parameters.add(movie_file.set("movie_file", "Movie.mov"));

	ofXml xml("settings.xml");

	xml.deserialize(parameters);

	std::string movie_path = parameters.getString("movie_file");

	player.loadMovie(movie_path);
	player.setLoopState(OF_LOOP_NORMAL);
	player.play();

	/*
	ofXml xml;
	xml.serialize(parameters);
	xml.save("settings.xml");
	*/
}

//--------------------------------------------------------------
void ofApp::update(){
	bool was_updated = false;

	while (receiver.hasWaitingMessages())
	{
		ofxOscMessage message;
		receiver.getNextMessage(&message);
		if (message.getAddress() == "/frame_number")
		{
			was_updated = true;

			bool previous_error_state = in_error;

			current_frame_number = message.getArgAsInt64(0);

			frame_numbers.push_back(current_frame_number);

			while (frame_numbers.size() > 100)
			{
				frame_numbers.pop_front();
			}

			bool in_error = false;

			int previous = 0;
			for (std::list<int>::const_iterator it = frame_numbers.begin(); it != frame_numbers.end(); ++it) {
				if (it != frame_numbers.begin())
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

	if (was_updated)
	{
		// For now, if frame number is out of range, loop around
		int total_frames = player.getTotalNumFrames();
		if (total_frames > 0)
		{
			long actual_frame = current_frame_number % total_frames;
			player.setFrame(actual_frame);
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	if (player.isLoaded())
	{
		player.draw(0, 0);
	}

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
