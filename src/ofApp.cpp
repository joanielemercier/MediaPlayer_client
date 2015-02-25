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
	current_frame_number = 0;
    client_parameters_changed = false;
    source_changed = true;
	ofBackground(0);
	font.loadFont(OF_TTF_MONO, 72);

    /*
    Define default settings and then override them with any stored in settings.xml
    */
    std::string random_string = ofToHex(uint16_t(ofRandom(UINT16_MAX)));

	client_parameters.setName("settings");
    ofParameter<std::string> source_param("source", "Movie.mov");
    client_parameters.add(source_param);
    ofParameter<bool> show_stats_param("show_stats", true);
    client_parameters.add(show_stats_param);
    ofParameter<std::string> client_id_param("client_id", random_string);
    client_parameters.add(client_id_param);
    ofParameter<bool> fullscreen_param("full_screen", false);
    client_parameters.add(fullscreen_param);
    ofParameter<bool> first_run_param("first_run", true);
    client_parameters.add(first_run_param);
    
	ofXml xml("settings.xml");

	xml.deserialize(client_parameters);

    /*
     Create outputs to match our settings
     */
    if (xml.exists("settings/outputs"))
    {
        xml.setTo("settings/outputs");
        int count = xml.getNumChildren();
        for (int i = 0; i < count; i++)
        {
            xml.setToChild(i);
            std::string output_name = xml.getAttribute("id");
            if (xml.getNumChildren() == 1)
            {
                outputs.insert(std::pair<std::string, Output>(output_name, Output(output_name)));
                xml.deserialize(*outputs.find(output_name)->second.parameters);
            }
            xml.setToParent();
        }
    }
    else if (client_parameters.getBool("first_run"))
    {
        /*
         If this is the first run and no outputs were in settings.xml, create a default output
         */
        outputs.insert(std::pair<std::string, Output>("1", Output("1")));
        outputs.find("1")->second.parameters_changed = true;
        client_parameters["first_run"].cast<bool>() = false;
    }

    /*
    Save settings now so that client_id is saved if we generated it
    */
	client_parameters_changed = true;

    /*
    Monitor any subsequent settings changes so we can save them
    */
    ofAddListener(client_parameters.parameterChangedE, this, &ofApp::parameterChanged);

    /*
    Restore full-screen state
    */
    ofSetFullscreen(client_parameters.getBool("full_screen"));
}

//--------------------------------------------------------------
void ofApp::update(){
	bool frame_was_updated = false;
    bool missed_frames_need_checked = false;
    bool outputs_were_reconfigured = false;

    while (receiver.hasWaitingMessages())
    {
        ofxOscMessage message;
        receiver.getNextMessage(&message);

        std::string address = stripWhiteSpace(message.getAddress());

        std::vector<std::string> parts = ofSplitString(address, "/");
        if (parts.size() > 1)
        {
            if (parts[1] == "client")
            {
                std::string client_id = client_parameters.getString("client_id");
                if (parts.size() > 2 && parts[2] == client_id)
                {
                    if (parts.size() > 3 && parts[3] == "output")
                    {
                        std::string output = parts[4];
                        parts.erase(parts.begin(), parts.begin() + 5);
                        address = "/" + ofJoinString(parts, "/");
                        if (outputs.count(output) > 0)
                        {
                            outputs.find(output)->second.doOSCEvent(address, message, frame_was_updated, missed_frames_need_checked);
                        }
                        else
                        {
                            ofLogWarning() << "Ignoring OSC message for unknown output \"" << output << "\"";
                        }
                    }
                    else
                    {
                        parts.erase(parts.begin(), parts.begin() + 3);
                        address = "/" + ofJoinString(parts, "/");
                        doClientOSCEvent(address, message, frame_was_updated, missed_frames_need_checked, outputs_were_reconfigured);
                    }
                }
            }
            else
            {
                doClientOSCEvent(address, message, frame_was_updated, missed_frames_need_checked, outputs_were_reconfigured);
            }
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
    }

    if (source_changed)
    {
        /*
        Load our source
        */

        std::string source_path = client_parameters.getString("source");

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

        source_changed = false;
    }

    if (frame_was_updated)
    {
        // For now, if frame number is out of range, loop around
        int total_frames = use_sequence ? sequence.size() : player.getTotalNumFrames();
        if (total_frames > 0)
        {
            bool dimensions_changed = false;
            long actual_frame = current_frame_number % total_frames;
            if (use_sequence)
            {
                image.loadImage(sequence[actual_frame]);
                if (image.getWidth() != image_dimensions.x || image.getHeight() != image_dimensions.y)
                {
                    image_dimensions.set(image.getWidth(), image.getHeight());
                    dimensions_changed = true;
                }
            }
            else
            {
                player.setFrame(actual_frame);
                if (player.getWidth() != image_dimensions.x || player.getHeight() != image_dimensions.y)
                {
                    image_dimensions.set(player.getWidth(), player.getHeight());
                    dimensions_changed = true;
                }
            }
            if (dimensions_changed)
            {
                for (std::map<std::string, Output>::iterator it = outputs.begin(); it != outputs.end(); ++it) {
                    it->second.dimensions_changed = true;
                }
            }
        }
    }

    for (std::map<std::string, Output>::iterator it = outputs.begin(); it != outputs.end(); ++it) {
        if (it->second.parameters_changed)
        {
            it->second.parameters_changed = false;
            outputs_were_reconfigured = true;
        }
    }

    if (client_parameters_changed || outputs_were_reconfigured)
    {
        /*
        Save the changed parameters
         Don't load the existing ones here because https://github.com/openframeworks/openFrameworks/issues/3643
        */
        ofXml xml;
        xml.serialize(client_parameters);

        xml.setTo("//settings");
        xml.addChild("outputs");

        xml.setTo("outputs");

        for (std::map<std::string, Output>::const_iterator it = outputs.begin(); it != outputs.end(); ++it) {
            ofXml output_xml;
            output_xml.addChild("output");
            output_xml.setTo("output");
            output_xml.setAttribute("id", it->first);
            output_xml.serialize(*it->second.parameters);
            xml.addXml(output_xml);
        }
        xml.setToParent();
        xml.setToParent();

        xml.save("settings.xml");

        client_parameters_changed = false;
    }
    for (std::map<std::string, Output>::iterator it = outputs.begin(); it != outputs.end(); ++it) {
        it->second.update(image_dimensions.x, image_dimensions.y);
    }
    player.update();
}

void ofApp::doClientOSCEvent(const std::string& local_address, const ofxOscMessage& message, bool& frame_was_updated, bool& missed_frames_need_checked, bool& outputs_were_reconfigured)
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
        client_parameters["show_stats"].cast<bool>() = getMessageInteger(message, 0);
    }
    else if (local_address == "/full_screen" && message.getNumArgs() == 1)
    {
        bool full_screen = getMessageInteger(message, 0);
        client_parameters["full_screen"].cast<bool>() = full_screen;
        ofSetFullscreen(full_screen);
    }
    else if (local_address == "/source" && message.getNumArgs() == 1)
    {
        client_parameters["source"].cast<string>() = message.getArgAsString(0);
        source_changed = true;
    }
    else if (local_address == "/add_output")
    {
        std::string name;
        if (message.getNumArgs() == 1)
        {
            name = message.getArgAsString(0);
        }
        else
        {
            name = ofToString(outputs.size() + 1);
        }
        if (outputs.count(name) == 0)
        {
            outputs.insert(std::pair<std::string, Output>(name, Output(name)));
            outputs_were_reconfigured = true;
        }
    }
    else if (local_address == "/delete_output" && message.getNumArgs() == 1)
    {
        std::string name = message.getArgAsString(0);
        if (outputs.count(name) == 1)
        {
            outputs.erase(name);
            outputs_were_reconfigured = true;
        }
    }
    else if (local_address == "/send_config" && message.getNumArgs() == 1)
    {
        std::string destination = message.getArgAsString(0);
        std::vector<std::string> parts = ofSplitString(destination, ":");
        if (parts.size() == 2)
        {
            ofxOscSender sender;
            sender.setup(parts[0], ofToInt(parts[1]));
            ofxOscMessage message;
            message.setAddress("/client/xml");
            ofXml xml;
            xml.addChild("config");
            xml.setToChild(0);
            xml.addChild("id");
            xml.setValue("id", client_parameters["client_id"].toString());
            xml.addChild("extent");
            xml.setTo("extent");
            xml.addChild("width");
            xml.addChild("height");
            xml.setValue("width", ofToString(ofGetWidth()));
            xml.setValue("height", ofToString(ofGetHeight()));
            xml.setToParent();
            xml.addChild("source");
            xml.setValue("source", client_parameters["source"].toString());
            xml.addChild("outputs");
            xml.setTo("outputs");
            for (std::map<std::string, Output>::const_iterator it = outputs.begin(); it != outputs.end(); ++it) {
                ofXml output_xml;
                output_xml.addChild("output");
                output_xml.setTo("output");
                output_xml.setAttribute("id", it->first);
                output_xml.addChild("name");
                output_xml.setValue("name", it->first);
                xml.addXml(output_xml);
            }
            xml.setToParent();
            xml.setToParent();
            message.addStringArg(xml.toString());
            sender.sendMessage(message);
        }

    }
    else
    {
        /*
         For any other address, forward it to every output
         */
        for (std::map<std::string, Output>::iterator it = outputs.begin(); it != outputs.end(); ++it) {
            it->second.doOSCEvent(local_address, message, frame_was_updated, missed_frames_need_checked);
        }
    }
}

void ofApp::Output::doOSCEvent(const std::string &local_address, const ofxOscMessage& message, bool &frame_was_updated, bool &missed_frames_need_checked)
{
    if (local_address == "/crop/active" && message.getNumArgs() == 1)
    {
        parameters->get("crop_active").cast<bool>() = getMessageInteger(message, 0);
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
        parameters->get("crop_width").cast<float>() = getMessageFloat(message, 0);
    }
    else if (local_address == "/crop/height" && message.getNumArgs() == 1)
    {
        parameters->get("crop_height").cast<float>() = getMessageFloat(message, 0);
    }
    else if (local_address == "/warp/top_left/x" && message.getNumArgs() == 1)
    {
        updatePointParameter("warp_top_left", 0, getMessageFloat(message, 0));
    }
    else if (local_address == "/warp/top_left/y" && message.getNumArgs() == 1)
    {
        updatePointParameter("warp_top_left", 1, getMessageFloat(message, 0));
    }
    else if (local_address == "/warp/top_right/x" && message.getNumArgs() == 1)
    {
        updatePointParameter("warp_top_right", 0, getMessageFloat(message, 0));
    }
    else if (local_address == "/warp/top_right/y" && message.getNumArgs() == 1)
    {
        updatePointParameter("warp_top_right", 1, getMessageFloat(message, 0));
    }
    else if (local_address == "/warp/bottom_right/x" && message.getNumArgs() == 1)
    {
        updatePointParameter("warp_bottom_right", 0, getMessageFloat(message, 0));
    }
    else if (local_address == "/warp/bottom_right/y" && message.getNumArgs() == 1)
    {
        updatePointParameter("warp_bottom_right", 1, getMessageFloat(message, 0));
    }
    else if (local_address == "/warp/bottom_left/x" && message.getNumArgs() == 1)
    {
        updatePointParameter("warp_bottom_left", 0, getMessageFloat(message, 0));
    }
    else if (local_address == "/warp/bottom_left/y" && message.getNumArgs() == 1)
    {
        updatePointParameter("warp_bottom_left", 1, getMessageFloat(message, 0));
    }
    else if (local_address == "/blend/top" && message.getNumArgs() == 1)
    {
        parameters->get("blend_top").cast<float>() = getMessageFloat(message, 0);
    }
    else if (local_address == "/blend/right" && message.getNumArgs() == 1)
    {
        parameters->get("blend_right").cast<float>() = getMessageFloat(message, 0);
    }
    else if (local_address == "/blend/bottom" && message.getNumArgs() == 1)
    {
        parameters->get("blend_bottom").cast<float>() = getMessageFloat(message, 0);
    }
    else if (local_address == "/blend/left" && message.getNumArgs() == 1)
    {
        parameters->get("blend_left").cast<float>() = getMessageFloat(message, 0);
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    bool show_stats = client_parameters.getBool("show_stats");
    ofTexture *texture = NULL;
    if (use_sequence)
    {
        texture = &image.getTextureReference();
    }
    else if (player.isLoaded())
    {
        texture = player.getTexture();
    }

    if (texture != NULL)
    {
        for (std::map<std::string, Output>::iterator it = outputs.begin(); it != outputs.end(); ++it) {
            it->second.draw(*texture, show_stats);
        }
    }

    if (show_stats)
    {
        ofPushStyle();
        if (in_error)
        {
            ofSetColor(200, 0, 0);
        }
        else
        {
            ofSetColor(0, 200, 0);
        }
        ofRect(10, 14, 20, 4);

        ofPopStyle();

        std::vector<std::string> messages;
        std::string client_id = client_parameters.getString("client_id");
        messages.push_back("Client ID: " + client_id + " " + ofToString(ofGetFrameRate(), 0) + " FPS");

        if (use_sequence == false && !player.isLoaded())
        {
            std::string source_path = client_parameters.getString("source");
            messages.push_back("Frame source not loaded: " + source_path);
        }
        if (in_error)
        {
            messages.push_back("Frame discontinuities: " + frame_number_errors);
        }

        float y_offset = 40;
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
    for (std::map<std::string, Output>::iterator it = outputs.begin(); it != outputs.end(); ++it) {
        it->second.dimensions_changed = true;
    }
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

void ofApp::parameterChanged(ofAbstractParameter & parameter)
{
    client_parameters_changed = true;
}

ofApp::Output::Output(std::string n) :
name(n), dimensions_changed(true), bounding_box(ofRectangle()), crop_box(ofRectangle()), parameters(new ofParameterGroup()), parameters_changed(false)
{
    parameters->setName("output_settings");
    ofParameter<std::string> name_param("name", name);
    parameters->add(name_param);
    ofParameter<bool> crop_active_param("crop_active", false);
    parameters->add(crop_active_param);
    ofParameter<ofPoint> crop_origin_param("crop_origin", ofPoint(0.0, 0.0));
    parameters->add(crop_origin_param);
    ofParameter<float> crop_width_param("crop_width", 0);
    parameters->add(crop_width_param);
    ofParameter<float> crop_height_param("crop_height", 0);
    parameters->add(crop_height_param);
    ofParameter<ofPoint> warp_top_left_param("warp_top_left", ofPoint(0.0, 0.0));
    parameters->add(warp_top_left_param);
    ofParameter<ofPoint> warp_top_right_param("warp_top_right", ofPoint(0.0, 0.0));
    parameters->add(warp_top_right_param);
    ofParameter<ofPoint> warp_bottom_right_param("warp_bottom_right", ofPoint(0.0, 0.0));
    parameters->add(warp_bottom_right_param);
    ofParameter<ofPoint> warp_bottom_left_param("warp_bottom_left", ofPoint(0.0, 0.0));
    parameters->add(warp_bottom_left_param);
    ofParameter<float> blend_top_param("blend_top", 0.0);
    parameters->add(blend_top_param);
    ofParameter<float> blend_right_param("blend_right", 0.0);
    parameters->add(blend_right_param);
    ofParameter<float> blend_bottom_param("blend_bottom", 0.0);
    parameters->add(blend_bottom_param);
    ofParameter<float> blend_left_param("blend_left", 0.0);
    parameters->add(blend_left_param);

    /*
     Monitor any subsequent settings changes so we can save them
     */
    ofAddListener(parameters->parameterChangedE, this, &ofApp::Output::parameterChanged);
}

ofApp::Output::~Output()
{
    ofRemoveListener(parameters->parameterChangedE, this, &ofApp::Output::parameterChanged);
}

ofApp::Output& ofApp::Output::operator = (const ofApp::Output& b)
{
    ofRemoveListener(parameters->parameterChangedE, this, &ofApp::Output::parameterChanged);
    parameters = b.parameters;
    name = b.name;

    dimensions_changed = b.dimensions_changed;
    parameters_changed = b.parameters_changed;
    bounding_box = b.bounding_box;
    crop_box = b.crop_box;

    blends = b.blends;
    warper = b.warper;

    ofAddListener(parameters->parameterChangedE, this, &ofApp::Output::parameterChanged);
    return *this;
}

ofApp::Output::Output(const ofApp::Output& p)
: parameters(p.parameters),
name(p.name),
dimensions_changed(p.dimensions_changed),
parameters_changed(p.parameters_changed),
bounding_box(p.bounding_box),
crop_box(p.crop_box),
blends(p.blends),
warper(p.warper)
{
    ofAddListener(parameters->parameterChangedE, this, &ofApp::Output::parameterChanged);
}

void ofApp::Output::parameterChanged(ofAbstractParameter &parameter)
{
    parameters_changed = true;
    dimensions_changed = true;
}

void ofApp::Output::updatePointParameter(std::string name, int index, float value)
{
    ofPoint current = parameters->get(name).cast<ofPoint>();
    current[index] = value;
    parameters->get(name).cast<ofPoint>() = current;
}

void ofApp::Output::update(float image_width, float image_height)
{
    if (dimensions_changed)
    {
        /*
         Update the warper
         */
        if (parameters->getBool("crop_active"))
        {
            crop_box.position = parameters->getPoint("crop_origin");
            crop_box.width = parameters->getFloat("crop_width");
            crop_box.height = parameters->getFloat("crop_height");
            // Flip for OF's coords
            crop_box.y = image_height - crop_box.y - crop_box.height;
            crop_box = crop_box.getIntersection(ofRectangle(0, 0, image_width, image_height));
        }
        else
        {
            crop_box.position = ofPoint(0);
            crop_box.width = image_width;
            crop_box.height = image_height;
        }
        bounding_box = crop_box;
        bounding_box.scaleTo(ofGetWindowRect());

        warper.setup(bounding_box.x, bounding_box.y, bounding_box.width, bounding_box.height);
        // Multiply by 1,-1 to invert for OF's coords
        warper.setCorner(ofxGLWarper::TOP_LEFT, ofPoint(1, -1) * parameters->getPoint("warp_top_left").get() + warper.getCorner(ofxGLWarper::TOP_LEFT));
        warper.setCorner(ofxGLWarper::TOP_RIGHT, ofPoint(1, -1) * parameters->getPoint("warp_top_right").get() + warper.getCorner(ofxGLWarper::TOP_RIGHT));
        warper.setCorner(ofxGLWarper::BOTTOM_LEFT, ofPoint(1, -1) * parameters->getPoint("warp_bottom_left").get() + warper.getCorner(ofxGLWarper::BOTTOM_LEFT));
        warper.setCorner(ofxGLWarper::BOTTOM_RIGHT, ofPoint(1, -1) * parameters->getPoint("warp_bottom_right").get() + warper.getCorner(ofxGLWarper::BOTTOM_RIGHT));

        /*
         Update blend meshes
         */
        blends.clear();
        if (parameters->getFloat("blend_top") > 0.0)
        {
            ofPoint adjustment(0.0, parameters->getFloat("blend_top"));
            ofMesh mesh;
            mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
            mesh.addVertex(bounding_box.position);
            mesh.addColor(ofColor::black);
            mesh.addVertex(bounding_box.getTopRight());
            mesh.addColor(ofColor::black);
            mesh.addVertex(bounding_box.getTopLeft() + adjustment);
            mesh.addColor(ofColor(0.0, 0.0));
            mesh.addVertex(bounding_box.getTopRight() + adjustment);
            mesh.addColor(ofColor(0.0, 0.0));
            blends.push_back(mesh);
        }
        if (parameters->getFloat("blend_right") > 0.0)
        {
            ofPoint adjustment(parameters->getFloat("blend_right"), 0.0);
            ofMesh mesh;
            mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
            mesh.addVertex(bounding_box.getTopRight() - adjustment);
            mesh.addColor(ofColor(0.0, 0.0));
            mesh.addVertex(bounding_box.getTopRight());
            mesh.addColor(ofColor::black);
            mesh.addVertex(bounding_box.getBottomRight() - adjustment);
            mesh.addColor(ofColor(0.0, 0.0));
            mesh.addVertex(bounding_box.getBottomRight());
            mesh.addColor(ofColor::black);
            blends.push_back(mesh);
        }
        if (parameters->getFloat("blend_bottom") > 0.0)
        {
            ofPoint adjustment(0.0, parameters->getFloat("blend_bottom"));
            ofMesh mesh;
            mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
            mesh.addVertex(bounding_box.getBottomLeft() - adjustment);
            mesh.addColor(ofColor(0.0, 0.0));
            mesh.addVertex(bounding_box.getBottomRight() - adjustment);
            mesh.addColor(ofColor(0.0, 0.0));
            mesh.addVertex(bounding_box.getBottomLeft());
            mesh.addColor(ofColor::black);
            mesh.addVertex(bounding_box.getBottomRight());
            mesh.addColor(ofColor::black);
            blends.push_back(mesh);
        }
        if (parameters->getFloat("blend_left") > 0.0)
        {
            ofPoint adjustment(parameters->getFloat("blend_left"), 0.0);
            ofMesh mesh;
            mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
            mesh.addVertex(bounding_box.position);
            mesh.addColor(ofColor::black);
            mesh.addVertex(bounding_box.getTopLeft() + adjustment);
            mesh.addColor(ofColor(0.0, 0.0));
            mesh.addVertex(bounding_box.getBottomLeft());
            mesh.addColor(ofColor::black);
            mesh.addVertex(bounding_box.getBottomLeft() + adjustment);
            mesh.addColor(ofColor(0.0, 0.0));
            blends.push_back(mesh);
        }
        dimensions_changed = false;
    }
}

void ofApp::Output::draw(ofTexture &texture, bool show_stats)
{
    warper.begin();

    texture.drawSubsection(bounding_box.x, bounding_box.y, bounding_box.width, bounding_box.height, crop_box.x, crop_box.y, crop_box.width, crop_box.height);

    for (std::vector<ofMesh>::iterator it = blends.begin(); it != blends.end(); ++it)
    {
        it->draw();
    }

    if (show_stats)
    {
        ofPushStyle();
        ofSetColor(128, 128, 128, 128);
        ofSetLineWidth(1.0);
        ofFill();
        ofRect(bounding_box);
        ofPopStyle();

        ofPoint draw_point = bounding_box.getCenter() - ofPoint(30, 200);
        ofDrawBitmapString("Output: " + name, draw_point);
    }

    warper.end();
}