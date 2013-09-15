//
//  FireParticles.cpp
//  stoke
//
//  Created by Adam on 2013-09-14.
//
//

#include "FireParticles.h"
#include <Accelerate/Accelerate.h>

const size_t MAX_VERTS = 8192 * 32;

const float MAX_PARTICLE_SIZE = 20;
const float MIN_PARTICLE_SIZE = 6;

const float MIN_DAMPEN = 0.91;
const float MAX_DAMPEN = 0.97;

const float FADE_SPEED = 0.95;

const float IMPULSE_AMOUNT = 20;

FireParticles::FireParticles()
: _hasSetup(false)
, _doImpulse(false)
, _particleVelocity(0, 0.5)
, _particleNoiseIndex(0) {
	_verts.reserve(MAX_VERTS);
	_vels.reserve(MAX_VERTS);
	_colors.reserve(MAX_VERTS);
	_dampens.reserve(MAX_VERTS);
	
	particleQueue = dispatch_queue_create("relay.particles", DISPATCH_QUEUE_SERIAL);
}

FireParticles::~FireParticles() {
	if(_hasSetup) {
		dispatch_suspend(_particleTimer);
		dispatch_release(particleQueue);
	}
}

void FireParticles::setup() {
	
	ofDisableArbTex();
	ofLoadImage(_particleTex, "dot.png");
	_particleShader.load("shaders/shader");
	
	vector<ofVec2f> blankVerts = vector<ofVec2f>(MAX_VERTS);
	vector<ofFloatColor> blankColors = vector<ofFloatColor>(MAX_VERTS, ofFloatColor(1,1,1,1));
	_particleVbo.setVertexData(&blankVerts.front(), MAX_VERTS, GL_DYNAMIC_DRAW);
	_particleVbo.setColorData(&blankColors.front(), MAX_VERTS, GL_DYNAMIC_DRAW);
	
	_particleShader.begin();
	{
		vector<float> blankPointSize;
		
		for(size_t i = 0; i < MAX_VERTS; i++) {
			blankPointSize.push_back(ofRandom(MIN_PARTICLE_SIZE, MAX_PARTICLE_SIZE));
		}
		
		int pointSizeAttribute = _particleShader.getAttributeLocation("pointSize");
		_particleVbo.setAttributeData(pointSizeAttribute, &blankPointSize.front(), 1, MAX_VERTS, GL_STATIC_DRAW);
	}
	_particleShader.end();
	
	_particleTimer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, particleQueue);
	
	if(_particleTimer) {
		const dispatch_time_t interval = 30 * NSEC_PER_MSEC;
		const dispatch_time_t leeway = 5 * NSEC_PER_MSEC;
		dispatch_source_set_timer(_particleTimer, dispatch_walltime(NULL, 0), interval, leeway);
		dispatch_source_set_event_handler(_particleTimer, ^{update();});
		dispatch_resume(_particleTimer);
		_hasSetup = true;
	} else {
		ofLogFatalError("FireParticles") << "couldn't start a timer what the fuck";
	}
}

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

void FireParticles::draw() {
	__block size_t vertsToDraw = 0;
	
	dispatch_sync(particleQueue, ^{
		vertsToDraw = UpdateVbo(_particleVbo, _verts, _colors);
	});
	
	_particleShader.begin();
	_particleTex.bind();
	{
		DrawVbo(_particleVbo, GL_POINTS, vertsToDraw, MAX_VERTS);
	}
	_particleTex.unbind();
	_particleShader.end();
}

void FireParticles::update() {
	_particleNoiseIndex += 0.1;
	addParticles((128 + (1800 * _particleVelocity.y)) * ofNoise(_particleNoiseIndex));
	updatePositions();
	removeDeadParticles();
}

#pragma mark - Particle Lifecycle

void FireParticles::addParticles(size_t particlesToAdd) {
	
	for(int i = 0; i < particlesToAdd; i++) {
		float verticalVelocity = ofRandom(5  * _particleVelocity.y,
										  40 * _particleVelocity.y);
		
		ofFloatColor particleColor(ofRandom(0.7, 1.0),
								   ofRandom(0.1, 0.3),
								   ofRandom(0.1),
								   ofRandom(_particleVelocity.y * 0.6));
		
		// apply the impulse to roughly 1/2 of the particles
		if(_doImpulse && !(rand() % 2)) {
			float impulse = ofNoise(_particleNoiseIndex) * IMPULSE_AMOUNT * _particleVelocity.y;
			verticalVelocity += ofRandom(impulse);
			particleColor.r  = 1.0;
			particleColor.g += 0.2;
			particleColor.b += 0.15;
		}
		
		_verts.push_back(ofVec2f(ofRandom(1280), ofRandom(770, 810)));
		_vels.push_back(ofVec2f(_particleVelocity.x * 20. + ofRandomf() * 3, -verticalVelocity));
		_colors.push_back(particleColor);
		_dampens.push_back(ofVec2f(0.9, ofRandom(MIN_DAMPEN, MAX_DAMPEN)));
	}
	
	_doImpulse = false;
}

void FireParticles::updatePositions() {
	const size_t count = MIN(_verts.size(), _vels.size()) * 2;
	
	float * verts = (float *)&_verts[0];
	float * vels = (float *)&_vels[0];
	float * cols = (float *)&_colors[0];
	float * damp = (float *)&_dampens[0];
	
	// add the velocities to the vertices
	vDSP_vadd(verts, 1, vels, 1, verts, 1, count);
	
	// dampen the velocities
	vDSP_vmul(vels, 1, damp, 1, vels, 1, count);
	
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

#pragma mark - Setters

void FireParticles::setVelocity(ofVec2f velocity) {
	dispatch_async(particleQueue, ^{
		_particleVelocity = velocity;
	});
}

void FireParticles::addImpulse() {
	dispatch_async(particleQueue, ^{
		_doImpulse = true;
	});
}
