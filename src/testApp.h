#pragma once

#include "ofMain.h"
#include "ofxMidi.h"
#include "FireParticles.h"

class testApp : public ofBaseApp {
	
public:
	void setup();
	void update();
	void draw();
	void exit();
	
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	bool showSpooky();
	
	FireParticles fireParticles;
	ofxMidiOut midiOut;
	
	float impulseIndex;
	
	ofColor bgColor;
	ofColor mouseColor;
	ofVec2f intensity;
	
	ofVbo spookyVbo;
	ofShader spookyShader;
	ofTexture spookyTex;
	vector<ofVec2f> spookyVerts;
	float spookyVisibility;
};
