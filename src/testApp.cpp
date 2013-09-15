#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	ofBackground(0);
	ofSetVerticalSync(true);
	ofEnablePointSprites();
	ofClear(0);
	
	fireParticles.setup(ofVec2f(ofGetWidth(), ofGetHeight()));
	bgColor = mouseColor = ofColor(0);
}

void testApp::exit() {
	fireParticles.shutdown();
}

//--------------------------------------------------------------
void testApp::update(){
	impulseIndex += 0.1;
	
	if(ofNoise(impulseIndex) > 0.68) {
		fireParticles.addImpulse();
	}
	
	// a bit of background flicker
	float flickerAmount = ofNoise(impulseIndex) * 7. * intensity.y;
	bgColor.r += flickerAmount;
	bgColor.g += flickerAmount * 0.5;
	
	bgColor.lerp(mouseColor, 0.15);
}

//--------------------------------------------------------------
void testApp::draw(){
	ofBackground(bgColor);
	
	ofSetColor(255);
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	fireParticles.draw();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	
	intensity = ofVec2f(ofMap(x, 0, ofGetWidth(), -1, 1, true),
						ofMap(y, 0, ofGetHeight(), 1, 0.1, true));
	
	mouseColor = ofColor(ofMap(intensity.y, 0.1, 1, 0,  50, true),
						 ofMap(intensity.y, 0.1, 1, 10, 30, true),
						 ofMap(intensity.y, 0.1, 1, 40,  0, true));
	
	fireParticles.setIntensity(intensity);
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
	fireParticles.setWindowSize(ofVec2f(ofGetWidth(), ofGetHeight()));
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}
