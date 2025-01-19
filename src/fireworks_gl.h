#pragma once
#include "fireworks_gl_process.h"

enum FWGL_Error {
	FWGL_OK										=   0,
	FWGL_ERROR_INIT								= 100,
	FWGL_ERROR_INIT_ARGCOUNT					= 101,
	FWGL_ERROR_INIT_GLFWWINDOW					= 102,
	FWGL_ERROR_INIT_UNKNOWNARG					= 103,
	FWGL_ERROR_INIT_GLAD						= 104,
	FWGL_ERROR_INIT_COMPILEVERTEX				= 105,
	FWGL_ERROR_INIT_COMPILEFRAGMENT				= 106,
	FWGL_ERROR_INIT_SHADERLINK					= 107,
	FWGL_ERROR_PREPBUFFER_FRAME_RENDER			= 200,
	FWGL_ERROR_PREPBUFFER_FRAME_EFFECT			= 201,
};

struct ParticleRenderData {
	float translate[3];
	float colour[4];
	float radius;
	float remainingLife;
	int particleType;
};

struct FWGL {
	enum FWGL_Error error;
	uint8_t is_preview;
	GLFWwindow* window;

	// Basic circle geometry
	unsigned int dimensionUBO, circleVAO, circleVBO, circleEBO, dataVBO;
	unsigned int geometryFBO, geometryTexture, geometryShader;
	unsigned int blurredFBO1, blurredTexture1, blurredShader;
	unsigned int blurredFBO2, blurredTexture2;
	unsigned int bloomFBO, bloomTexture, bloomShader;
	unsigned int screenShader, screenVAO;

	struct FWGLSimulation simulation;
	struct ParticleRenderData* renderData;
};

#define TO_GLCOLOR(b) (b / 255.0f)

enum FWGL_Error FWGL_Init(struct FWGL* fwgl, int maxParticles, int maxRockets);
void FWGL_parseArgs(struct FWGL* fwgl, int argc, char* argv[]);
void FWGL_createGLFWWindow(struct FWGL* fwgl);
void FWGL_framebufferSizeCallback(GLFWwindow* window, int width, int height);
void FWGL_process(struct FWGL* fwgl, float dSecs);
void FWGL_compileShader(unsigned int* program, const char* vertexSource, const char* fragSource);
void FWGL_prepareBuffers(struct FWGL* fwgl);
void FWGL_render(struct FWGL* fwgl);