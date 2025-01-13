#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>

#include "fireworks_gl.h"

const int MAX_PARTICLES = 200;

const char* vertexShaderSource =
SHADER(     #version 330 core                                       )
SHADER(     layout(location = 0) in vec3 aPos;                      )
SHADER(     layout(location = 1) in vec2 aTranslate;                )
SHADER(     void main()                                             )
SHADER(     {                                                       )
SHADER(     \t gl_Position = vec4(aPos.x + aTranslate.x, aPos.y + aTranslate.y, aPos.z, 1.0);  )
SHADER(     }                                                       )"\0";


const char* fragmentShaderSource =
SHADER(     #version 330 core                               )
SHADER(     out vec4 FragColor;                             )
SHADER(     void main()                                     )
SHADER(     {                                               )
SHADER(     \t FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);     )
SHADER(     };                                              )"\0";


const float circleVertices[] = {
     1.000f,  0.000f, 0.0f,
     0.866f,  0.500f, 0.0f,
     0.707f,  0.707f, 0.0f,
     0.500f,  0.866f, 0.0f,

     0.000f,  1.000f, 0.0f,
    -0.500f,  0.866f, 0.0f,
    -0.707f,  0.707f, 0.0f,
    -0.866f,  0.500f, 0.0f,

    -1.000f,  0.000f, 0.0f,
    -0.866f, -0.500f, 0.0f,
    -0.707f, -0.707f, 0.0f,
    -0.500f, -0.866f, 0.0f,

     0.000f, -1.000f, 0.0f,
     0.500f, -0.866f, 0.0f,
     0.707f, -0.707f, 0.0f,
     0.866f, -0.500f, 0.0f,
};
const int circleIndices[] = {
    // Edge of the circle
     0,  1,  2,
     2,  3,  4, 
     4,  5,  6,
     6,  7,  8,
     8,  9, 10,
    10, 11, 12,
    12, 13, 14,
    14, 15,  0,
    // 
     0,  2,  4,
     4,  6,  8,
     8, 10, 12,
    12, 14,  0,
    //
     0,  4,  8,
     8, 12,  0
};

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
        glfwTerminate();
        return fwgl.error;
    }

    FWGL_compileShaders(&fwgl);
    if (fwgl.error != FWGL_OK) {
        printf("Failed to set up rendering infrastructure!");
        glfwTerminate();
        return fwgl.error;
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    FWGL_prepareBuffers(&fwgl);

    while (!glfwWindowShouldClose(fwgl.window)) {

        FWGL_process(&fwgl);
        FWGL_render(&fwgl);

        glfwSwapBuffers(fwgl.window);
        glfwPollEvents();
    }
    printf("Shutting down...\n");

    glDeleteVertexArrays(1, &(fwgl.VAO));
    glDeleteBuffers(1, &(fwgl.VBO));
    glDeleteBuffers(1, &(fwgl.EBO));
    glDeleteProgram(fwgl.shaderProgram);

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
    printf(" ~~ Help ~~ \n");
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

void FWGL_framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void FWGL_process(struct FWGL* fwgl) {
    if (GLFW_PRESS == glfwGetKey(fwgl->window, GLFW_KEY_SPACE)
            || GLFW_PRESS == glfwGetKey(fwgl->window, GLFW_KEY_ENTER)
            || GLFW_PRESS == glfwGetMouseButton(fwgl->window, GLFW_MOUSE_BUTTON_LEFT)
        ) {
        printf("Input detected! Triggering close...\n");
        glfwSetWindowShouldClose(fwgl->window, TRUE);
    }
}

void FWGL_getCircleVertices(float radius, float x, float y, float z, float vertices[]) {
    int count = (sizeof(circleVertices) / sizeof(float));

    for (int i = 0; i < count; i += 3) {
        float tx = circleVertices[i] * radius + x;
        float ty = circleVertices[i + 1] * radius + y;
        float tz = circleVertices[i + 2] * radius + z;
        vertices[i] = tx;
        vertices[i + 1] = ty;
        vertices[i + 2] = tz;
    }
}

void FWGL_compileShaders(struct FWGL* fwgl) {
    printf("\nVertex Shader:\n%s\n", vertexShaderSource);
    printf("\nFragment Shader:\n%s\n", fragmentShaderSource);

    int success;
    char log[512];

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, log);
        printf("Failed to compile vertex shader: %s", log);
        fwgl->error = FWGL_ERROR_INIT_COMPILEVERTEX;
        return;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, log);
        printf("Failed to compile fragment shader: %s", log);
        fwgl->error = FWGL_ERROR_INIT_COMPILEVERTEX;
        return;
    }

    unsigned program = glCreateProgram();
    fwgl->shaderProgram = program;
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, log);
        printf("Failed to link shader program: %s", log);
        fwgl->error = FWGL_ERROR_INIT_SHADERLINK;
        return;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    printf("Successfully compiled and linked shader program!\n");
}

void FWGL_prepareBuffers(struct FWGL* fwgl) {
    unsigned int dataVBO, VAO, vertexVBO, EBO;

    // Translations
    float data[] = {
         0.5f,  0.5f,
        -0.5f,  0.5f,
        -0.5f, -0.5f,
         0.5f, -0.5f
    };
    glGenBuffers(1, &dataVBO);
    glBindBuffer(GL_ARRAY_BUFFER, dataVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), &data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Vertices
    float vertices[48];
    FWGL_getCircleVertices(0.5, 0, 0, 0, &vertices);
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &vertexVBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(circleIndices), circleIndices, GL_STATIC_DRAW);

    // 3 floats (x,y,z) as vertex attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // 2 floats (translate x,y)
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, dataVBO);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(1, 1); // Stride of 1 between swapping attributes

    glBindVertexArray(0);

    fwgl->VAO = VAO;
    fwgl->VBO = vertexVBO;
    fwgl->EBO = EBO;
}

void FWGL_render(struct FWGL* fwgl) {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    int indexCount = (int)(sizeof(circleIndices) / sizeof(int));

    glUseProgram(fwgl->shaderProgram);
    glBindVertexArray(fwgl->VAO);
    glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, 4);
}