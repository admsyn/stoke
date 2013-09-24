#include "testApp.h"

const size_t SPOOKY_VERTS = 5500;

//--------------------------------------------------------------
void testApp::setup(){
	midiOut.openVirtualPort("Stoke MIDI");
	
	ofBackground(0);
	ofSetVerticalSync(true);
	ofEnablePointSprites();
	ofClear(0);
	
	fireParticles.setup(ofVec2f(ofGetWidth(), ofGetHeight()));
	bgColor = mouseColor = ofColor(0);
	
	for(int i = 0; i < SPOOKY_VERTS; i++) {
		spookyVerts.push_back(ofVec2f(ofRandomWidth(), ofRandomHeight()));
	}
	
	spookyVbo.setVertexData(&spookyVerts.front(), SPOOKY_VERTS, GL_DYNAMIC_DRAW);
	
	ofDisableArbTex();
	ofLoadImage(spookyTex, "dot.png");
	spookyShader.load("shaders/spooky-shader");
	
	spookyShader.begin();
	{
		vector<float> randomPointSizes;
		
		for(size_t i = 0; i < spookyVerts.size(); i++) {
			randomPointSizes.push_back(ofRandom(2, 13));
		}
		
		int pointSizeAttribute = spookyShader.getAttributeLocation("pointSize");
		spookyVbo.setAttributeData(pointSizeAttribute, &randomPointSizes.front(), 1, SPOOKY_VERTS, GL_STATIC_DRAW);
	}
	spookyShader.end();
	spookyVisibility = 0;
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
	
	if(showSpooky()) {
		for(size_t i = 0; i < spookyVerts.size(); ++i) {
			spookyVerts[i] += ofVec2f(ofSignedNoise(impulseIndex * 0.1, i * 0.2),
									  ofSignedNoise(impulseIndex * 0.1, i * 0.1)) * 0.1;
		}
		spookyVbo.updateVertexData(&spookyVerts[0], spookyVerts.size());
	}
	
	if(spookyVisibility > intensity.y) {
		spookyVisibility = ofLerp(spookyVisibility, intensity.y, 0.005);
	} else {
		spookyVisibility = intensity.y;
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
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	
	if(showSpooky()) {
		spookyShader.begin();
//		spookyShader.setUniform1f("visibility", ofMap(intensity.y, 0.4, 0.1, 0, 0.7, true));
		spookyShader.setUniform1f("visibility", ofMap(spookyVisibility, 0.4, 0.1, 0, 0.7, true));
		spookyShader.setUniform1f("colorMod", ofNoise(impulseIndex * 0.1) * 0.5);
		spookyTex.bind();
		{
			spookyVbo.draw(GL_POINTS, 0, SPOOKY_VERTS);
		}
		spookyTex.unbind();
		spookyShader.end();
	}
	
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofSetColor(255);
	fireParticles.draw();
	
	ofEnableAlphaBlending();
	ofDrawBitmapStringHighlight(ofToString((int)ofGetFrameRate()), 10, 15);
}

bool testApp::showSpooky(){return intensity.y < 0.45;}

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
						 ofMap(intensity.y, 0.1, 1, 50,  0, true));
	
	fireParticles.setIntensity(intensity);
	midiOut.sendControlChange(1, 1, ofMap(y, 0, ofGetHeight(), 0, 127, true));
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
