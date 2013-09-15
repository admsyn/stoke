#pragma once

#include "ofVbo.h"
#include "ofVec2f.h"
#include "ofImage.h"
#include "ofShader.h"
#include <vector>

class FireParticles {
public:
	
	FireParticles();
	~FireParticles();
	
	void setup();
	void draw();
	void setVelocity(ofVec2f velocity);

protected:
	
	dispatch_queue_t particleQueue;
	
	void update();
	void addParticles(size_t particlesToAdd);
	void removeDeadParticles();
	void updatePositions();
	
	dispatch_source_t _particleTimer;
	
	ofVbo _particleVbo;
	
	vector<ofVec2f> _verts;
	vector<ofVec2f> _vels;
	vector<ofFloatColor> _colors;
	vector<ofVec2f> _dampens;

	ofTexture _particleTex;
	ofShader _particleShader;
	
	ofVec2f _particleVelocity;
	float _particleNoiseIndex;
	
	bool _hasSetup;
};
