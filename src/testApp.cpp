#include "testApp.h"

const size_t SPOOKY_VERTS = 5500;

enum AbletonControls {
	kSpookyVol = 1,
	kFireIntensity,
	kFireA,
	kFireB,
	kFireC,
	kFireD
};

float avg(const vector<float> &v) {
	float avg = 0;
	for(size_t i = 0; i < v.size(); ++i) {
		avg += v[i];
	}
	return avg / (float)v.size();
}

//--------------------------------------------------------------
void testApp::setup(){
	midiOut.openVirtualPort("Stoke MIDI");
	
	ofBackground(0);
	ofSetVerticalSync(true);
	ofEnablePointSprites();
	ofClear(0);
	
	fireParticles.resize(4);
	for(size_t i = 0; i < fireParticles.size(); ++i) {
		fireParticles[i].setup(1 / (float)fireParticles.size());
	}
	setupParticleRects();
	intensity.resize(fireParticles.size());
	
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
	for(size_t i = 0; i < fireParticles.size(); ++i) {
		fireParticles[i].shutdown();
	}
}

//--------------------------------------------------------------
void testApp::update(){
	impulseIndex += 0.1;
	
	size_t targetColumn = ofMap(intensityVector.x, -1, 1, 0, fireParticles.size());
	for(size_t i = 0; i < intensity.size(); ++i) {
		float columnTarget = i == targetColumn ? intensityVector.y : 0;
		
		if(intensity[i] < columnTarget) {
			intensity[i] = ofLerp(intensity[i], columnTarget, 0.5);
		} else {
			intensity[i] = ofLerp(intensity[i], columnTarget, 0.01);
		}
	}
	
	float totalIntensity = avg(intensity);
	
	mouseColor = ofColor(ofMap(totalIntensity, 0, 1, 0,  50, true),
						 ofMap(totalIntensity, 0, 1, 10, 30, true),
						 ofMap(totalIntensity, 0, 1, 50,  0, true));

	if(ofNoise(impulseIndex) > 0.68) {
		for(size_t i = 0; i < fireParticles.size(); ++i) {
			fireParticles[i].addImpulse();
		}
	}
	
	
	for(size_t i = 0; i < fireParticles.size(); ++i) {
		fireParticles[i].setIntensity(ofVec2f(ofSignedNoise(impulseIndex * 0.1, i),
											  intensity[i]));
	}
	
	if(showSpooky()) {
		for(size_t i = 0; i < spookyVerts.size(); ++i) {
			spookyVerts[i] += ofVec2f(ofSignedNoise(impulseIndex * 0.1, i * 0.2),
									  ofSignedNoise(impulseIndex * 0.1, i * 0.1)) * 0.1;
		}
		spookyVbo.updateVertexData(&spookyVerts[0], spookyVerts.size());
	}
	
	if(spookyVisibility > totalIntensity) {
		spookyVisibility = ofLerp(spookyVisibility, totalIntensity, 0.005);
	} else {
		spookyVisibility = totalIntensity;
	}
	
	// a bit of background flicker
	float flickerAmount = ofNoise(impulseIndex) * 7. * totalIntensity;
	bgColor.r += flickerAmount;
	bgColor.g += flickerAmount * 0.5;
	bgColor.lerp(mouseColor, 0.15);
	
	// update MIDI
	midiOut.sendControlChange(1, kFireIntensity, totalIntensity * 127);
	midiOut.sendControlChange(1, kFireA, intensity[0] * 127);
	midiOut.sendControlChange(1, kFireB, intensity[1] * 127);
	midiOut.sendControlChange(1, kFireC, intensity[2] * 127);
	midiOut.sendControlChange(1, kFireD, intensity[3] * 127);
	midiOut.sendControlChange(1, kSpookyVol, ofMap(spookyVisibility, 1.25, 0, 0, 127, true));
}

//--------------------------------------------------------------
void testApp::draw(){
	ofBackground(bgColor);
	
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	
	if(showSpooky()) {
		spookyShader.begin();
		spookyShader.setUniform1f("visibility", ofMap(spookyVisibility, 0.25, 0, 0, 0.7, true));
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
	for(size_t i = 0; i < fireParticles.size(); ++i) {
		fireParticles[i].draw();
	}
	
	ofEnableAlphaBlending();
	ofDrawBitmapStringHighlight(ofToString((int)ofGetFrameRate()), 10, 15);
}

bool testApp::showSpooky(){return avg(intensity) < 0.25;}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	intensityVector = ofVec2f(ofMap(x, 0, ofGetWidth(), -1, 1, true),
							  ofMap(y, 0, ofGetHeight(), 1, 0, true));
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

void testApp::setupParticleRects() {
	float gridWidth = ofGetWidth() / (float)fireParticles.size();
	for(size_t i = 0; i < fireParticles.size(); ++i) {
		ofRectangle rect(i * gridWidth, 0, gridWidth, ofGetHeight());
		fireParticles[i].setRect(rect);
	}
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
	setupParticleRects();
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}
