#include "fireworks_gl_process.h"
#include <stdlib.h>
#include <math.h>

int RandIntRange(int lower, int upper) {
    int r = rand();

    if (upper < lower) {
        int temp = lower;
        lower = upper;
        upper = temp;
    }

    int diff = upper - lower;
    int n = lower + (r % diff);
    return n;
}

double RandDouble() {
    return (double)rand() / (double)RAND_MAX;
}

void DeleteParticle(struct FWGLSimulation* simulation, int particle) {
    struct Particle* p = &(simulation->particles[particle]);
    
    if (p->type == PT_SPARK_ROCKET) {
        simulation->liveRockets--;
    }

    p->isAlive = 0;
    simulation->liveParticles--;
}

void RandomBrightColour(float rgba[4]) {
    int offset = RandIntRange(0, 3);

    rgba[0] = 0.0f;
    rgba[1] = 0.0f;
    rgba[2] = 0.0f;
    rgba[3] = 1.0f;

    for (int i = 0; i < 3; i++) {
        int index = (offset + i) % 3;

        switch (index) {
        case 0:
            rgba[i] = 0;
            break;
        case 1:
            rgba[i] = 1;
            break;
        case 2:
            rgba[i] = (float) RandDouble();
            break;
        }
    }
}

int ReviveDeadParticle(struct FWGLSimulation* simulation) {

    // First, look for dead particles
    for (int i = 0; i < simulation->maxParticles; i++) {
        struct Particle* c = &(simulation->particles[i]);
        if (!c->isAlive) {
            c->isAlive = 1;
            simulation->liveParticles++;
            return i;
        }
    }

    // Then, look for already alive haze
    for (int i = 0; i < simulation->maxParticles; i++) {
        struct Particle* c = &(simulation->particles[i]);
        if (c->type == PT_HAZE) {
            // Don't increment because we're just reassigning
            printf("No dead particles to revive, reallocating haze particle %d (you should increase FWGL_Init maxParticles)\n", i);
            return i;
        }
    }

    // I hope this never happens
    int x = RandIntRange(0, simulation->maxParticles);
    printf("Particle overflow! No dead and no haze, so reallocating whatever %d is!\n", x);
    return x;
}

void MakePTSparkRocket(struct FWGLSimulation* simulation, int particle) {
    struct Particle* p = &(simulation->particles[particle]);

    p->type = PT_SPARK_ROCKET;

    p->velocity[0] = (float) RandIntRange(-100, 100);
    p->velocity[1] = (float) RandIntRange(250, 400);
    p->velocity[2] = 0;
    p->acceleration[0] = 0;
    p->acceleration[1] = -100;
    p->acceleration[2] = 0;

    float colour[4] = { 0 };
    RandomBrightColour(&colour);
    p->colour[0] = colour[0];
    p->colour[1] = colour[1];
    p->colour[2] = colour[2];
    p->colour[3] = colour[3];

    p->remainingLife = RandIntRange(10, 40) / 10.0f;
    p->radius = 6;
    p->children = RandIntRange(5, 12);
}

void MakePTSpark(struct FWGLSimulation* simulation, int particle) {
    struct Particle* p = &(simulation->particles[particle]);

    p->type = PT_SPARK;

    p->velocity[0] = (float) RandIntRange(-200, 200);
    p->velocity[1] = (float) RandIntRange(-200, 200);
    p->velocity[2] = 0;
    p->acceleration[0] = 0;
    p->acceleration[1] = -100;
    p->acceleration[2] = 0;

    p->remainingLife = 1.0f;
    p->radius = 3;
    p->children = 0;
}

void MakePTHaze(struct FWGLSimulation* simulation, int particle) {
    struct Particle* p = &(simulation->particles[particle]);

    p->type = PT_HAZE;

    p->velocity[0] = 0;
    p->velocity[1] = 0;
    p->velocity[2] = 0;
    p->acceleration[0] = 0;
    p->acceleration[1] = -8;
    p->acceleration[2] = 0;

    p->remainingLife = 2.0f;
    p->radius = 1;
    p->children = 0;
}

void ProcessPTSparkRocket(struct FWGLSimulation* simulation, int particle, float dSecs) {
    struct Particle* rocket = &(simulation->particles[particle]);

    rocket->velocity[0] += RandIntRange(-30, 30) / 10.0f;
    rocket->radius += RandIntRange(-100, 100) / 2500.0f;

    if (rocket->timeSinceLastEmission > 0.05f) {
        rocket->timeSinceLastEmission = 0;

        int hId = ReviveDeadParticle(simulation);
        MakePTHaze(simulation, hId);
        struct Particle* haze = &(simulation->particles[hId]);

        float vMag = sqrt(
            rocket->velocity[0]*rocket->velocity[0] 
            + rocket->velocity[1]* rocket->velocity[1]
            + rocket->velocity[2]* rocket->velocity[2]
        );

        haze->position[0] = rocket->position[0] - (rocket->radius * rocket->velocity[0] / vMag);
        haze->position[1] = rocket->position[1] - (rocket->radius * rocket->velocity[1] / vMag);
        haze->position[2] = rocket->position[2] - (rocket->radius * rocket->velocity[2] / vMag);

        float erraticness = pow(min(0.35 / rocket->remainingLife, 1), 1.5);

        haze->velocity[0] = (- 0.75f * rocket->velocity[0]) + (erraticness * RandDouble() * rocket->velocity[1]);
        haze->velocity[1] = (- 0.75f * rocket->velocity[1]) + (erraticness * RandDouble() * rocket->velocity[0]);
        haze->velocity[2] = (- 0.75f * rocket->velocity[2]);

        haze->acceleration[0] = 0;
        haze->acceleration[1] = 0;
        haze->acceleration[2] = 0;
        haze->colour[0] = rocket->colour[0];
        haze->colour[1] = rocket->colour[1];
        haze->colour[2] = rocket->colour[2];
        haze->colour[3] = rocket->colour[3];
    }
    rocket->timeSinceLastEmission += dSecs;
}

void ProcessPTSpark(struct FWGLSimulation* simulation, int particle, float dSecs) {
    struct Particle* spark = &(simulation->particles[particle]);

    spark->acceleration[0] = -1.6f * spark->velocity[0];
    spark->acceleration[1] = -60;

    if (spark->timeSinceLastEmission > 0.1) {
        spark->timeSinceLastEmission = 0;

        int hId = ReviveDeadParticle(simulation);
        struct Particle* haze = &(simulation->particles[hId]);
        MakePTHaze(simulation, hId);

        haze->position[0] = spark->position[0];
        haze->position[1] = spark->position[1];
        haze->position[2] = spark->position[2];

        haze->velocity[0] = (0.1 * spark->velocity[0]) + 5 * (RandDouble() - 0.5);
        haze->velocity[1] = (0.1 * spark->velocity[1]) + 5 * (RandDouble() - 0.5);
        haze->velocity[2] = 0;

        haze->colour[0] = spark->colour[0];
        haze->colour[1] = spark->colour[1];
        haze->colour[2] = spark->colour[2];
        haze->colour[3] = spark->colour[3];
    }

    spark->timeSinceLastEmission += dSecs;
}

void ProcessPTHaze(struct FWGLSimulation* simulation, int particle, float dSecs) {
    // No processing required
    // Haze doesn't move and fading/alpha is handled by the fragment shader

    // This may cause some artifacting around red particles? Overexposure?
    struct Particle* haze = &(simulation->particles[particle]);
    double factor = (haze->remainingLife)/3 * (RandDouble() - 0.5) / 5.0f;
    haze->colour[0] += factor;
    haze->colour[1] += factor;
    haze->colour[2] += factor;
}

void KillPTSpark(struct FWGLSimulation* simulation, int particle) {
    // Sparks don't do anything special when they die
}

void KillPTSparkRocket(struct FWGLSimulation* simulation, int particle) {
    struct Particle* rocket = &(simulation->particles[particle]);

    float xTotal = 0;
    float yTotal = 0;
    float zTotal = 0;

    for (int i = 0; i < rocket->children; i++) {
        int sId = ReviveDeadParticle(simulation);
        struct Particle* spark = &(simulation->particles[sId]);
        MakePTSpark(simulation, sId);

        // Always try to push the total towards zero to make more even explosions.
        // If going in same x direction, flip vX
        if (xTotal * spark->velocity[0] > 0) {
            spark->velocity[0] *= -1;
        }
        // If going in same y direction, flip vY
        if (yTotal * spark->velocity[1] > 0) {
            spark->velocity[1] *= -1;
        }
        // If going in same z direction, flip vZ
        if (zTotal * spark->velocity[2] > 0) {
            spark->velocity[2] *= -1;
        }
        xTotal += spark->velocity[0];
        yTotal += spark->velocity[1];
        zTotal += spark->velocity[2];

        spark->position[0] = rocket->position[0];
        spark->position[1] = rocket->position[1];
        spark->position[2] = rocket->position[2];

        spark->velocity[0] += 0.5f * rocket->velocity[0];
        spark->velocity[1] += 0.5f * rocket->velocity[1];
        spark->velocity[2] += 0.5f * rocket->velocity[2];

        spark->colour[0] = rocket->colour[0];
        spark->colour[1] = rocket->colour[1];
        spark->colour[2] = rocket->colour[2];
        spark->colour[3] = rocket->colour[3];
    }
}

void KillPTHaze(struct FWGLSimulation* simulation, int particle) {
    // Haze does nothing special when it dies
}

void MoveParticles(struct FWGLSimulation* simulation, int width, int height, float dSecs) {
    // Sometimes the rocket count gets out of sync?
    // No idea how that happens, but here's a bodge for it
    int rocketCheck = 0;
    if (simulation->timeSinceRocketCount > 5.0f) {
        for (int i = 0; i < simulation->maxParticles; i++) {
            struct Particle* p = &(simulation->particles[i]);
            if ((p->type == PT_SPARK_ROCKET) && (p->isAlive)) { 
                rocketCheck++;
            }
        }

        simulation->liveRockets = rocketCheck;
        simulation->timeSinceRocketCount = 0;
    }
    simulation->timeSinceRocketCount += dSecs;

    // Make new rockets
    while (simulation->maxRockets > simulation->liveRockets) {
        int pId = ReviveDeadParticle(simulation);
        struct Particle* p = &(simulation->particles[pId]);
        MakePTSparkRocket(simulation, pId);
        simulation->liveRockets += 1;

        p->position[0] = (float)RandIntRange(200, width - 200);
        p->position[1] = -50;
        p->position[2] = 0;
    }

    // Move and process all particles
    for (int pId = 0; pId < simulation->maxParticles; pId++) {
        struct Particle* p = &(simulation->particles[pId]);

        // Skip already dead particles
        if (!p->isAlive) {
            continue;
        }

        // Kill old particles
        if (p->remainingLife <= 0) {
            switch (p->type) {
            case PT_SPARK:
                KillPTSpark(simulation, pId);
                break;
            case PT_SPARK_ROCKET:
                KillPTSparkRocket(simulation, pId);
                break;
            case PT_HAZE:
                KillPTHaze(simulation, pId);
                break;
            }

            DeleteParticle(simulation, pId);
        }

        // Kill out of bounds particles
        if (p->position[0] < -50
            || p->position[0] > width +50
            || p->position[1] < -50
            || p->position[1] > height +50) {
            DeleteParticle(simulation, pId);
        }

        // Skip newly dead particles
        if (!p->isAlive) {
            continue;
        }

        // Make particles older
        p->remainingLife -= dSecs;

        // Process different types of particle
        switch (p->type) {
        case PT_SPARK_ROCKET:
            ProcessPTSparkRocket(simulation, pId, dSecs);
            break;
        case PT_SPARK:
            ProcessPTSpark(simulation, pId, dSecs);
            break;
        case PT_HAZE:
            ProcessPTHaze(simulation, pId, dSecs);
            break;
        }

        // Update position and velocity
        p->position[0] += p->velocity[0] * dSecs;
        p->position[1] += p->velocity[1] * dSecs;
        p->position[2] += p->velocity[2] * dSecs;

        p->velocity[0] += p->acceleration[0] * dSecs;
        p->velocity[1] += p->acceleration[1] * dSecs;
        p->velocity[2] += p->acceleration[2] * dSecs;
    }
}