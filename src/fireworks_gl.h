typedef uint8_t BOOL;
#define TRUE 1
#define FALSE 0

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
};

void FWGL_parseArgs(struct FWGL* fwgl, int argc, char* argv[]);
void FWGL_createGLFWWindow(struct FWGL* fwgl);