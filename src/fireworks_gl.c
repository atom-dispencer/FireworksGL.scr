#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "fireworks_gl.h"
#include "fireworks_gl_process.h"

const char *stdVertexShaderSource =
    "   #version 330 core                                               \n"
    "   layout(location = 0) in vec3 aBasePos;                          \n"
    "   layout(location = 1) in vec3 aTranslate;                        \n"
    "   layout(location = 2) in vec4 aColour;                           \n"
    "   layout(location = 3) in float aRadius;                          \n"
    "   layout(location = 4) in float aRemainingLife;                   \n"
    "   layout(location = 5) in int aParticleType;                      \n"
    "                                                                   \n"
    "   layout (std140) uniform WindowDimensions {                      \n"
    "       int width;                                                  \n"
    "       int height;                                                 \n"
    "   };                                                              \n"
    "                                                                   \n"
    "   out vec4 vertexColour;                                          \n"
    "   out float remainingLife;                                        \n"
    "   flat out int particleType;                                      \n"
    "                                                                   \n"
    "   void main()                                                     \n"
    "   {                                                               \n"
    "       gl_Position = vec4(aBasePos*aRadius + aTranslate, 1.0f);    \n"
    "       gl_Position.x /= (width / 2.0f);                            \n"
    "       gl_Position.y /= (height / 2.0f);                           \n"
    "       gl_Position += vec4(-1, -1, 0, 0);                          \n"
    "       vertexColour = aColour;                                     \n"
    "       remainingLife = aRemainingLife;                             \n"
    "       particleType = aParticleType;                               \n"
    "   }                                                               \n"
    "\0";

const char *stdFragmentShaderSource =
    "   #version 330 core                                               \n"
    "   out vec4 FragColor;                                             \n"
    "   in vec4 vertexColour;                                           \n"
    "   in float remainingLife;                                         \n"
    "   flat in int particleType;                                       \n"
    "   void main() {                                                   \n"
    "       FragColor = vec4(vertexColour);                             \n"
    "       if (particleType == 0 && remainingLife < 0.5) {             \n"
    "           float factor = 2 * remainingLife;                       \n"
    "           FragColor.w = factor * factor;                          \n"
    "       }                                                           \n"
    "       if (particleType == 2) {                                    \n"
    "           float factor = remainingLife / 3;                       \n"
    "           FragColor.w = 0.5 * factor * factor;                    \n"
    "       }                                                           \n"
    "   }                                                               \n"
    "\0";

const char* quadVertexShaderSource =
    "#version 330 core                                      \n"
    "layout(location = 0) in vec2 aPos;                     \n"
    "layout(location = 1) in vec2 aTexCoords;               \n"
    "                                                       \n"
    "out vec2 TexCoords;                                    \n"
    "                                                       \n"
    "void main()                                            \n"
    "{                                                      \n"
    "    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);      \n"
    "    TexCoords = aTexCoords;                            \n"
    "}                                                      \n"
    "\0";

const char* quadFragmentShaderSource =
    "#version 330 core                                      \n"
    "out vec4 FragColor;                                    \n"
    "                                                       \n"
    "in vec2 TexCoords;                                     \n"
    "                                                       \n"
    "uniform sampler2D screenTexture;                       \n"
    "                                                       \n"
    "void main()                                            \n"
    "{                                                      \n"
    "    FragColor = texture(screenTexture, TexCoords);     \n"
    "}                                                      \n"
    "\0";

const float circleVertices[] = {
    1.000f,  0.000f,  0.0f, 0.866f,  0.500f,  0.0f,
    0.707f,  0.707f,  0.0f, 0.500f,  0.866f,  0.0f,

    0.000f,  1.000f,  0.0f, -0.500f, 0.866f,  0.0f,
    -0.707f, 0.707f,  0.0f, -0.866f, 0.500f,  0.0f,

    -1.000f, 0.000f,  0.0f, -0.866f, -0.500f, 0.0f,
    -0.707f, -0.707f, 0.0f, -0.500f, -0.866f, 0.0f,

    0.000f,  -1.000f, 0.0f, 0.500f,  -0.866f, 0.0f,
    0.707f,  -0.707f, 0.0f, 0.866f,  -0.500f, 0.0f,
};

const int circleIndices[] = {
    // Edge of the circle
    0, 1, 2, 2, 3, 4, 4, 5, 6, 6, 7, 8, 8, 9, 10, 10, 11, 12, 12, 13, 14, 14,
    15, 0,
    //
    0, 2, 4, 4, 6, 8, 8, 10, 12, 12, 14, 0,
    //
    0, 4, 8, 8, 12, 0};

float quadVertices[] = {
    // Top left triangle
     1,  1, 0,
    -1, -1, 0,
    -1,  1, 0,
    // Bottom right triangle
     1,  1, 0,
    -1, -1, 0,
     1, -1, 0
};

int main(int argc, char *argv[]) {
  printf("FireworksGL!\n");

  struct FWGL *fwgl = malloc(sizeof(struct FWGL));
  FWGL_Init(fwgl, 250, 1);

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
    printf("Failed to set up rendering infrastructure!\n");
    glfwTerminate();
    return fwgl->error;
  }

  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  // glfwSwapInterval(0);  // 0 for vsync off
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  FWGL_prepareBuffers(fwgl);
  if (fwgl->error != FWGL_OK) {
      printf("Error preparing OpenGL buffers: %d\n", fwgl->error);
      glfwTerminate();
      return fwgl->error;
  }

  // Set up timing
  long long lastEpochNano = 0;
  long long thisEpochNano = 0;
  struct timespec ts;

  timespec_get(&ts, TIME_UTC);
  lastEpochNano = (long long)(ts.tv_sec * 1e9 + ts.tv_nsec);
  thisEpochNano = lastEpochNano + 1;

  long long dNanos;
  float dSecs;

  while (!glfwWindowShouldClose(fwgl->window)) {
    timespec_get(&ts, TIME_UTC);
    thisEpochNano = (long long)(ts.tv_sec * 1e9 + ts.tv_nsec);
    dNanos = thisEpochNano - lastEpochNano;
    dSecs = (float)(dNanos / 1e9);
    lastEpochNano = thisEpochNano;

    //printf("\n%.6fs\n%ffps\n", dSecs, 1 / dSecs);

    FWGL_process(fwgl, dSecs);
    FWGL_render(fwgl);

    glfwSwapBuffers(fwgl->window);
    glfwPollEvents();
  }
  printf("Shutting down gracefully...\n");

  enum FWGL_Error error = FWGL_DeInit(fwgl);
  if (error != FWGL_OK) {
      printf("Error deinitialising FWGL: %d\n", error);
  }

  printf("Terminating GLFW\n");
  glfwTerminate();

  printf("Shutdown OK\n");
  return FWGL_OK;
}

enum FWGL_Error FWGL_Init(struct FWGL *fwgl, int maxParticles, int maxRockets) {
    printf("Initialising new FWGL with maxParticles=%d, maxRockets=%d\n", maxParticles, maxRockets);
    fwgl->error = FWGL_ERROR_INIT;

    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    srand(ts.tv_nsec);
    printf("Random seed is %ld\n", ts.tv_nsec);


    int renderDataAllocation = sizeof(struct ParticleRenderData) * maxParticles;
    int particlesAllocation = sizeof(struct Particle) * maxParticles;
    printf("renderData will be allocated %d bytes\n", renderDataAllocation);
    printf("particles will be allocated %d bytes\n", renderDataAllocation);

    fwgl->is_preview = 0;
    fwgl->window, fwgl->stdShaderProgram, fwgl->VAO, fwgl->vertexVBO, fwgl->dataVBO,
    fwgl->EBO = -1;
    fwgl->renderData = malloc(renderDataAllocation);

  struct FWGLSimulation simulation;
  simulation.maxParticles = maxParticles;
  simulation.liveParticles = 0;
  simulation.maxRockets = maxRockets;
  simulation.liveRockets = 0;
  simulation.particles = malloc(particlesAllocation);
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

enum FWGL_Error FWGL_DeInit(struct FWGL *fwgl) {
    printf("Deinitialising FWGL: ");

    printf("Deleting OpenGL resources...  ");
    glDeleteVertexArrays(1, &(fwgl->VAO));
    glDeleteBuffers(1, &(fwgl->vertexVBO));
    glDeleteBuffers(1, &(fwgl->EBO));
    glDeleteProgram(fwgl->stdShaderProgram);
    glDeleteFramebuffers(1, &(fwgl->vfxFBO));

    printf("Freeing memory...  ");
    free(fwgl->renderData);
    free(fwgl->simulation.particles);
    free(fwgl);

    printf("FWGL_DeInit OK\n");
    return FWGL_OK;
}

void FWGL_parseArgs(struct FWGL *fwgl, int argc, char *argv[]) {
  if (argc < 2) {
    printf("Not enough arguments!\n");
    fwgl->error = FWGL_ERROR_INIT_ARGCOUNT;
    return;
  }

  if (strcmp(argv[1], "/s") == 0) {
    fwgl->is_preview = 0;
  } else if (strcmp(argv[1], "/p") == 0) {
    fwgl->is_preview = 1;
  } else {
    printf("Unrecognised argument: %s\n", argv[1]);
    fwgl->error = FWGL_ERROR_INIT_UNKNOWNARG;
  }

  fwgl->error = FWGL_OK;
}

void FWGL_printHelp() { printf(" ~~ Help ~~ \n"); }

void FWGL_createGLFWWindow(struct FWGL *fwgl) {

  GLFWwindow *window = NULL;
  int width = 0;
  int height = 0;
  
  // It's a screensaver, it shouldn't change size
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  if (fwgl->is_preview) {
    printf("Creating preview window\n");
    width = 800;
    height = 600;
    window = glfwCreateWindow(width, height, "FireworksGL", NULL, NULL);
  } else {
    printf("Creating full screen window\n");
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

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

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("Failed to initialize GLAD\n");
    fwgl->error = FWGL_ERROR_INIT_GLAD;
    return;
  }

  glViewport(0, 0, width, height);
  glfwSetFramebufferSizeCallback(window, FWGL_framebufferSizeCallback);

  fwgl->window = window;
  fwgl->error = FWGL_OK;
}

void FWGL_framebufferSizeCallback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void FWGL_process(struct FWGL *fwgl, float dSecs) {
  if (GLFW_PRESS == glfwGetKey(fwgl->window, GLFW_KEY_SPACE) ||
      GLFW_PRESS == glfwGetKey(fwgl->window, GLFW_KEY_ENTER) ||
      GLFW_PRESS == glfwGetMouseButton(fwgl->window, GLFW_MOUSE_BUTTON_LEFT)) {
    printf("Input detected! Triggering close...\n");
    glfwSetWindowShouldClose(fwgl->window, GLFW_TRUE);
  }

  int width;
  int height;
  glfwGetWindowSize(fwgl->window, &width, &height);
  MoveParticles(&(fwgl->simulation), width, height, dSecs);
}

void FWGL_compileShaders(struct FWGL *fwgl) {

  int success;
  char log[512];

  printf("\nStd Vertex Shader:\n%s\n", stdVertexShaderSource);
  printf("\nStd Fragment Shader:\n%s\n", stdFragmentShaderSource);

  unsigned int stdVertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(stdVertexShader, 1, &stdVertexShaderSource, NULL);
  glCompileShader(stdVertexShader);
  glGetShaderiv(stdVertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(stdVertexShader, 512, NULL, log);
    printf("Failed to compile std vertex shader: %s", log);
    fwgl->error = FWGL_ERROR_INIT_COMPILEVERTEX;
    return;
  }

  unsigned int stdFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(stdFragmentShader, 1, &stdFragmentShaderSource, NULL);
  glCompileShader(stdFragmentShader);
  glGetShaderiv(stdFragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(stdFragmentShader, 512, NULL, log);
    printf("Failed to compile std fragment shader: %s", log);
    fwgl->error = FWGL_ERROR_INIT_COMPILEVERTEX;
    return;
  }

  unsigned stdProgram = glCreateProgram();
  fwgl->stdShaderProgram = stdProgram;
  glAttachShader(stdProgram, stdVertexShader);
  glAttachShader(stdProgram, stdFragmentShader);
  glLinkProgram(stdProgram);
  glGetProgramiv(stdProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(stdProgram, 512, NULL, log);
    printf("Failed to link shader program: %s", log);
    fwgl->error = FWGL_ERROR_INIT_SHADERLINK;
    return;
  }

  glDeleteShader(stdVertexShader);
  glDeleteShader(stdFragmentShader);
  printf("Successfully compiled and linked std shader program!\n");

  // Round 2! (Quad)

  printf("\nQuad Vertex Shader:\n%s\n", quadVertexShaderSource);
  printf("\nQuad Fragment Shader:\n%s\n", quadFragmentShaderSource);

  unsigned int quadVertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(quadVertexShader, 1, &quadVertexShaderSource, NULL);
  glCompileShader(quadVertexShader);
  glGetShaderiv(quadVertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
      glGetShaderInfoLog(quadVertexShader, 512, NULL, log);
      printf("Failed to compile quad vertex shader: %s", log);
      fwgl->error = FWGL_ERROR_INIT_COMPILEVERTEX;
      return;
  }

  unsigned int quadFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(quadFragmentShader, 1, &quadFragmentShaderSource, NULL);
  glCompileShader(quadFragmentShader);
  glGetShaderiv(quadFragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
      glGetShaderInfoLog(quadFragmentShader, 512, NULL, log);
      printf("Failed to compile quad fragment shader: %s", log);
      fwgl->error = FWGL_ERROR_INIT_COMPILEVERTEX;
      return;
  }

  unsigned quadProgram = glCreateProgram();
  fwgl->quadShaderProgram = quadProgram;
  glAttachShader(quadProgram, quadVertexShader);
  glAttachShader(quadProgram, quadFragmentShader);
  glLinkProgram(quadProgram);
  glGetProgramiv(quadProgram, GL_LINK_STATUS, &success);
  if (!success) {
      glGetProgramInfoLog(quadProgram, 512, NULL, log);
      printf("Failed to link quad shader program: %s", log);
      fwgl->error = FWGL_ERROR_INIT_SHADERLINK;
      return;
  }

  glDeleteShader(quadVertexShader);
  glDeleteShader(quadFragmentShader);
  printf("Successfully compiled and linked quad shader program!\n");
}

void FWGL_prepareBuffers(struct FWGL *fwgl) {
  // Handles which will be stored in FWGL
  unsigned int vfxFBO, vfxTexture, quadVAO, quadVBO, dimensionUBO, dataVBO, VAO, vertexVBO, EBO;

  int width, height;
  glfwGetWindowSize(fwgl->window, &width, &height);

  // 
  // Visual Effects
  //
  // Texture
  glGenTextures(1, &vfxTexture);
  glBindTexture(GL_TEXTURE_2D, vfxTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // Framebuffer
  glGenFramebuffers(1, &vfxFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, vfxFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, vfxTexture, 0);
  GLenum fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (GL_FRAMEBUFFER_COMPLETE != fbStatus) {
      printf("Erronious framebuffer status: %d\n", fbStatus);
      fwgl->error = FWGL_ERROR_PREPBUFFER_FRAME_RENDER;
      return;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // 2*f Screen Position (x,y)
  // 2*f Texture Coordinates (x,y)
  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

  //
  // Standard rendering
  //

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

  // 2*i Dimensions (w,h)
  glGenBuffers(1, &dimensionUBO);
  glBindBuffer(GL_UNIFORM_BUFFER, dimensionUBO);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, dimensionUBO);
  int defaultDimensions[4] = { 100, 100, 0, 0 }; // Pad to 16 bytes for std140
  glBufferData(GL_UNIFORM_BUFFER, sizeof(defaultDimensions), defaultDimensions, GL_STATIC_DRAW);

  // Vertex attributes
  // Vertex base position (x,y,z)
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  // Translate (x,y,z)
  glBindBuffer(GL_ARRAY_BUFFER, dataVBO);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct ParticleRenderData), (void *)0);
  // Colour (r,g,b,a)
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(struct ParticleRenderData), (void *)(3 * sizeof(float)));
  // Radius (r)
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(struct ParticleRenderData), (void *)(7 * sizeof(float)));
  // Remaining Life (l)
  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(struct ParticleRenderData), (void *)(8 * sizeof(float)));
  // Particle Type (t)
  glEnableVertexAttribArray(5);
  glVertexAttribIPointer(5, 1, GL_INT, sizeof(struct ParticleRenderData), (void *)(9 * sizeof(float)));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glVertexAttribDivisor(1, 1); // Stride of 1 between swapping attributes
  glVertexAttribDivisor(2, 1); // Stride of 1 between swapping attributes
  glVertexAttribDivisor(3, 1); // Stride of 1 between swapping attributes
  glVertexAttribDivisor(4, 1); // Stride of 1 between swapping attributes
  glVertexAttribDivisor(5, 1); // Stride of 1 between swapping attributes

  glBindVertexArray(0);

  fwgl->vfxFBO = vfxFBO;
  fwgl->vfxTexture = vfxTexture;
  fwgl->quadVAO = quadVAO;
  fwgl->VAO = VAO;
  fwgl->dimensionUBO = dimensionUBO;
  fwgl->vertexVBO = vertexVBO;
  fwgl->dataVBO = dataVBO;
  fwgl->EBO = EBO;

  fwgl->error = FWGL_OK;
}

void FWGL_render(struct FWGL *fwgl) {

  struct FWGLSimulation *simulation = &(fwgl->simulation);
  struct Particle *p;

  int renderParticles = 0;
  for (int pId = 0; pId < simulation->maxParticles; pId++) {
    p = &(simulation->particles[pId]);
    if (!p->isAlive) {
      continue;
    }

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

    fwgl->renderData[renderParticles] = data;
    renderParticles++;
  }

  // Need to pad it to 16 bytes for std140 layout
  int dimensions[4] = { 200, 200, 0, 0 };
  glfwGetWindowSize(fwgl->window, &(dimensions[0]), &(dimensions[1]));
  glBindBuffer(GL_UNIFORM_BUFFER, fwgl->dimensionUBO);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(dimensions), &dimensions);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  // Does this need to come before the uniform buffer?
  glBindFramebuffer(GL_FRAMEBUFFER, fwgl->vfxFBO);
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  if (renderParticles > 0) {
    int bufferSize = sizeof(struct ParticleRenderData) * renderParticles;
    glBindBuffer(GL_ARRAY_BUFFER, fwgl->dataVBO);
    glBufferData(GL_ARRAY_BUFFER, bufferSize, fwgl->renderData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    int indexCount = (int)(sizeof(circleIndices) / sizeof(int));

    glUseProgram(fwgl->stdShaderProgram);
    glBindVertexArray(fwgl->VAO);
    glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, renderParticles);
  }

  // Return to the regular framebuffer and render the processed texture as a quad
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // Don't need to clear colours because quad is opaque.
  glUseProgram(fwgl->quadShaderProgram);
  glBindVertexArray(fwgl->quadVAO);
  glBindTexture(GL_TEXTURE_2D, fwgl->vfxTexture);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}
