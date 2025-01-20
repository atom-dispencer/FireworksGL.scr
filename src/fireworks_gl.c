#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "fireworks_gl.h"
#include "fireworks_gl_process.h"
#include "fireworks_gl_shaders.h"

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
    // Screen position      // Texture position
    // Bottom left triangle
    -1,  1,                 0, 1,
    -1, -1,                 0, 0,
     1, -1,                 1, 0,
    // Top right triangle
    -1,  1,                 0, 1,
     1, -1,                 1, 0,
     1,  1,                 1, 1
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

  FWGL_compileShader(&(fwgl->geometryShader), geometryVertexShaderSource, geometryFragmentShaderSource);
  FWGL_compileShader(&(fwgl->screenShader), screenVertexShaderSource, screenFragmentShaderSource);
  FWGL_compileShader(&(fwgl->blurredShader), blurVertexShaderSource, blurFragmentShaderSource);
  FWGL_compileShader(&(fwgl->bloomShader), bloomVertexShaderSource, bloomFragmentShaderSource);

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
    fwgl->window, fwgl->geometryShader, fwgl->circleVAO, fwgl->circleVBO, fwgl->dataVBO,
    fwgl->circleEBO = -1;
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
    glDeleteVertexArrays(1, &(fwgl->circleVAO));
    glDeleteBuffers(1, &(fwgl->circleVBO));
    glDeleteBuffers(1, &(fwgl->circleEBO));
    glDeleteProgram(fwgl->geometryShader);
    glDeleteFramebuffers(1, &(fwgl->geometryFBO));

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

void FWGL_compileShader(unsigned int* program, const char* vertexSource, const char* fragSource) {

    int success;
    char log[512];

    printf("\Vertex Shader:\n%s\n", vertexSource);
    printf("\Fragment Shader:\n%s\n", fragSource);

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, log);
        printf("Failed to compile vertex shader: %s", log);
    }

    unsigned int fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragSource, NULL);
    glCompileShader(fragShader);
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragShader, 512, NULL, log);
        printf("Failed to compile fragment shader: %s", log);
    }

    unsigned quadProgram = glCreateProgram();
    *program = quadProgram;
    glAttachShader(quadProgram, vertexShader);
    glAttachShader(quadProgram, fragShader);
    glLinkProgram(quadProgram);
    glGetProgramiv(quadProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(quadProgram, 512, NULL, log);
        printf("Failed to link quad shader program: %s", log);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);
    printf("Successfully compiled and linked shader program!\n");
}

void FWGL_makeTexture(unsigned int* texture, int width, int height) {

    unsigned int handle;
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    *texture = handle;
}

void FWGL_makeFramebuffer(unsigned int* framebuffer, unsigned int texture) {
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    GLenum fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (GL_FRAMEBUFFER_COMPLETE != fbStatus) {
        printf("Erronious framebuffer status: %d\n", fbStatus);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    *framebuffer = fbo;
}

void FWGL_prepareBuffers(struct FWGL *fwgl) {
  // 
  // Handles
  // 
  // Basic output of the particle geometry (semi-transparent circles on a black background)
  unsigned int dimensionUBO, circleVAO, circleVBO, circleEBO, dataVBO;
  unsigned int geometryFBO, geometryTexture, geometryShader;
  // A blurred version of the geometry
  unsigned int blurredFBO1, blurredTexture1, blurredShader;
  unsigned int blurredFBO2, blurredTexture2;
  // A HDR buffer with the bloom (addition) result of the blur and geometry buffers
  unsigned int bloomFBO, bloomTexture;
  // A tone-remapped FBO to reduce the bloom to the standard 0-1 range.
  unsigned int tonemappedFBO;
  // Tone-mapped buffer gets drawn to the screen
  unsigned int screenShader, screenVAO, screenVBO;

  int width, height;
  glfwGetWindowSize(fwgl->window, &width, &height);

  //
  // Framebuffers
  //
  // Geometry
  FWGL_makeTexture(&geometryTexture, width, height);
  FWGL_makeFramebuffer(&geometryFBO, geometryTexture);
  // Blur
  FWGL_makeTexture(&blurredTexture1, width, height);
  FWGL_makeTexture(&blurredTexture2, width, height);
  FWGL_makeFramebuffer(&blurredFBO1, blurredTexture1);
  FWGL_makeFramebuffer(&blurredFBO2, blurredTexture2);
  // Bloom
  FWGL_makeTexture(&bloomTexture, width, height);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
  FWGL_makeFramebuffer(&bloomFBO, bloomTexture);
  

  // 2*f Screen Position (x,y)
  // 2*f Texture Coordinates (x,y)
  glGenVertexArrays(1, &screenVAO);
  glGenBuffers(1, &screenVBO);
  glBindVertexArray(screenVAO);
  glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
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
  glGenVertexArrays(1, &circleVAO);
  glGenBuffers(1, &circleVBO);
  glGenBuffers(1, &circleEBO);

  glBindVertexArray(circleVAO);
  glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(circleVertices), circleVertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circleEBO);
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

  //
  fwgl->dimensionUBO = dimensionUBO;
  fwgl->circleVAO = circleVAO;
  fwgl->circleVBO = circleVBO;
  fwgl->circleEBO = circleEBO;
  fwgl->dataVBO = dataVBO;
  //
  fwgl->geometryFBO = geometryFBO;
  fwgl->geometryTexture = geometryTexture;
  //
  fwgl->blurredFBO1 = blurredFBO1;
  fwgl->blurredTexture1 = blurredTexture1;
  fwgl->blurredFBO2 = blurredFBO2;
  fwgl->blurredTexture2 = blurredTexture2;
  //
  fwgl->bloomFBO = bloomFBO;
  fwgl->bloomTexture = bloomTexture;
  //
  fwgl->screenVAO = screenVAO;

  fwgl->error = FWGL_OK;
}

void FWGL_render(struct FWGL *fwgl) {


    //
    // Geometry
    //
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
  glBindFramebuffer(GL_FRAMEBUFFER, fwgl->geometryFBO);
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  if (renderParticles > 0) {
    int bufferSize = sizeof(struct ParticleRenderData) * renderParticles;
    glBindBuffer(GL_ARRAY_BUFFER, fwgl->dataVBO);
    glBufferData(GL_ARRAY_BUFFER, bufferSize, fwgl->renderData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    int indexCount = (int)(sizeof(circleIndices) / sizeof(int));

    glUseProgram(fwgl->geometryShader);
    glBindVertexArray(fwgl->circleVAO);
    glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, renderParticles);
  }

  //
  // Blur
  //
  const int BLUR_PASSES_1 = 3;
  unsigned int blurFBOs[] = { fwgl->blurredFBO1, fwgl->blurredFBO2 };
  unsigned int blurTextures[] = { fwgl->blurredTexture1, fwgl->blurredTexture2 };

  glUseProgram(fwgl->blurredShader);
  for (int pass = 0; pass < 2*BLUR_PASSES_1; pass++) {
      int pingpong = pass % 2;

      unsigned int blurDestFBO = blurFBOs[pingpong];
      unsigned int blurSourceTexture = blurTextures[1 - pingpong];

      glBindFramebuffer(GL_FRAMEBUFFER, blurDestFBO);

      unsigned int loc = glGetUniformLocation(fwgl->blurredShader, "horizontal");
      glUniform1i(loc, 0 == pingpong);

      // 0: (Initial condition) Draw from geometry to texture1
      // 1: Draw from texture1 to texture2
      // 2: Draw from texture2 to texture1
      // 3: Draw from texture1 to texture2
      // etc... (alternating)
      if (0 == pass) {
          glBindTexture(GL_TEXTURE_2D, fwgl->geometryTexture);
      }
      else {
          glBindTexture(GL_TEXTURE_2D, blurSourceTexture);
      }

      glBindVertexArray(fwgl->screenVAO);
      glDrawArrays(GL_TRIANGLES, 0, 6);
  }
  
  // Bloom
  glBindFramebuffer(GL_FRAMEBUFFER, fwgl->bloomFBO);
  glUseProgram(fwgl->bloomShader);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, fwgl->blurredTexture2);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, fwgl->geometryTexture);

  glUniform1i(glGetUniformLocation(fwgl->bloomShader, "texture0_screen"), 0);
  glUniform1i(glGetUniformLocation(fwgl->bloomShader, "texture1_blur"), 1);
  glBindVertexArray(fwgl->screenVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  // Blur round 2
  const BLUR_PASSES_2 = 3;
  glUseProgram(fwgl->blurredShader);
  for (int pass = 0; pass < 2 * BLUR_PASSES_2; pass++) {
      int pingpong = pass % 2;

      unsigned int blurDestFBO = blurFBOs[pingpong];
      unsigned int blurSourceTexture = blurTextures[1 - pingpong];

      glBindFramebuffer(GL_FRAMEBUFFER, blurDestFBO);

      unsigned int loc = glGetUniformLocation(fwgl->blurredShader, "horizontal");
      glUniform1i(loc, 0 == pingpong);

      // 0: (Initial condition) Draw from geometry to texture1
      // 1: Draw from texture1 to texture2
      // 2: Draw from texture2 to texture1
      // 3: Draw from texture1 to texture2
      // etc... (alternating)
      if (0 == pass) {
          glBindTexture(GL_TEXTURE_2D, fwgl->bloomTexture);
      }
      else {
          glBindTexture(GL_TEXTURE_2D, blurSourceTexture);
      }

      glBindVertexArray(fwgl->screenVAO);
      glDrawArrays(GL_TRIANGLES, 0, 6);
  }

  //
  // Screen
  // 
  // Return to the regular framebuffer and render the processed texture as a quad
  // Don't need to clear colours because quad is opaque.
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glUseProgram(fwgl->screenShader);
  glBindVertexArray(fwgl->screenVAO);
  glBindTexture(GL_TEXTURE_2D, fwgl->blurredTexture2);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}
