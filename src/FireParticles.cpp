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

FireParticles::FireParticles()
: _hasSetup(false)
, _particleVelocity(0, 0.3) {
	_verts.reserve(MAX_VERTS);
	_vels.reserve(MAX_VERTS);
	_colors.reserve(MAX_VERTS);
	
	vector<ofVec2f> blankVerts = vector<ofVec2f>(MAX_VERTS);
	vector<ofFloatColor> blankColors = vector<ofFloatColor>(MAX_VERTS, ofFloatColor(1,1,1,1));
	_particleVbo.setVertexData(&blankVerts.front(), MAX_VERTS, GL_DYNAMIC_DRAW);
	_particleVbo.setColorData(&blankColors.front(), MAX_VERTS, GL_DYNAMIC_DRAW);
	
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
	
	for(int i = 0; i < 256 + (1800 * _particleVelocity.y); i++) {
		_verts.push_back(ofVec2f(ofRandom(1280),
								 ofRandom(770, 790)));
		
		_vels.push_back(ofVec2f(_particleVelocity.x * 20. + ofRandomf() * 3,
								ofRandom(-5  * _particleVelocity.y,
										 -35 * _particleVelocity.y)));
		
		_colors.push_back(ofFloatColor(ofRandom(0.7, 1.0),
									   ofRandom(0.1, 0.3),
									   ofRandom(0.1),
									   ofRandom(_particleVelocity.y * 0.6)));
	}
	
	const size_t count = MIN(_verts.size(), _vels.size()) * 2;
	const float dampen = 0.94;
	
	float * verts = (float *)&_verts[0];
	float * vels  = (float *)&_vels[0];
	float * cols  = (float *)&_colors[0];
	vDSP_vadd(verts, 1, vels, 1, verts, 1, count);
	vDSP_vsmul(vels, 1, &dampen, vels, 1, count);
	vDSP_vsmul(cols, 1, &dampen, cols, 1, _colors.size() * 4);
	
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
	}
}

void FireParticles::setVelocity(ofVec2f velocity) {
	dispatch_sync(particleQueue, ^{
		_particleVelocity = velocity;
	});
}
