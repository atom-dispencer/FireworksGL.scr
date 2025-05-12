#pragma once

enum ParticleType { PT_SPARK = 0, PT_SPARK_ROCKET = 1, PT_HAZE = 2 };

struct Particle {
  float position[3];
  float velocity[3];
  float acceleration[3];
  enum ParticleType type;
  int isAlive;
  float remainingLife;
  float radius;
  float colour[4];
  int children;
  float timeSinceLastEmission;
  int rocketIsPinwheel;
  float hazeDragFactor;
};

struct FWGLSimulation {
  int maxParticles;
  int liveParticles;
  int maxRockets;
  int liveRockets;
  struct Particle *particles;
  float timeSinceRocketCount;
};

void RandomBrightColour(float rgba[4]);
int RandIntRange(int lower, int upper);
double RandDouble();
void DistributeSpeeds(float *speeds, float *velocities, int speedCount);
void MoveParticles(struct FWGLSimulation *simulation, int width, int height,
                   float dSecs);
void DeleteParticle(struct FWGLSimulation *simulation, int particle);

void MakePTSpark(struct FWGLSimulation *simulation, int particle);
void MakePTSparkRocket(struct FWGLSimulation *simulation, int particle);
void MakePTHaze(struct FWGLSimulation *simulation, int particle);

void ProcessPTSpark(struct FWGLSimulation *simulation, int particle,
                    float dSecs);
void ProcessPTSparkRocket(struct FWGLSimulation *simulation, int particle,
                          float dSecs);
void ProcessPTHaze(struct FWGLSimulation *simulation, int particle,
                   float dSecs);

void KillPTSpark(struct FWGLSimulation *simulation, int particle);
void KillPTSparkRocket(struct FWGLSimulation *simulation, int particle);
void KillPTHaze(struct FWGLSimulation *simulation, int particle);
