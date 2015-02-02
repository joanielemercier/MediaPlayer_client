#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(30);
	receiver.setup(6666);
	in_error = true;
    show_stats = true;
	current_frame_number = 0;
	ofBackground(76, 153, 0);
	font.loadFont(OF_TTF_MONO, 72);

	parameters.setName("settings");
	parameters.add(movie_file.set("movie_file", "Movie.mov"));

	ofXml xml("settings.xml");

	xml.deserialize(parameters);

	std::string movie_path = parameters.getString("movie_file");

	player.loadMovie(movie_path);
	player.setLoopState(OF_LOOP_NORMAL);
	player.setSpeed(0.0);
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
		if (message.getAddress() == "/frame_number_reset")
		{
			frame_numbers.clear();
			current_frame_number = message.getArgAsInt64(0);
			in_error = false;
			was_updated = true;
		}
		else if (message.getAddress() == "/frame_number")
		{
			bool previous_error_state = in_error;

			long incoming_frame_number = message.getArgAsInt64(0);

			if (incoming_frame_number > current_frame_number)
			{
				current_frame_number = incoming_frame_number;
				was_updated = true;
			}

			frame_numbers.push_back(incoming_frame_number);

			while (frame_numbers.size() > 100)
			{
				frame_numbers.pop_front();
			}

			in_error = false;

			int previous = 0;
			for (std::list<int>::const_iterator it = frame_numbers.begin(); it != frame_numbers.end(); ++it) {
				if (it != frame_numbers.begin())
				{
					if (previous != *it - 1)
					{
						if (show_stats)
						{
							ofLog() << "expected frame " << previous + 1 << " got " << *it;
						}
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
		else if (message.getAddress() == "/display_stats")
		{
			show_stats = message.getArgAsInt32(0);
		}
		else
		{
			ofLog() << "Unexpected message address: " << message.getAddress();
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

    player.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
	if (player.isLoaded())
	{
		player.draw(0, 0);
	}

	if (show_stats)
	{
		std::string rate_message = "frame-rate: " + ofToString(ofGetFrameRate());

		ofDrawBitmapString(rate_message, 20, 30);

		if (in_error)
		{
			ofDrawBitmapString("Frame discontinuity", 20, 60);
		}

		std::string message = ofToString(current_frame_number);

		font.drawString(message, (ofGetWindowWidth() / 2) - (font.stringWidth(message) / 2), (ofGetWindowHeight() / 2) - (font.stringHeight(message) / 2));
	}
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
