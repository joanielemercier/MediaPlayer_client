#include "ofMain.h"
#include "ofApp.h"
#include "ofAppGLFWWindow.h"

//========================================================================
int main( ){
    /*
    Cause OF to go fullscreen over all displays
    http://forum.openframeworks.cc/t/fullscreen-dual-screen/693/26
    */
    ofAppGLFWWindow window;
    window.setMultiDisplayFullscreen(true);
	ofSetupOpenGL(&window, 1024,768,OF_WINDOW);

	ofRunApp(new ofApp());

}
