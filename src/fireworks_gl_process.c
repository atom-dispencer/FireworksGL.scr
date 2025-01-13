#include "fireworks_gl_process.h"

const int MAX_ROCKETS = 1;
struct Particle* PARTICLES[200];
int MAX_PARTICLES = sizeof(PARTICLES) / sizeof(struct Particle);
int currentRockets = 0;
int currentParticles = 0;

int RandInRange(int lower, int upper) {
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

void DeleteParticle(Particle& p) {
    if (p.type == PT_SPARK_ROCKET) {
        currentRockets--;
    }

    p.isAlive = false;
    currentParticles--;
}

Color RandomBrightColour() {
    int offset = RandInRange(0, 3);
    int channels[3] = { 0xff, 0xff, 0xff };

    for (int i = 0; i < 3; i++) {
        int index = (offset + i) % 3;

        switch (index) {
        case 0:
            channels[i] = 0;
            break;
        case 1:
            channels[i] = 255;
            break;
        case 2:
            channels[i] = RandInRange(0, 255);
            break;
        }
    }


    return Color(0xff, channels[0], channels[1], channels[2]);
}

Particle& ReviveDeadParticle() {
    for (Particle& c : PARTICLES) {
        if (!c.isAlive) {
            c.isAlive = true;
            currentParticles++;
            return c;
        }
    }

    // I hope this never happens
    return PARTICLES[RandInRange(0, MAX_PARTICLES)];
}

void MakePTSparkRocket(Particle& p) {
    p.type = PT_SPARK_ROCKET;

    p.vX = RandInRange(-100, 100);
    p.vY = RandInRange(-400, -250);
    p.aX = 0;
    p.aY = 100;

    p.remainingLife = RandInRange(10, 40) / 10.0f;
    p.radius = 15;
    p.color = RandomBrightColour();
    p.children = RandInRange(5, 12);
}

void MakePTSpark(Particle& p) {
    p.type = PT_SPARK;

    p.vX = RandInRange(-200, 200);
    p.vY = RandInRange(-200, 200);
    p.aX = 0;
    p.aY = 100;

    p.remainingLife = 1.0f;
    p.radius = 9;
    p.children = 0;
}

void MakePTHaze(Particle& p) {
    p.type = PT_HAZE;

    p.vX = 0;
    p.vY = 0;
    p.aX = 0;
    p.aY = 8;

    p.remainingLife = 3.0f;
    p.radius = 5;
    p.children = 0;
}

void ProcessPTSparkRocket(Particle& rocket, float dSecs) {

    rocket.vX += RandInRange(-30, 30) / 10.0f;

    if (rocket.timeSinceLastEmission > 0.05f) {
        rocket.timeSinceLastEmission = 0;

        Particle& s = ReviveDeadParticle();
        MakePTHaze(s);
        s.pX = rocket.pX;
        s.pY = rocket.pY;
        s.vX = -0.75 * rocket.vX;
        s.vY = -0.75 * rocket.vY;
        s.aX = 0;
        s.aY = 0;
        s.color = rocket.color;
    }
    rocket.timeSinceLastEmission += dSecs;
}

void ProcessPTSpark(Particle& spark, float dSecs) {
    spark.aX = -1.6 * spark.vX;
    spark.aY = 60;

    if (spark.remainingLife < 0.5) {
        float factor = 2 * spark.remainingLife;

        // Remaining life is < 1
        BYTE bAlpha = (BYTE)(255 * factor * factor);
        int iAlpha = (int)bAlpha;
        int siAlpha = iAlpha << 24;
        ARGB _rgb = spark.color.GetValue() & 0x00ffffff;
        ARGB argb = _rgb | siAlpha;
        spark.color.SetValue(argb);
    }

    if (spark.timeSinceLastEmission > 0.1) {
        spark.timeSinceLastEmission = 0;
        Particle& haze = ReviveDeadParticle();
        MakePTHaze(haze);
        haze.pX = spark.pX;
        haze.pY = spark.pY;
        haze.color = spark.color;
    }

    spark.timeSinceLastEmission += dSecs;
}

void ProcessPTHaze(Particle& p, float dSecs) {
    float factor = p.remainingLife / 3.0f;

    BYTE bAlpha = (BYTE)(255 * 0.5f * factor * factor);
    int iAlpha = (int)bAlpha;
    int siAlpha = iAlpha << 24;
    ARGB _rgb = p.color.GetValue() & 0x00ffffff;
    ARGB argb = _rgb | siAlpha;
    p.color.SetValue(argb);
}

void KillPTSpark(Particle& p) {
}

void KillPTSparkRocket(Particle& rocket) {

    int xTotal = 0;
    int yTotal = 0;

    for (int i = 0; i < rocket.children; i++) {
        Particle& spark = ReviveDeadParticle();
        MakePTSpark(spark);

        // Always try to push the total towards zero to make more even
        // explosions.
        // If going in same x direction, flip vX
        if (xTotal * spark.vX > 0) {
            spark.vX *= -1;
        }
        // If going in same y direction, flip vY
        if (yTotal * spark.vY > 0) {
            spark.vY *= -1;
        }
        xTotal += spark.vX;
        yTotal += spark.vY;

        spark.color = rocket.color;
        spark.pX = rocket.pX;
        spark.pY = rocket.pY;

        spark.vX += 0.5f * rocket.vX;
        spark.vY += 0.5f * rocket.vY;
    }
}

void KillPTHaze(Particle& p) {
}

float timeSinceRocketCount = 0;
auto lastStep = std::chrono::high_resolution_clock::now();
void MoveParticles(HWND hWnd) {
    auto thisStep = std::chrono::high_resolution_clock::now();
    float dSecs = std::chrono::duration_cast<std::chrono::nanoseconds>(thisStep - lastStep).count() * 1.0e-9;
    lastStep = thisStep;

    if (isPreview) {
        frameTimes[frameTimeIndex++] = dSecs;
    }

    RECT rc;
    GetClientRect(
        hWnd,
        &rc
    );

    // Sometimes the rocket count gets out of sync?
    // No idea how that happens, but here's a bodge for it
    int rocketCheck = 0;
    if (timeSinceRocketCount > 5.0f) {
        for (Particle& p : PARTICLES) {
            if (p.type == PT_SPARK_ROCKET && p.isAlive) {
                rocketCheck++;
            }
        }

        currentRockets = rocketCheck;
        timeSinceRocketCount = 0;
    }
    timeSinceRocketCount += dSecs;

    for (Particle& p : PARTICLES) {

        // Create new rockets
        if (!p.isAlive) {
            if (MAX_ROCKETS > currentRockets) {

                MakePTSparkRocket(p);

                p.pX = RandInRange(rc.left + 200, rc.right - 200);
                p.pY = rc.bottom + 50;

                p.isAlive = true;
                currentRockets += 1;
                currentParticles += 1;
            }

            continue;
        }

        // Make particles older
        p.remainingLife -= dSecs;

        if (p.pX < rc.left - 50 || p.pX > rc.right + 50 || p.pY < rc.top - 50 || p.pY > rc.bottom + 50) {
            DeleteParticle(p);
            continue;
        }

        // Kill old particles
        if (p.remainingLife <= 0) {
            switch (p.type) {
            case PT_SPARK:
                KillPTSpark(p);
                break;
            case PT_SPARK_ROCKET:
                KillPTSparkRocket(p);
                break;
            case PT_HAZE:
                KillPTHaze(p);
                break;
            }

            DeleteParticle(p);
            continue;
        }

        // Process different types of particle
        switch (p.type) {
        case PT_SPARK_ROCKET:
            ProcessPTSparkRocket(p, dSecs);
            break;
        case PT_SPARK:
            ProcessPTSpark(p, dSecs);
            break;
        case PT_HAZE:
            ProcessPTHaze(p, dSecs);
            break;
        }

        // 
        p.pX += p.vX * dSecs;
        p.pY += p.vY * dSecs;
        p.vX += p.aX * dSecs;
        p.vY += p.aY * dSecs;
    }

    InvalidateRect(hWnd, NULL, NULL);
}