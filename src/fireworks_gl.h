#include "fireworks_gl_process.h"

typedef uint8_t BOOL;
#define TRUE 1
#define FALSE 0

#define SHADER(txt) #txt "\n"

enum FWGL_Error {
	FWGL_OK = 0,
	FWGL_ERROR_INIT_ARGCOUNT = 100,
	FWGL_ERROR_INIT_GLFWWINDOW = 101,
	FWGL_ERROR_INIT_UNKNOWNARG = 102,
	FWGL_ERROR_INIT_GLAD = 103,
	FWGL_ERROR_INIT_COMPILEVERTEX = 104,
	FWGL_ERROR_INIT_COMPILEFRAGMENT = 105,
	FWGL_ERROR_INIT_SHADERLINK = 106,
};

struct ParticleRenderData {
	float translate[3];
	float colour[4];
	float radius;
};

struct FWGL {
	enum FWGL_Error error;
	BOOL is_preview;
	GLFWwindow* window;

	unsigned int shaderProgram;
	unsigned int VAO, VBO, EBO;

	int maxParticles;
	struct Particle particles[];
	struct ParticleRenderData renderData[];
};

#define TO_GLCOLOR(b) (b / 255.0f)

void FWGL_parseArgs(struct FWGL* fwgl, int argc, char* argv[]);
void FWGL_createGLFWWindow(struct FWGL* fwgl);
void FWGL_framebufferSizeCallback(GLFWwindow* window, int width, int height);
void FWGL_process(struct FWGL* fwgl, double dSecs);
void FWGL_compileShaders(struct FWGL* fwgl);
void FWGL_prepareBuffers(struct FWGL* fwgl);
void FWGL_render(struct FWGL* fwgl);