#include "ofMain.h"
#include "testApp.h"

//========================================================================
int main( ){
	ofSetupOpenGL(1440,900,OF_FULLSCREEN);
	ofRunApp(new testApp());

}
