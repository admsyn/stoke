#include "ofMain.h"
#include "testApp.h"

int winMode = OF_FULLSCREEN;
//int winMode = OF_WINDOW;

//========================================================================
int main( ){
	ofSetupOpenGL(1440,900,winMode);
	ofRunApp(new testApp());

}
