#pragma once

//
// Geometry
//

const char* geometryVertexShaderSource =
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

const char* geometryFragmentShaderSource =
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

//
// Blur
//

const char* blurVertexShaderSource =
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

const char* blurFragmentShaderSource =
"#version 330 core																					\n"
"out vec4 FragColor;																				\n"
"																									\n"
"in vec2 TexCoords;																					\n"
"																									\n"
"uniform sampler2D image;																			\n"
"																									\n"
"uniform bool horizontal;																			\n"
"uniform float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);				\n"
"																									\n"
"void main()																						\n"
"{																									\n"
"    vec2 tex_offset = 1.0 / textureSize(image, 0); // gets size of single texel					\n"
"    vec3 result = texture(image, TexCoords).rgb * weight[0]; // current fragment's contribution	\n"
"    if (horizontal)																				\n"
"    {																								\n"
"        for (int i = 1; i < 5; ++i)																\n"
"        {																							\n"
"            result += texture(image, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];		\n"
"            result += texture(image, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];		\n"
"        }																							\n"
"    }																								\n"
"    else																							\n"
"    {																								\n"
"        for (int i = 1; i < 5; ++i)																\n"
"        {																							\n"
"            result += texture(image, TexCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];		\n"
"            result += texture(image, TexCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];		\n"
"        }																							\n"
"    }																								\n"
"    FragColor = vec4(result, 1.0);																	\n"
"}																									\n"
"\0";

//
// Screen
//

const char* screenVertexShaderSource =
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

const char* screenFragmentShaderSource =
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