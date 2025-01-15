#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "fireworks_gl.h"
#include "fireworks_gl_process.h"

const char* vertexShaderSource =
SHADER(     #version 330 core                                               )
SHADER(     layout(location = 0) in vec3 aBasePos;                          )
SHADER(     layout(location = 1) in vec3 aTranslate;                        )
SHADER(     layout(location = 2) in vec4 aColour;                           )
SHADER(     layout(location = 3) in float aRadius;                          )
SHADER(     layout(location = 4) in float aRemainingLife;                   )
SHADER(     layout(location = 5) in int aParticleType;                      )
SHADER(     uniform in vec2 dimensions;                                     )
SHADER(     out vec4 vertexColour;                                          )
SHADER(     out float remainingLife;                                        )
SHADER(     flat out int particleType;                                      )
SHADER(     void main()                                                     )
SHADER(     {                                                               )
SHADER(     \t gl_Position = vec4(aBasePos*aRadius + aTranslate, 1.0f);     )
SHADER(     \t gl_Position.x /= (dimensions.x / 2);                         )
SHADER(     \t gl_Position.y /= (dimensions.y / 2);                         )
SHADER(     \t gl_Position += vec4(-1, -1, 0, 0);                           )
SHADER(     \t vertexColour = aColour;                                      )
SHADER(     \t remainingLife = aRemainingLife;                              )
SHADER(     \t particleType = aParticleType;                                )
SHADER(     }                                                               )"\0";


const char* fragmentShaderSource =
SHADER(     #version 330 core                                       )
SHADER(     out vec4 FragColor;                                     )
SHADER(     in vec4 vertexColour;                                   )
SHADER(     in float remainingLife;                                 )
SHADER(     flat in int particleType;                               )
SHADER(     void main()                                             )
SHADER(     {                                                       )
SHADER(     \t FragColor = vec4(vertexColour);                      )
SHADER(     \t if (particleType == 0 && remainingLife < 0.5) {      )
SHADER(     \t \t float factor = 2 * remainingLife;                 )
SHADER(     \t \t FragColor.w = factor * factor;                    )
SHADER(     \t }                                                    )
SHADER(     \t if (particleType == 2) {                             )
SHADER(     \t \t float factor = remainingLife / 3;                 )
SHADER(     \t \t FragColor.w = 0.5 * factor * factor;              )
SHADER(     \t }                                                    )
SHADER(     }                                                       )"\0";


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

    struct FWGL* fwgl = malloc(sizeof(struct FWGL));
    FWGL_Init(fwgl, 200, 1);

    FWGL_parseArgs(fwgl, argc, argv);
    if (fwgl->error != FWGL_OK) {
        printf("Error parsing arguments: %d\n", fwgl->error);
        return fwgl->error;
    }    

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    FWGL_createGLFWWindow(fwgl);
    if (fwgl->error != FWGL_OK) {
        printf("Error creating GLFW window: %d\n", fwgl->error);
        glfwTerminate();
        return fwgl->error;
    }

    FWGL_compileShaders(fwgl);
    if (fwgl->error != FWGL_OK) {
        printf("Failed to set up rendering infrastructure!");
        glfwTerminate();
        return fwgl->error;
    }

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glfwSwapInterval(0);
    FWGL_prepareBuffers(fwgl);

    // Set up timing
    long long lastEpochNano = 0;
    long long thisEpochNano = 0;
    struct timespec ts;

    timespec_get(&ts, TIME_UTC);
    lastEpochNano = (long long)(ts.tv_sec * 1e9 + ts.tv_nsec);
    thisEpochNano = lastEpochNano + 1;

    long long dNanos;
    float dSecs;

    int i = 0;
    while (!glfwWindowShouldClose(fwgl->window)) {
        timespec_get(&ts, TIME_UTC);
        thisEpochNano = (long long)(ts.tv_sec * 1e9 + ts.tv_nsec);
        dNanos = thisEpochNano - lastEpochNano;
        dSecs = (float) (dNanos / 1e9);
        lastEpochNano = thisEpochNano;
        printf("\n%.6fs\n%ffps\n", dSecs, 1 / dSecs);

        FWGL_process(fwgl, dSecs);
        FWGL_render(fwgl);

        glfwSwapBuffers(fwgl->window);
        glfwPollEvents();

        printf("%d\n", i++);
    }
    printf("Shutting down...\n");

    glDeleteVertexArrays(1, &(fwgl->VAO));
    glDeleteBuffers(1, &(fwgl->vertexVBO));
    glDeleteBuffers(1, &(fwgl->EBO));
    glDeleteProgram(fwgl->shaderProgram);

    glfwTerminate();
    return FWGL_OK;
}

enum FWGL_Error FWGL_Init(struct FWGL* fwgl, int maxParticles, int maxRockets) {
    fwgl->error = FWGL_ERROR_INIT;
    fwgl->is_preview = 0;
    fwgl->window, fwgl->shaderProgram, fwgl->VAO, fwgl->vertexVBO, fwgl->dataVBO, fwgl->EBO = -1;
    fwgl->renderData = malloc(sizeof(struct ParticleRenderData) * maxParticles);

    struct FWGLSimulation simulation;
    simulation.maxParticles = maxParticles;
    simulation.liveParticles = 0;
    simulation.maxRockets = maxRockets;
    simulation.liveRockets = 0;
    simulation.particles = malloc(sizeof(struct Particle) * maxParticles);
    simulation.timeSinceRocketCount = 0;
    fwgl->simulation = simulation;

    struct Particle defaultParticle;
    defaultParticle.isAlive = 0;
    defaultParticle.position[0] = 0;
    defaultParticle.position[1] = 0;
    defaultParticle.position[2] = 0;
    defaultParticle.velocity[0] = 0;
    defaultParticle.velocity[1] = 0;
    defaultParticle.velocity[2] = 0;
    defaultParticle.acceleration[0] = 0;
    defaultParticle.acceleration[1] = 0;
    defaultParticle.acceleration[2] = 0;
    defaultParticle.colour[0] = 1;
    defaultParticle.colour[1] = 1;
    defaultParticle.colour[2] = 1;
    defaultParticle.colour[3] = 1;
    defaultParticle.children = 0;
    defaultParticle.radius = 0;
    defaultParticle.remainingLife = 0;
    defaultParticle.timeSinceLastEmission = 0;
    defaultParticle.type = PT_HAZE;

    for (int i = 0; i < simulation.maxParticles; i++) {
        fwgl->simulation.particles[i] = defaultParticle;
    }

    struct ParticleRenderData defaultRenderData;
    defaultRenderData.translate[0] = 0;
    defaultRenderData.translate[1] = 0;
    defaultRenderData.translate[2] = 0;
    defaultRenderData.colour[0] = 1;
    defaultRenderData.colour[1] = 1;
    defaultRenderData.colour[2] = 1;
    defaultRenderData.colour[3] = 1;
    defaultRenderData.radius = 1;
    defaultRenderData.remainingLife = 0;
    defaultRenderData.particleType = PT_HAZE;

    for (int i = 0; i < simulation.maxParticles; i++) {
        fwgl->renderData[i] = defaultRenderData;
    }

    fwgl->error = FWGL_OK;
    return FWGL_OK;
}

enum FWGL_Error FWGL_DeInit(struct FWGL* fwgl) {
    free(fwgl->renderData);
    free(fwgl->simulation.particles);
    free(fwgl);

    return FWGL_OK;
}

void FWGL_parseArgs(struct FWGL* fwgl, int argc, char* argv[]) {
    if (argc < 2) {
        printf("Not enough arguments!\n");
        fwgl->error = FWGL_ERROR_INIT_ARGCOUNT;
        return;
    }

    if (strcmp(argv[1], "/s") == 0) {
        fwgl->is_preview = 0;
    }
    else if (strcmp(argv[1], "/p") == 0) {
        fwgl->is_preview = 1;
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

void FWGL_process(struct FWGL* fwgl, float dSecs) {
    if (GLFW_PRESS == glfwGetKey(fwgl->window, GLFW_KEY_SPACE)
            || GLFW_PRESS == glfwGetKey(fwgl->window, GLFW_KEY_ENTER)
            || GLFW_PRESS == glfwGetMouseButton(fwgl->window, GLFW_MOUSE_BUTTON_LEFT)
        ) {
        printf("Input detected! Triggering close...\n");
        glfwSetWindowShouldClose(fwgl->window, GLFW_TRUE);
    }

    int width;
    int height;
    glfwGetWindowSize(fwgl->window, &width, &height);
    MoveParticles(&(fwgl->simulation), width, height, dSecs);
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

    //float data[] = {
    //    // Translate x,y,z      // Colour r,g,b,a           // Radius     // RL   // PT
    //     0.5f,  0.5f,  0.0f,    1.0f, 0.0f, 0.0f, 1.0f,     0.5f,         1.0f,   2,
    //    -0.5f,  0.5f,  0.0f,    0.0f, 1.0f, 0.0f, 1.0f,     0.4f,         0.5f,   2,
    //    -0.5f, -0.5f,  0.0f,    0.0f,  0.0f, 1.0f, 1.0f,    0.3f,         0.3f,   2,
    //     0.5f, -0.5f,  0.0f,    1.0f,  1.0f, 0.0f, 1.0f,    0.2f,         0.1f,   2,
    //};
    //glGenBuffers(1, &dataVBO);
    //glBindBuffer(GL_ARRAY_BUFFER, dataVBO);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(data), &data, GL_STATIC_DRAW);
    //glBindBuffer(GL_ARRAY_BUFFER, 0);

    // 3*f Translate (x,y,z)
    // 4*f Colour (r,g,b,a)
    // 1*f Radius (r)
    // 1*f Remaining Life (l)
    // 1*i Particle Type (t)
    glGenBuffers(1, &dataVBO);

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
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct ParticleRenderData), (void*)0);
    // Colour (r,g,b,a)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(struct ParticleRenderData), (void*)(3 * sizeof(float)));
    // Radius (r)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(struct ParticleRenderData), (void*)(7 * sizeof(float)));
    // Remaining Life (l)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(struct ParticleRenderData), (void*)(8 * sizeof(float)));
    // Particle Type (t)
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 1, GL_INT, GL_FALSE, sizeof(struct ParticleRenderData), (void*)(9 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(1, 1); // Stride of 1 between swapping attributes
    glVertexAttribDivisor(2, 1); // Stride of 1 between swapping attributes
    glVertexAttribDivisor(3, 1); // Stride of 1 between swapping attributes
    glVertexAttribDivisor(4, 1); // Stride of 1 between swapping attributes
    glVertexAttribDivisor(5, 1); // Stride of 1 between swapping attributes

    glBindVertexArray(0);

    fwgl->VAO = VAO;
    fwgl->vertexVBO = vertexVBO;
    fwgl->dataVBO = dataVBO;
    fwgl->EBO = EBO;
}

void FWGL_render(struct FWGL* fwgl) {

    struct FWGLSimulation* simulation = &(fwgl->simulation);
    struct Particle* p;
    int ptr = 0;

    int renderParticles = 0;

    for (int i = 0; i < simulation->maxParticles; i++) {
        p = &(simulation->particles[i]);
        if (!p->isAlive) {
            continue;
        }
        renderParticles++;

        struct ParticleRenderData data;
        // Translate (x,y,z)
        data.translate[0] = p->position[0];
        data.translate[1] = p->position[1];
        data.translate[2] = p->position[2];
        // Colour (r,g,b,a)
        data.colour[0] = p->colour[0];
        data.colour[1] = p->colour[1];
        data.colour[2] = p->colour[2];
        data.colour[3] = p->colour[3];
        // Radius (r)
        data.radius = p->radius;
        // Remaining Life (l)
        data.remainingLife = p->remainingLife;
        // Particle Type (t)
        data.particleType = p->type;

        fwgl->renderData[ptr] = data;
    }

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // THE ERROR IS IN HERE!!!
    // WHEN RENDERPARTICLES == 0, there is no error!!
    // nvoglv64 is in the Nvidia driver, so I'm probably accessing memory I'm not allowed to!
    if (renderParticles > 0) {
        int bufferSize = sizeof(struct ParticleRenderData) * renderParticles;
        glBindBuffer(GL_ARRAY_BUFFER, fwgl->dataVBO);
        glBufferData(GL_ARRAY_BUFFER, bufferSize, fwgl->renderData, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        int indexCount = (int)(sizeof(circleIndices) / sizeof(int));

        glUseProgram(fwgl->shaderProgram);
        glBindVertexArray(fwgl->VAO);
        glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, 4);
    }
}