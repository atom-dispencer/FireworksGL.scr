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
"           float factor = remainingLife / 2;                       \n"
"           FragColor.w = vertexColour.w * 0.5 * factor * factor;   \n"
"       }                                                           \n"
"   }                                                               \n"
"\0";

const char* pointVertexShaderSource =
"   #version 330 core                                       \n"
"   layout(location = 0) in vec3 aPosition;                 \n"
"	layout(location = 1) in int particleType;				\n"
"                                                           \n"
"   layout (std140) uniform WindowDimensions {              \n"
"       int width;                                          \n"
"       int height;                                         \n"
"   };                                                      \n"
"                                                           \n"
"   void main()                                             \n"
"   {                                                       \n"
"		// Only applies to rockets                          \n"
"		if (particleType == 1) {							\n"
"			gl_Position = vec4(aPosition, 1.0f);            \n"
"			gl_Position.x /= (width / 2.0f);                \n"
"			gl_Position.y /= (height / 2.0f);               \n"
"			gl_Position += vec4(-1, -1, 0, 0);              \n"
"		}													\n"
"		else {												\n"
"			gl_Position = vec4(-2, -2, -2, 1);				\n"
"		}													\n"
"	}														\n"
"\0";

const char* pointFragmentShaderSource =
"#version 330 core                                      \n"
"out vec4 FragColor;                                    \n"
"                                                       \n"
"in vec2 TexCoords;                                     \n"
"                                                       \n"
"uniform sampler2D screenTexture;                       \n"
"                                                       \n"
"void main()                                            \n"
"{                                                      \n"
"    FragColor = vec4(1,1,1,1);							\n"
"}                                                      \n"
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
//"uniform float weight[3] = float[](2*0.3513, 0.1159, 0.0228);				\n"
//"uniform float weight[5] = float[](2*0.2340, 2*0.1603, 2*0.0753, 2*0.0242, 2*0.0062);				\n"
//"uniform float weight[6] = float[](2*0.1915, 0.1499, 0.0918, 0.0441, 0.0165, 2*0.0062);				\n"
"const int WEIGHTS = 5;																									\n"
"																									\n"
"void main()																						\n"
"{																									\n"
"    vec2 tex_offset = 1.0 / textureSize(image, 0); // gets size of single texel					\n"
"    vec3 result = texture(image, TexCoords).rgb * weight[0]; // current fragment's contribution	\n"
"    if (horizontal)																				\n"
"    {																								\n"
"        for (int i = 1; i < WEIGHTS; ++i)																\n"
"        {																							\n"
"            result += texture(image, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];		\n"
"            result += texture(image, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];		\n"
"        }																							\n"
"    }																								\n"
"    else																							\n"
"    {																								\n"
"        for (int i = 1; i < WEIGHTS; ++i)																\n"
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

const char* bloomVertexShaderSource =
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

const char* bloomFragmentShaderSource =
"#version 330 core												\n"
"out vec4 FragColor;											\n"
"																\n"
"in vec2 TexCoords;												\n"
"																\n"
"uniform sampler2D texture0_screen;								\n"
"uniform sampler2D texture1_blur;								\n"
"																\n"
"void main()													\n"
"{																\n"
"	const float exposure = 2;									\n"
"	const float gamma = 2.2;									\n"
"	vec3 hdrColor = texture(texture0_screen, TexCoords).rgb;	\n"
"	vec3 bloomColor = texture(texture1_blur, TexCoords).rgb;	\n"
"																\n"
"	// Additive blending										\n"
"	hdrColor += bloomColor;										\n"
"																\n"
"	// tone mapping												\n"
"	vec3 result = vec3(1.0) - exp(-hdrColor * exposure);		\n"
"																\n"
"	// also gamma correct while we're at it						\n"
"	result = pow(result, vec3(1.0 / gamma));					\n"
"	FragColor = vec4(result, 1.0);								\n"
"}																\n"
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