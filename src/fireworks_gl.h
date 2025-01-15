#pragma once
#include "fireworks_gl_process.h"

typedef uint8_t BOOL;
#define TRUE 1
#define FALSE 0

#define SHADER(txt) #txt "\n"

enum FWGL_Error {
	FWGL_OK								=   0,
	FWGL_ERROR_INIT						= 100,
	FWGL_ERROR_INIT_ARGCOUNT			= 101,
	FWGL_ERROR_INIT_GLFWWINDOW			= 102,
	FWGL_ERROR_INIT_UNKNOWNARG			= 103,
	FWGL_ERROR_INIT_GLAD				= 104,
	FWGL_ERROR_INIT_COMPILEVERTEX		= 105,
	FWGL_ERROR_INIT_COMPILEFRAGMENT		= 106,
	FWGL_ERROR_INIT_SHADERLINK			= 107,
};

struct ParticleRenderData {
	float translate[3];
	float colour[4];
	float radius;
	float remainingLife;
	float particleType;
};

struct FWGL {
	enum FWGL_Error error;
	BOOL is_preview;
	GLFWwindow* window;

	unsigned int shaderProgram;
	unsigned int VAO, vertexVBO, dataVBO, EBO;
	struct FWGLSimulation simulation;
	struct ParticleRenderData* renderData;
};

#define TO_GLCOLOR(b) (b / 255.0f)

enum FWGL_Error FWGL_Init(struct FWGL* fwgl, int maxParticles, int maxRockets);
void FWGL_parseArgs(struct FWGL* fwgl, int argc, char* argv[]);
void FWGL_createGLFWWindow(struct FWGL* fwgl);
void FWGL_framebufferSizeCallback(GLFWwindow* window, int width, int height);
void FWGL_process(struct FWGL* fwgl, double dSecs);
void FWGL_compileShaders(struct FWGL* fwgl);
void FWGL_prepareBuffers(struct FWGL* fwgl);
void FWGL_render(struct FWGL* fwgl);