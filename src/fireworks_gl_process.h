#pragma once

enum ParticleType {
    PT_SPARK            = 0,
    PT_SPARK_ROCKET     = 1,
    PT_HAZE             = 2
};

struct Particle {
    float position[3];
    float velocity[3];
    float acceleration[3];
    enum ParticleType type;
    uint8_t isAlive;
    float remainingLife;
    int radius;
    float colour[4];
    int children;
    float timeSinceLastEmission;
};

struct FWGLSimulation {
    int maxParticles;
    int liveParticles;
    int maxRockets;
    int liveRockets;
    struct Particle** particles;
    float timeSinceRocketCount;
};

float* RandomBrightColour();
int RandIntRange(int lower, int upper);
void MoveParticles(struct FWGLSimulation* simulation, int width, int height, double dSecs);
void DeleteParticle(struct FWGLSimulation* simulation, struct Particle* p);

void MakePTSpark(struct FWGLSimulation* simulation, int particle);
void MakePTSparkRocket(struct FWGLSimulation* simulation, int particle);
void MakePTHaze(struct FWGLSimulation* simulation, int particle);

void ProcessPTSpark(struct FWGLSimulation* simulation, int particle, float dSecs);
void ProcessPTSparkRocket(struct FWGLSimulation* simulation, int particle, float dSecs);
void ProcessPTHaze(struct FWGLSimulation* simulation, int particle, float dSecs);

void KillPTSpark(struct FWGLSimulation* simulation, int particle);
void KillPTSparkRocket(struct FWGLSimulation* simulation, int particle);
void KillPTHaze(struct FWGLSimulation* simulation, int particle);
