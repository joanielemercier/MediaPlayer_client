#include "ofApp.h"

static long getMessageInteger(const ofxOscMessage& message, int index)
{
    switch (message.getArgType(index))
    {
    case OFXOSC_TYPE_INT32:
        return message.getArgAsInt32(index);
    case OFXOSC_TYPE_INT64:
    case OFXOSC_TYPE_FLOAT:
        return message.getArgAsInt64(index);
    case OFXOSC_TYPE_STRING:
        return ofToInt(message.getArgAsString(index));
    default:
        return 0;
    }
}

static float getMessageFloat(const ofxOscMessage& message, int index)
{
    switch (message.getArgType(index))
    {
    case OFXOSC_TYPE_INT32:
    case OFXOSC_TYPE_FLOAT:
        return message.getArgAsFloat(index); // also handles the int32 case
    case OFXOSC_TYPE_INT64:
        return message.getArgAsInt64(index);
    case OFXOSC_TYPE_STRING:
        return ofToInt(message.getArgAsString(index));
    default:
        return 0;
    }
}

static std::string stripWhiteSpace(const std::string& string)
{
    std::string::size_type first = string.find_first_not_of(" \t");
    if (first == std::string::npos)
    {
        return "";
    }
    std::string::size_type last = string.find_last_not_of(" \t");
    return string.substr(first, last - first + 1);
}

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(60);
	receiver.setup(6666);
	in_error = true;
    show_stats = true;
	current_frame_number = 0;
    parameters_changed = false;
    dimensions_changed = true;
	ofBackground(76, 153, 0);
	font.loadFont(OF_TTF_MONO, 72);

    /*
    Define default settings and then override them with any stored in settings.xml
    */
    std::string random_string = ofToHex(uint16_t(ofRandom(UINT16_MAX)));

	parameters.setName("settings");
    ofParameter<std::string> source_param("source", "Movie.mov");
    parameters.add(source_param);
    ofParameter<std::string> client_id_param("client_id", random_string);
    parameters.add(client_id_param);
    ofParameter<ofPoint> crop_origin_param("crop_origin", ofPoint(0.0, 0.0));
    parameters.add(crop_origin_param);
    ofParameter<float> crop_width_param("crop_width", 0);
    parameters.add(crop_width_param);
    ofParameter<float> crop_height_param("crop_height", 0);
    parameters.add(crop_height_param);
    ofParameter<ofPoint> warp_top_left_param("warp_top_left", ofPoint(0.0, 0.0));
    parameters.add(warp_top_left_param);
    ofParameter<ofPoint> warp_top_right_param("warp_top_right", ofPoint(0.0, 0.0));
    parameters.add(warp_top_right_param);
    ofParameter<ofPoint> warp_bottom_right_param("warp_bottom_right", ofPoint(0.0, 0.0));
    parameters.add(warp_bottom_right_param);
    ofParameter<ofPoint> warp_bottom_left_param("warp_bottom_left", ofPoint(0.0, 0.0));
    parameters.add(warp_bottom_left_param);

	ofXml xml("settings.xml");

	xml.deserialize(parameters);

    /*
    Save settings now so that client_id is saved if we generated it
    */
	parameters_changed = true;

    /*
    Monitor any subsequent settings changes so we can save them
    */
    ofAddListener(parameters.parameterChangedE, this, &ofApp::parameterChanged);

    /*
    Load our source
    */

    std::string source_path = parameters.getString("source");

    if (ofFile(source_path).isDirectory() || ofFilePath::getFileExt(source_path) == ofxHapImage::HapImageFileExtension())
    {
        sequence.load(source_path);
        use_sequence = true;
    }
    else
    {
        player.loadMovie(source_path);
        player.setLoopState(OF_LOOP_NORMAL);
        player.setSpeed(0.0);
        player.play();
        use_sequence = false;
    }
}

//--------------------------------------------------------------
void ofApp::update(){
	bool frame_was_updated = false;
    bool missed_frames_need_checked = false;

    while (receiver.hasWaitingMessages())
    {
        ofxOscMessage message;
        receiver.getNextMessage(&message);

        std::string address = stripWhiteSpace(message.getAddress());

        std::string client_prefix = "/client/";
        if (address.compare(0, client_prefix.length(), client_prefix) == 0)
        {
            client_prefix += parameters.getString("client_id");
            if (address.compare(0, client_prefix.length(), client_prefix) == 0)
            {
                doOSCEvent(address.substr(client_prefix.length(), std::string::npos), message, frame_was_updated, missed_frames_need_checked);
            }
        }
        else
        {
            doOSCEvent(address, message, frame_was_updated, missed_frames_need_checked);
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
        int total_frames = use_sequence ? sequence.size() : player.getTotalNumFrames();
        if (total_frames > 0)
        {
            long actual_frame = current_frame_number % total_frames;
            if (use_sequence)
            {
                image = sequence[actual_frame];
                if (image.getWidth() != image_dimensions.x || image.getHeight() != image_dimensions.y)
                {
                    dimensions_changed = true;
                }
            }
            else
            {
                player.setFrame(actual_frame);
                if (player.getWidth() != image_dimensions.x || player.getHeight() != image_dimensions.y)
                {
                    dimensions_changed = true;
                }
            }
        }
    }

    if (parameters_changed)
    {
        /*
        Save the changed parameters
        */
        ofXml xml("settings.xml");
        xml.serialize(parameters);
        xml.save("settings.xml");

        dimensions_changed = true;
        parameters_changed = false;
    }
    if (dimensions_changed)
    {
        /*
        Update the warper
        */
        image_dimensions.x = (use_sequence ? image.getWidth() : player.getWidth());
        image_dimensions.y = (use_sequence ? image.getHeight() : player.getHeight());

        bounding_box.set(0.0, 0.0, image_dimensions.x, image_dimensions.y);
        bounding_box.scaleTo(ofGetWindowRect());

        warper.setup(bounding_box.x, bounding_box.y, bounding_box.width, bounding_box.height);
        warper.setCorner(ofxGLWarper::TOP_LEFT, parameters.getPoint("warp_top_left").get() + warper.getCorner(ofxGLWarper::TOP_LEFT));
        warper.setCorner(ofxGLWarper::TOP_RIGHT, parameters.getPoint("warp_top_right").get() + warper.getCorner(ofxGLWarper::TOP_RIGHT));
        warper.setCorner(ofxGLWarper::BOTTOM_LEFT, parameters.getPoint("warp_bottom_left").get() + warper.getCorner(ofxGLWarper::BOTTOM_LEFT));
        warper.setCorner(ofxGLWarper::BOTTOM_RIGHT, parameters.getPoint("warp_bottom_right").get() + warper.getCorner(ofxGLWarper::BOTTOM_RIGHT));
        dimensions_changed = false;
    }
    player.update();
}

void ofApp::doOSCEvent(std::string local_address, const ofxOscMessage& message, bool& frame_was_updated, bool& missed_frames_need_checked)
{
    if (local_address == "/frame_number_reset")
    {
        frame_numbers.clear();
        if (message.getNumArgs() == 1)
        {
            current_frame_number = getMessageInteger(message, 0);
        }
        else
        {
            current_frame_number = 0;
        }
        in_error = false;
        frame_was_updated = true;
        missed_frames_need_checked = true;
    }
    else if (local_address == "/frame_number" && message.getNumArgs() == 1)
    {
        long incoming_frame_number = getMessageInteger(message, 0);

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
    else if (local_address == "/display_stats" && message.getNumArgs() == 1)
    {
        show_stats = getMessageInteger(message, 0);
    }
    else if (local_address == "/crop/x" && message.getNumArgs() == 1)
    {
        updatePointParameter("crop_origin", 0, getMessageFloat(message, 0));
    }
    else if (local_address == "/crop/y" && message.getNumArgs() == 1)
    {
        updatePointParameter("crop_origin", 1, getMessageFloat(message, 0));
    }
    else if (local_address == "/crop/width" && message.getNumArgs() == 1)
    {
        parameters["crop_width"].cast<int>() = getMessageFloat(message, 0);
    }
    else if (local_address == "/crop/height" && message.getNumArgs() == 1)
    {
        parameters["crop_height"].cast<int>() = getMessageFloat(message, 0);
    }
    else if (local_address == "/warp/top_left/x" && message.getNumArgs() == 1)
    {
        updatePointParameter("warp_top_left", 0, getMessageFloat(message, 0));
    }
    else if (local_address == "/warp/top_left/y" && message.getNumArgs() == 1)
    {
        updatePointParameter("warp_top_left", 1, -getMessageFloat(message, 0));
    }
    else if (local_address == "/warp/top_right/x" && message.getNumArgs() == 1)
    {
        updatePointParameter("warp_top_right", 0, getMessageFloat(message, 0));
    }
    else if (local_address == "/warp/top_right/y" && message.getNumArgs() == 1)
    {
        updatePointParameter("warp_top_right", 1, -getMessageFloat(message, 0));
    }
    else if (local_address == "/warp/bottom_right/x" && message.getNumArgs() == 1)
    {
        updatePointParameter("warp_bottom_right", 0, getMessageFloat(message, 0));
    }
    else if (local_address == "/warp/bottom_right/y" && message.getNumArgs() == 1)
    {
        updatePointParameter("warp_bottom_right", 1, -getMessageFloat(message, 0));
    }
    else if (local_address == "/warp/bottom_left/x" && message.getNumArgs() == 1)
    {
        updatePointParameter("warp_bottom_left", 0, getMessageFloat(message, 0));
    }
    else if (local_address == "/warp/bottom_left/y" && message.getNumArgs() == 1)
    {
        updatePointParameter("warp_bottom_left", 1, -getMessageFloat(message, 0));
    }
    else
    {
        ofLog() << "Unexpected message address: " << message.getAddress();
    }
}

//--------------------------------------------------------------
void ofApp::draw(){

    warper.begin();
    if (use_sequence)
    {        
        image.draw(bounding_box.x, bounding_box.y, bounding_box.width, bounding_box.height);
    }
    else if (player.isLoaded())
    {
        player.draw(bounding_box.x, bounding_box.y, bounding_box.width, bounding_box.height);
    }
    warper.end();

    if (show_stats)
    {
        warper.begin();
        ofPushStyle();
        ofSetColor(128, 128, 128, 128);
        ofSetLineWidth(1.0);
        ofFill();
        ofRect(bounding_box);
        ofPopStyle();
        warper.end();

        std::vector<std::string> messages;
        std::string client_id = parameters.getString("client_id");
        messages.push_back("Client ID: " + client_id + " " + ofToString(ofGetFrameRate(), 0) + " FPS");

        if (use_sequence == false && !player.isLoaded())
        {
            std::string source_path = parameters.getString("source");
            messages.push_back("Frame source not loaded: " + source_path);
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
    dimensions_changed = true;
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

void ofApp::parameterChanged(ofAbstractParameter & parameter)
{
    parameters_changed = true;
}

void ofApp::updatePointParameter(std::string name, int index, float value)
{
    ofPoint current = parameters[name].cast<ofPoint>();
    current[index] = value;
    parameters[name].cast<ofPoint>() = current;
}
