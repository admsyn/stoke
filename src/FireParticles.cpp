//
//  FireParticles.cpp
//  stoke
//
//  Created by Adam on 2013-09-14.
//

#include "FireParticles.h"
#include "ofImage.h"
#include <Accelerate/Accelerate.h>

//const size_t MAX_PARTICLES = (8192 * 32);
const float FADE_SPEED = 0.95;
const float IMPULSE_AMOUNT = 20;

// these pairs are (min, max)
const pair<float, float> P_SIZE(4, 13); // particle size
//const pair<size_t, size_t> P_RATE(128, 2200); // particle addition rate
const pair<float, float> DAMP_RATE(0.90, 0.97); // velocity dampening (vel *= damp every update)

dispatch_queue_t FireParticles::_particleQueue() {
	static dispatch_once_t onceToken;
	static dispatch_queue_t queue = NULL;
	
	dispatch_once(&onceToken, ^{
		queue = dispatch_queue_create("stoke.particles", DISPATCH_QUEUE_SERIAL);
	});
	
	return queue;
}

FireParticles::FireParticles()
: _isRunning(false)
, _doImpulse(false)
, _particleIntensity(0, 0.5)
, _particleNoiseIndex(0) {
	const size_t initReserve = 16384;
	_verts.reserve(initReserve);
	_vels.reserve(initReserve);
	_colors.reserve(initReserve);
	_dampens.reserve(initReserve);
}

FireParticles::~FireParticles() {
	shutdown();
}

void FireParticles::setup(float particlePercentage) {
	
	_rect = ofRectangle();
	
	MAX_PARTICLES = (8192 * 32) * particlePercentage;
	P_RATE = pair<size_t, size_t>(128 * particlePercentage, 1600 * particlePercentage);//2200 * particlePercentage);
	
	// load texture and shader
	ofDisableArbTex();
	ofLoadImage(_particleTex, "dot.png");
	_particleShader.load("shaders/fire-shader");
	
	// fill vbo with dummy values
	vector<ofVec2f> blankVerts = vector<ofVec2f>(MAX_PARTICLES);
	vector<ofFloatColor> blankColors = vector<ofFloatColor>(MAX_PARTICLES, ofFloatColor(1,1,1,1));
	_particleVbo.setVertexData(&blankVerts.front(), MAX_PARTICLES, GL_DYNAMIC_DRAW);
	_particleVbo.setColorData(&blankColors.front(), MAX_PARTICLES, GL_DYNAMIC_DRAW);
	
	// set random point size attribute data
	_particleShader.begin();
	{
		vector<float> randomPointSizes;
		
		for(size_t i = 0; i < MAX_PARTICLES; i++) {
			randomPointSizes.push_back(ofRandom(P_SIZE.first, P_SIZE.second));
		}
		
		int pointSizeAttribute = _particleShader.getAttributeLocation("pointSize");
		_particleVbo.setAttributeData(pointSizeAttribute, &randomPointSizes.front(), 1, MAX_PARTICLES, GL_STATIC_DRAW);
	}
	_particleShader.end();
	
	// start update timer
	_particleTimer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, _particleQueue());
	
	if(_particleTimer) {
		const dispatch_time_t interval = 30 * NSEC_PER_MSEC;
		const dispatch_time_t leeway = 7 * NSEC_PER_MSEC;
		dispatch_source_set_timer(_particleTimer, dispatch_walltime(NULL, 0), interval, leeway);
		dispatch_source_set_event_handler(_particleTimer, ^{update();});
		dispatch_resume(_particleTimer);
		_isRunning = true;
	} else {
		ofLogFatalError("FireParticles") << "couldn't start a timer";
	}
}

void FireParticles::shutdown() {
	if(_isRunning) {
		// wait for any updating to finish before returning
		// this prevents a possible crash on exit
		dispatch_sync(_particleQueue(), ^{dispatch_suspend(_particleTimer);});
		_isRunning = false;
	}
}

#pragma mark - Utils

size_t UpdateVbo(ofVbo &vbo, const vector<ofVec2f> &verts, const vector<ofFloatColor> &colors) {
	vbo.updateVertexData(&verts[0], verts.size());
	vbo.updateColorData(&colors[0], colors.size());
	return MIN(verts.size(), colors.size());
}

void DrawVbo(ofVbo &vbo, int drawMode, size_t vertsToDraw, size_t vboMaxSize, size_t begin = 0)
{
	vertsToDraw = MIN(vertsToDraw, vboMaxSize);
	
	if(vertsToDraw) {
		vbo.draw(drawMode, begin, vertsToDraw);
	}
}

#pragma mark - Update / Draw

void FireParticles::update() {
	_particleNoiseIndex += 0.1;
	
	size_t particlesToAdd = P_RATE.first + (P_RATE.second * _particleIntensity.y);
	particlesToAdd *= ofNoise(_particleNoiseIndex);
	
	addParticles(particlesToAdd);
	updateParticleData();
	removeDeadParticles();
}

void FireParticles::draw() {
	__block size_t vertsToDraw = 0;
	
	dispatch_sync(_particleQueue(), ^{
		vertsToDraw = UpdateVbo(_particleVbo, _verts, _colors);
	});
	
	_particleShader.begin();
	_particleTex.bind();
	{
		DrawVbo(_particleVbo, GL_POINTS, vertsToDraw, MAX_PARTICLES);
	}
	_particleTex.unbind();
	_particleShader.end();
}

#pragma mark - Particle Lifecycle

void FireParticles::addParticles(size_t particlesToAdd) {
	
	float colorNoise = ofNoise(_particleNoiseIndex);
	float impulse = ofNoise(_particleNoiseIndex) * IMPULSE_AMOUNT * _particleIntensity.y;
	
	for(int i = 0; i < particlesToAdd; i++) {
		float verticalVelocity = ofRandom(7  * _particleIntensity.y,
										  45 * _particleIntensity.y);
		
		ofFloatColor particleColor(ofRandom(0.7, 1.0),
								   ofRandom(0.1, 0.1 + colorNoise * 0.3),
								   ofRandom(0.2 - (colorNoise * 0.1)),
								   ofRandom(_particleIntensity.y * 0.6));
		
		// apply the impulse to roughly 1/2 of the particles
		if(_doImpulse && (rand() % 2)) {
			verticalVelocity += ofRandom(impulse);
			particleColor.r =  1.0;
			particleColor.g += 0.3;
			particleColor.b += 0.15;
		}
		
		_verts.push_back(ofVec2f(ofRandom(_rect.width) + _rect.x,
								 ofRandom(_rect.height, _rect.height + 35.)));
		
		_vels.push_back(ofVec2f(_particleIntensity.x * 3. + ofRandomf() * 3, -verticalVelocity));
		_colors.push_back(particleColor);
		_dampens.push_back(ofVec2f(0.9, ofRandom(DAMP_RATE.first, DAMP_RATE.second)));
	}
	
	_doImpulse = false;
}

void FireParticles::updateParticleData() {
	const size_t count = MIN(_verts.size(), _vels.size()) * 2;
	
	float * verts = (float *)&_verts[0];
	float * vels = (float *)&_vels[0];
	float * cols = (float *)&_colors[0];
	float * damps = (float *)&_dampens[0];
	
	// add the velocities to the vertices
	vDSP_vadd(verts, 1, vels, 1, verts, 1, count);
	
	// dampen the velocities
	vDSP_vmul(vels, 1, damps, 1, vels, 1, count);
	
	// fade out the colours
	vDSP_vsmul(cols, 1, &FADE_SPEED, cols, 1, _colors.size() * 4);
}

void FireParticles::removeDeadParticles() {
	size_t vertsToRemove = 0;
	for(size_t i = 0; i < _vels.size(); ++i) {
		if(_vels[i].lengthSquared() > 16) {
			break;
		} else {
			++vertsToRemove;
		}
	}
	
	if(vertsToRemove) {
		_verts.erase(_verts.begin(), _verts.begin() + vertsToRemove);
		_vels.erase(_vels.begin(), _vels.begin() + vertsToRemove);
		_colors.erase(_colors.begin(), _colors.begin() + vertsToRemove);
		_dampens.erase(_dampens.begin(), _dampens.begin() + vertsToRemove);
	}
}

void FireParticles::clearParticles() {
	_verts.clear();
	_vels.clear();
	_colors.clear();
	_dampens.clear();
}

#pragma mark - Setters

void FireParticles::setRect(ofRectangle rect) {
	dispatch_async(_particleQueue(), ^{
		clearParticles();
		_rect = rect;
	});
}

void FireParticles::setIntensity(ofVec2f intensity) {
	dispatch_async(_particleQueue(), ^{
		_particleIntensity = intensity;
	});
}

void FireParticles::addImpulse() {
	dispatch_async(_particleQueue(), ^{
		_doImpulse = true;
	});
}
