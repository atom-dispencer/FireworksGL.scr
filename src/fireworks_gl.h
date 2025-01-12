typedef uint8_t BOOL;
#define TRUE 1
#define FALSE 0

#define SHADER(txt) #txt "\n"

enum FWGL_Error {
	FWGL_OK = 0,
	FWGL_ERROR_INIT_ARGCOUNT = 100,
	FWGL_ERROR_INIT_GLFWWINDOW = 101,
	FWGL_ERROR_INIT_UNKNOWNARG = 102,
	FWGL_ERROR_INIT_GLAD = 103
};

struct FWGL {
	enum FWGL_Error error;
	BOOL is_preview;
	GLFWwindow* window;

	unsigned int VAO;
	unsigned int shaderProgram;
};

#define TO_GLCOLOR(b) (b / 255.0f)

void FWGL_parseArgs(struct FWGL* fwgl, int argc, char* argv[]);
void FWGL_createGLFWWindow(struct FWGL* fwgl);
void FWGL_process(struct FWGL* fwgl);
void FWGL_initRender(struct FWGL* fwgl);
void FWGL_render(struct FWGL* fwgl);