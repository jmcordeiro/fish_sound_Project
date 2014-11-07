

#include "particlesSqr.h"

//--------------------------------------------------------------
ParticlesSqr::ParticlesSqr()
{
    
    ofBackground(255, 0, 0);
    
    // 1,000,000 particles
    unsigned w = 1000;
    unsigned h = 1000;
    
    particles.init(w, h);
    
    // initial positions
    // use new to allocate 4,000,000 floats on the heap rather than
    // the stack
    float* particlePosns = new float[w * h * 4];
    for (unsigned y = 0; y < h; ++y)
    {
        for (unsigned x = 0; x < w; ++x)
        {
            unsigned idx = y * w + x;
            particlePosns[idx * 4] = 400.f * x / (float)w - 200.f; // particle x
            particlePosns[idx * 4 + 1] = 400.f * y / (float)h - 200.f; // particle y
            particlePosns[idx * 4 + 2] = 0.f; // particle z
            particlePosns[idx * 4 + 3] = 0.f; // dummy
        }
    }
    particles.loadDataTexture(ofxGpuParticles::POSITION, particlePosns);
    delete[] particlePosns;
    
    // initial velocities
    particles.zeroDataTexture(ofxGpuParticles::VELOCITY);
    
    // listen for update event to set additonal update uniforms
    ofAddListener(particles.updateEvent, this, &ParticlesSqr::onParticlesUpdate);
    
    cout << "TESTE" << endl;
    
}


ParticlesSqr::~ParticlesSqr(){
}


//--------------------------------------------------------------
void ParticlesSqr::updatePrtSqr()
{
    ofSetWindowTitle(ofToString(ofGetFrameRate(), 2));
    particles.update();
}

// set any update uniforms in this function
void ParticlesSqr::onParticlesUpdate(ofShader& shader)
{
    ofVec3f mouse(ofGetMouseX() - .5f * ofGetWidth(), .5f * ofGetHeight() - ofGetMouseY() , 0.f);
    shader.setUniform3fv("mouse", mouse.getPtr());
    shader.setUniform1f("elapsed", ofGetLastFrameTime());
    shader.setUniform1f("radiusSquared", 400.f * 200.f);
}

//--------------------------------------------------------------
void ParticlesSqr::drawPrtSqr()
{
    cam.begin();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    particles.draw();
    ofDisableBlendMode();
    cam.end();
}
