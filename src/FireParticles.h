#pragma once

#include "ofVbo.h"
#include "ofVec2f.h"
#include "ofImage.h"
#include "ofShader.h"
#include <vector>

class FireParticles {
public:
	
	FireParticles();
	virtual ~FireParticles();
	
	virtual void setup();
	virtual void draw();
	virtual void setVelocity(ofVec2f velocity);

protected:
	
	dispatch_queue_t particleQueue;
	
	virtual void update();
	
	dispatch_source_t _particleTimer;
	
	ofVbo _particleVbo;
	
	vector<ofVec2f> _verts;
	vector<ofVec2f> _vels;
	vector<ofFloatColor> _colors;

	ofTexture _particleTex;
	ofShader _particleShader;
	ofVec2f _particleVelocity;
	
	bool _hasSetup;
};
