#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>

#include "fireworks_gl.h"

int main(int argc, char *argv[])
{
    printf("FireworksGL!\n");

    struct FWGL fwgl;

    FWGL_parseArgs(&fwgl, argc, argv);
    if (fwgl.error != FWGL_OK) {
        printf("Error parsing arguments: %d\n", fwgl.error);
        return fwgl.error;
    }    

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    FWGL_createGLFWWindow(&fwgl);
    if (fwgl.error != FWGL_OK) {
        printf("Error creating GLFW window: %d\n", fwgl.error);
        return fwgl.error;
    }

    while (!glfwWindowShouldClose(fwgl.window)) {
        glfwSwapBuffers(fwgl.window);
        glfwPollEvents();
    }
    printf("Shutting down...\n");

    glfwTerminate();
    return FWGL_OK;
}

void FWGL_parseArgs(struct FWGL* fwgl, int argc, char* argv[]) {
    if (argc < 2) {
        fwgl->error = FWGL_ERROR_INIT_ARGCOUNT;
        return;
    }

    if (strcmp(argv[1], "/s") == 0) {
        fwgl->is_preview = FALSE;
    }
    else if (strcmp(argv[1], "/p") == 0) {
        fwgl->is_preview = TRUE;
    }
    else {
        printf("Unrecognised argument: %s\n", argv[1]);
        fwgl->error = FWGL_ERROR_INIT_UNKNOWNARG;
    }

    fwgl->error = FWGL_OK;
}

void FWGL_printHelp() {

}

void FWGL_createGLFWWindow(struct FWGL* fwgl) {

    GLFWwindow* window = NULL;
    int width = 0;
    int height = 0;
    
    if (fwgl->is_preview) {
        printf("Creating preview window\n");
        width = 800;
        height = 600;
        window = glfwCreateWindow(width, height, "FireworksGL", NULL, NULL);
    }
    else {
        printf("Creating full screen window\n");
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        width = mode->width;
        height = mode->height;
        printf("Using dimensions: %dx%d\n", width, height);

        window = glfwCreateWindow(width, height, "FireworksGL", monitor, NULL);
    }

    if (window == NULL) {
        printf("Failed to create GLFW window.\n");
        glfwTerminate();
        fwgl->error = FWGL_ERROR_INIT_GLFWWINDOW;
        return;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD\n");
        fwgl->error = FWGL_ERROR_INIT_GLAD;
        return;
    }

    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(window, glfwSetFramebufferSizeCallback);

    fwgl->window = window;
    fwgl->error = FWGL_OK;
}

//
// Prototype implementation!
//
void FWGL_framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}