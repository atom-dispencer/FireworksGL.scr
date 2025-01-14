#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "fireworks_gl.h"
#include "fireworks_gl_process.h"

const int MAX_PARTICLES = 200;

const char* vertexShaderSource =
SHADER(     #version 330 core                                       )
SHADER(     layout(location = 0) in vec3 aBasePos;                  )
SHADER(     layout(location = 1) in vec3 aTranslate;                )
SHADER(     layout(location = 2) in vec4 aColour;                   )
SHADER(     layout(location = 3) in float radius;                   )
SHADER(     out vec4 vertexColour;                                  )
SHADER(     void main()                                             )
SHADER(     {                                                       )
SHADER(     \t gl_Position = vec4(aBasePos*radius + aTranslate, 1.0f);      )
SHADER(     \t vertexColour = aColour;                              )
SHADER(     }                                                       )"\0";


const char* fragmentShaderSource =
SHADER(     #version 330 core                               )
SHADER(     out vec4 FragColor;                             )
SHADER(     in vec4 vertexColour;                           )
SHADER(     void main()                                     )
SHADER(     {                                               )
SHADER(     \t FragColor = vec4(vertexColour);              )
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

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glfwSwapInterval(0);
    FWGL_prepareBuffers(&fwgl);

    // Set up timing
    long long lastEpochNano = 0;
    long long thisEpochNano = 0;
    struct timespec ts;

    timespec_get(&ts, TIME_UTC);
    lastEpochNano = ts.tv_sec * 1e9 + ts.tv_nsec;
    thisEpochNano = lastEpochNano + 1;

    long long dNanos;
    double dSecs;

    while (!glfwWindowShouldClose(fwgl.window)) {
        timespec_get(&ts, TIME_UTC);
        thisEpochNano = ts.tv_sec * 1e9 + ts.tv_nsec;
        dNanos = thisEpochNano - lastEpochNano;
        dSecs = dNanos / 1e9;
        lastEpochNano = thisEpochNano;
        printf("\n%.6fs\n%ffps\n", dSecs, 1 / dSecs);

        FWGL_process(&fwgl, dSecs);
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
    glfwSetFramebufferSizeCallback(window, FWGL_framebufferSizeCallback);

    fwgl->window = window;
    fwgl->error = FWGL_OK;
}

void FWGL_framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void FWGL_process(struct FWGL* fwgl, double dSecs) {
    if (GLFW_PRESS == glfwGetKey(fwgl->window, GLFW_KEY_SPACE)
            || GLFW_PRESS == glfwGetKey(fwgl->window, GLFW_KEY_ENTER)
            || GLFW_PRESS == glfwGetMouseButton(fwgl->window, GLFW_MOUSE_BUTTON_LEFT)
        ) {
        printf("Input detected! Triggering close...\n");
        glfwSetWindowShouldClose(fwgl->window, TRUE);
    }

    MoveParticles();
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

    float data[] = {
        // Translate x,y,z      // Colour r,g,b,a           Radius
         0.5f,  0.5f,  0.0f,    1.0f, 0.0f, 0.0f, 1.0f,     0.5f,
        -0.5f,  0.5f,  0.0f,    0.0f, 1.0f, 0.0f, 1.0f,     0.4f,
        -0.5f, -0.5f,  0.0f,    0.0f,  0.0f, 1.0f, 1.0f,    0.3f,
         0.5f, -0.5f,  0.0f,    1.0f,  1.0f, 0.0f, 1.0f,    0.2f,
    };
    glGenBuffers(1, &dataVBO);
    glBindBuffer(GL_ARRAY_BUFFER, dataVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), &data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Vertices
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &vertexVBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circleVertices), circleVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(circleIndices), circleIndices, GL_STATIC_DRAW);
    
    // Vertex attributes
    // Vertex base position (x,y,z)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // Translate (x,y,z)
    glBindBuffer(GL_ARRAY_BUFFER, dataVBO);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    // Colour (r,g,b,a)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    // Radius (r)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(7 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(1, 1); // Stride of 1 between swapping attributes
    glVertexAttribDivisor(2, 1); // Stride of 1 between swapping attributes
    glVertexAttribDivisor(3, 1); // Stride of 1 between swapping attributes

    glBindVertexArray(0);

    fwgl->VAO = VAO;
    fwgl->VBO = vertexVBO;
    fwgl->EBO = EBO;
}

void FWGL_render(struct FWGL* fwgl) {

    struct Particle* particles;
    struct Particle p;
    int ptr = 0;
    for (int i = 0; i < MAX_PARTICLES; i++) {
        p = particles[i];
        if (!p.isAlive) {
            continue;
        }

        struct ParticleRenderData data;
        // Translate (x,y,z)
        data.translate[0] = p.position[0];
        data.translate[1] = p.position[1];
        data.translate[2] = p.position[2];
        // Colour (r,g,b,a)
        data.colour[0] = p.color[0];
        data.colour[1] = p.color[1];
        data.colour[2] = p.color[2];
        data.colour[3] = p.color[3];
        // Radius
        data.radius = p.radius;

        fwgl->renderData[ptr] = data;
    }

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    int indexCount = (int)(sizeof(circleIndices) / sizeof(int));

    glUseProgram(fwgl->shaderProgram);
    glBindVertexArray(fwgl->VAO);
    glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, 4);
}