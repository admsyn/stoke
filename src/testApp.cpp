#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	ofBackground(0);
	ofSetVerticalSync(true);
	ofEnablePointSprites();
	ofClear(0);
	
	fire.setup();
}

//--------------------------------------------------------------
void testApp::update(){

}

//--------------------------------------------------------------
void testApp::draw(){
	glPointSize(12);
	glDepthMask(GL_FALSE);
	ofSetColor(255);
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	fire.draw();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	
	ofVec2f amount(ofMap(x, 0, ofGetWidth(), -1, 1, true),
				   ofMap(y, 0, ofGetHeight(), 1, 0.1, true));
	
	ofBackground(amount.y * 100,
				 ofMap(amount.y, 0, 1, 10, 50, true),
				 ofMap(amount.y, 0, 1, 40, 0, true));
	
	fire.setVelocity(amount);
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

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}
