#pragma once

#include "ofVbo.h"
#include "ofTexture.h"
#include "ofVec2f.h"
#include "ofShader.h"
#include <vector>

class FireParticles {
public:
	
	FireParticles();
	~FireParticles();
	
	void setup();
	void draw();
	void shutdown();
	
	void setIntensity(ofVec2f intensity);
	void addImpulse();

private:
	
	dispatch_queue_t _particleQueue;
	
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
	
	ofVec2f _particleIntensity;
	float _particleNoiseIndex;
	
	bool _hasSetup;
	bool _doImpulse;
};
