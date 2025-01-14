#pragma once

enum ParticleType {
    PT_SPARK,
    PT_SPARK_ROCKET,
    PT_HAZE
};

struct Particle {
    float position[3];
    float velocity[3];
    float acceleration[3];
    enum ParticleType type;
    uint8_t isAlive;
    float remainingLife;
    int radius;
    float color[4];
    int children;
    float timeSinceLastEmission;
};

float* RandomBrightColour();
int RandIntRange(int lower, int upper);
void MoveParticles(HWND hWnd);
void DeleteParticle(struct Particle* p);

void MakePTSpark(struct Particle* p);
void MakePTSparkRocket(struct Particle* p);
void MakePTHaze(struct Particle* p);

void ProcessPTSpark(struct Particle* p, float dSecs);
void ProcessPTSparkRocket(struct Particle* p, float dSecs);
void ProcessPTHaze(struct Particle* p, float dSecs);

void KillPTSpark(struct Particle* p);
void KillPTSparkRocket(struct Particle* p);
void KillPTHaze(struct Particle* p);
