#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(60);
	receiver.setup(6666);
	in_error = true;
    show_stats = true;
	current_frame_number = 0;
	ofBackground(76, 153, 0);
	font.loadFont(OF_TTF_MONO, 72);

	parameters.setName("settings");
    ofParameter<std::string> movie_file("movie_file", "Movie.mov");
	parameters.add(movie_file);

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
	bool frame_was_updated = false;
    bool missed_frames_need_checked = false;

	while (receiver.hasWaitingMessages())
	{
		ofxOscMessage message;
		receiver.getNextMessage(&message);
		if (message.getAddress() == "/frame_number_reset")
		{
			frame_numbers.clear();
			current_frame_number = message.getArgAsInt64(0);
			in_error = false;
			frame_was_updated = true;
            missed_frames_need_checked = true;
		}
		else if (message.getAddress() == "/frame_number")
		{
			long incoming_frame_number = message.getArgAsInt64(0);

			if (incoming_frame_number > current_frame_number)
			{
				current_frame_number = incoming_frame_number;
				frame_was_updated = true;
			}

			frame_numbers.push_back(incoming_frame_number);

            while (frame_numbers.size() > 300)
			{
				frame_numbers.pop_front();
			}
            missed_frames_need_checked = true;
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

    if (missed_frames_need_checked)
    {
        bool previous_error_state = in_error;
        in_error = false;
        frame_number_errors.clear();

        int previous = 0;
        for (std::list<int>::const_iterator it = frame_numbers.begin(); it != frame_numbers.end(); ++it) {
            if (it != frame_numbers.begin())
            {
                if (previous != *it - 1)
                {
                    if (!frame_number_errors.empty())
                    {
                        frame_number_errors += ",";
                    }
                    frame_number_errors += ofToString(previous) + "->" + ofToString(*it);
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
    }

	if (frame_was_updated)
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
        std::vector<std::string> messages;
        messages.push_back(ofToString(ofGetFrameRate(), 0) + " FPS");

        if (!player.isLoaded())
        {
            std::string movie_path = parameters.getString("movie_file");
            messages.push_back("Movie not loaded: " + movie_path);
        }
		if (in_error)
		{
            messages.push_back("Frame discontinuities: " + frame_number_errors);
		}

        float y_offset = 20;
        for (std::vector<std::string>::const_iterator it = messages.begin(); it != messages.end(); ++it)
        {
            ofDrawBitmapString(*it, 10, y_offset);
            y_offset += 20;
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
