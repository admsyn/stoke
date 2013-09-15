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
	
	void setup(ofVec2f windowSize);
	void draw();
	void shutdown();
	
	void setIntensity(ofVec2f intensity);
	void setWindowSize(ofVec2f windowSize);
	void addImpulse();

private:
	
	dispatch_queue_t _particleQueue;
	
	void update();
	void addParticles(size_t particlesToAdd);
	void updateParticleData();
	void removeDeadParticles();
	void clearParticles();
	
	dispatch_source_t _particleTimer;
	
	ofVbo _particleVbo;
	
	vector<ofVec2f> _verts;
	vector<ofVec2f> _vels;
	vector<ofFloatColor> _colors;
	vector<ofVec2f> _dampens;

	ofTexture _particleTex;
	ofShader _particleShader;
	
	ofVec2f _windowSize;
	ofVec2f _particleIntensity;
	float _particleNoiseIndex;
	
	bool _isRunning;
	bool _doImpulse;
};
