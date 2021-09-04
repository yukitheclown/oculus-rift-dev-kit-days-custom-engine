#define GLEW_STATIC
#include <GL/glew.h>
#include <stdio.h>
#include <string.h>
#include "shaders.h"

static float cameraClipMatrix[16];
static float viewMatrix[16];
static float projMatrix[16];
static float modelMatrix[16];
static float invViewMatrix[16];
static float invTransMatrix[9];
static float depthMvpMatrix[16];
static GLuint activeProgram = -1;
static Vector3 lightInvDir = { 0, 0, -1 };

static struct Shader {
	GLuint program;
	GLuint fShader;
	GLuint vShader;
	GLuint modelLoc;
	GLuint projLoc;
	GLuint viewLoc;
	GLuint mInvTransLoc;
	GLuint useShadowsLoc;
	GLuint useColorAttribLoc;
	GLuint depthMvpLoc;
	GLuint clipMatrixLoc;
	GLuint invViewLoc;
	GLuint camRightLoc;
	GLuint camUpLoc;
	GLuint uniColorLoc;
	GLuint rgbSplitAmountLoc;
	GLuint reflectionAmountLoc;
	GLuint blurAmountLoc;
	GLuint glowAmountLoc;
	GLuint lightInvDirLoc;
	GLuint bonesLoc;
} shaders[TEXTURED_2D_SHADER + 1];

static void CreateShader(struct Shader *shader, const char *vSource, const char *fSource){
	shader->program = glCreateProgram();
	shader->fShader = glCreateShader(GL_FRAGMENT_SHADER);
	shader->vShader = glCreateShader(GL_VERTEX_SHADER);
	const GLchar*   glSrc = fSource;
	GLint           status;
	char            buffer[512];

	glShaderSource(shader->fShader, 1, &glSrc, NULL);
	glCompileShader(shader->fShader);
	glGetShaderiv(shader->fShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE){
		glGetShaderInfoLog(shader->fShader, 512, NULL, buffer);
		printf("FSHADER: %s\n", buffer);
		return;
	}

	glAttachShader(shader->program, shader->fShader);

	glSrc = vSource;
	glShaderSource(shader->vShader, 1, &glSrc, NULL);
	glCompileShader(shader->vShader);
	glGetShaderiv(shader->vShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE){
		glGetShaderInfoLog(shader->vShader, 512, NULL, buffer);
		printf("VSHADER: %s\n", buffer);
		return;
	}

	glAttachShader(shader->program, shader->vShader);

	glLinkProgram(shader->program);
	glUseProgram(shader->program);

	shader->lightInvDirLoc = glGetUniformLocation(shader->program, "invLightDir");
	shader->modelLoc = glGetUniformLocation(shader->program, "model");
	shader->projLoc = glGetUniformLocation(shader->program, "projection");
	shader->viewLoc = glGetUniformLocation(shader->program, "view");
	shader->mInvTransLoc = glGetUniformLocation(shader->program, "invTrans");
	shader->depthMvpLoc = glGetUniformLocation(shader->program, "depthMvp");
	shader->useShadowsLoc = glGetUniformLocation(shader->program, "useShadows");
	shader->useColorAttribLoc = glGetUniformLocation(shader->program, "useColorAttrib");
	shader->invViewLoc = glGetUniformLocation(shader->program, "invView");
	shader->camRightLoc = glGetUniformLocation(shader->program, "cameraRight");
	shader->camUpLoc = glGetUniformLocation(shader->program, "cameraUp");
	shader->uniColorLoc = glGetUniformLocation(shader->program, "uniColor");
	shader->clipMatrixLoc = glGetUniformLocation(shader->program, "clipMatrix");
	shader->rgbSplitAmountLoc = glGetUniformLocation(shader->program, "rgbSplitAmount");
	shader->blurAmountLoc = glGetUniformLocation(shader->program, "blurAmount");
	shader->reflectionAmountLoc = glGetUniformLocation(shader->program, "reflectionAmount");
	shader->glowAmountLoc = glGetUniformLocation(shader->program, "glowAmount");
	shader->bonesLoc = glGetUniformLocation(shader->program, "bones");
	glUniform1i(glGetUniformLocation(shader->program, "shadowMap"), 1);
	glUniform1i(glGetUniformLocation(shader->program, "colorMap"), 2);
	glUniform1i(glGetUniformLocation(shader->program, "depthMap"), 3);
	glUniform1i(glGetUniformLocation(shader->program, "positionMap"), 4);
	glUniform1i(glGetUniformLocation(shader->program, "normalMap"), 5);
	glUniform1i(glGetUniformLocation(shader->program, "fragAttribMap"), 6);
}

static const char *textureless2DFSource = "#version 130\n"
"uniform vec4 uniColor = vec4(1,1,1,1);\n"
"in vec4 Color;\n"
"void main(){\n"
"gl_FragColor = uniColor * Color;"
"}\n"
"\n";

static const char *textureless2DVSource = "#version 130\n"
"in vec2 pos;\n"
"in vec2 coord;\n"
"in vec4 color;\n"
"out vec4 Color;\n"
"uniform mat4 projection;\n"
"void main(){\n"
"gl_Position = projection * vec4(pos,1,1);\n"
"Color = color;"
"}\n";

static const char *textured2DFSource = "#version 130\n"
"uniform vec4 uniColor = vec4(1,1,1,1);\n"
"uniform sampler2D tex;\n"
"in vec2 TexCoord;\n"
"void main(){\n"
"vec4 t = texture2D(tex, TexCoord);\n"
"if(t.a == 0) discard;"
"gl_FragColor = t * uniColor;"
"}\n"
"\n";

static const char *textured2DVSource = "#version 130\n"
"in vec2 pos;\n"
"in vec2 coord;\n"
"in vec4 color;\n"
"uniform mat4 projection;\n"
"out vec2 TexCoord;\n"
"void main(){\n"
"gl_Position = projection * vec4(pos,1,1);\n"
"TexCoord = coord;"
"}\n";

static const char *text2DFSource = "#version 130\n"
"in vec4 colorFromVertShader;\n"
"uniform sampler2D tex;\n"
"uniform vec4 uniColor;\n"
"in vec2 TexCoord;\n"
"void main(){\n"
"vec4 t = texture2D(tex, TexCoord);\n"
"if(t.a < 0.1) discard;\n"
"gl_FragColor = vec4(uniColor.x, uniColor.y, uniColor.z, t.a) * colorFromVertShader;\n"
"}\n";

static const char *text2DVSource = "#version 130\n"
"in vec3 pos;\n"
"in vec2 coord;\n"
"in vec4 color;\n"
"uniform mat4 projection, view, model;\n"
"out vec2 TexCoord;\n"
"out vec4 colorFromVertShader;\n"
"void main(){\n"
"vec3 Position_CamSpace = (projection * view * model * vec4(pos, 1)).xyz;\n"
"gl_Position = vec4(Position_CamSpace,1);\n"
"colorFromVertShader = color;\n"
"TexCoord = coord;"
"}\n";

static const char *text3DFSource = "#version 130\n"
"in vec4 colorFromVertShader;\n"
"uniform vec4 uniColor = vec4(0,0,0,1);\n"
"uniform sampler2D tex;\n"
"in vec2 TexCoord;\n"
"void main(){\n"
"vec4 t = texture2D(tex, TexCoord);\n"
"if(t.a == 0) discard;\n"
"gl_FragColor = vec4(uniColor.x, uniColor.y, uniColor.z, t.a) * colorFromVertShader;\n"
"}\n";

static const char *text3DVSource = "#version 130\n"
"in vec2 pos;\n"
"in vec2 coord;\n"
"in vec2 billboardSize;\n"
"in vec3 billboardCenter;\n"
"in vec4 color;\n"
"out vec2 TexCoord;\n"
"out vec4 colorFromVShader;\n"
"uniform mat4 model = mat4(1);\n"
"uniform mat4 invView;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main(){\n"
"vec3 cameraRight = vec3(invView[0][0], invView[1][0], invView[2][0]);\n"
"vec3 cameraUp = vec3(invView[0][1], invView[1][1], invView[2][1]);\n"
"vec3 right = vec3(\n"
"cameraRight.x * pos.x * billboardSize.x, \n"
"cameraRight.y * pos.x * billboardSize.x,\n"
"cameraRight.z * pos.x * billboardSize.x);\n"
"vec3 up = vec3(\n"
"cameraUp.x * pos.y * billboardSize.y, \n"
"cameraUp.y * pos.y * billboardSize.y, \n"
"cameraUp.z * pos.y * billboardSize.y);\n"
"vec3 newpos;\n"
"newpos.x = billboardCenter.x + right.x + up.x;\n"
"newpos.y = billboardCenter.y + right.y + up.y;\n"
"newpos.z = billboardCenter.z + right.z + up.z;\n"
"TexCoord = coord;\n"
"gl_Position = projection * view * model * vec4(newpos,1);\n"
"colorFromVShader = color;\n"
"}";

static const char *texturelessVSource = "#version 130\n"
"in vec3 pos;\n"
"in vec3 norm;\n"
"in vec4 color;\n"
"out vec4 Position;\n"
"out vec3 Normal_Worldspace;\n"
"out vec3 Normal_CamSpace;\n"
"out vec3 Position_CamSpace;\n"
"out vec4 Color;\n"
"uniform mat4 depthMvp = mat4(1);\n"
"out vec4 ShadowCoord;\n"
"uniform mat4 model = mat4(1);\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"uniform mat3 invTrans = mat3(1);\n"
"mat4 bias = mat4(0.5, 0, 0, 0,\n"
"0, 0.5, 0, 0,\n"
"0, 0, 0.5, 0,\n"
"0.5, 0.5, 0.5, 1);\n"
"void main(){\n"
"Color = color;\n"
"Normal_Worldspace = (invTrans * norm).xyz;\n"
"Normal_CamSpace = vec3(normalize(view * vec4(Normal_Worldspace,0)));\n"
"Position = model * vec4(pos,1)\n;"
"Position_CamSpace = (view * Position).xyz;\n"
"gl_Position = projection * vec4(Position_CamSpace, 1);\n"
"ShadowCoord = bias * depthMvp * Position;\n"
"}";

static const char *texturelessFSource = "#version 130\n"
"in vec4 Position;\n"
"in vec3 Normal_Worldspace;\n"
"in vec3 Normal_CamSpace;\n"
"in vec3 Position_CamSpace;\n"
"in vec4 Color;\n"
"in vec4 ShadowCoord;\n"
"uniform sampler2DShadow shadowMap;\n"
"uniform vec4 uniColor = vec4(1,1,1,1);\n"
"uniform bool useShadows = false;\n"
"uniform bool useColorAttrib = false;\n"
"uniform vec3 lightPos = vec3(0,0,0);\n"
"uniform float blurAmount = 0.0;\n"
"uniform float reflectionAmount = 0.0;\n"
"uniform float glowAmount = 0.0;\n"

// "vec2 poissonDisk[4] = vec2[](\n"
//     "vec2( -0.94201624, -0.39906216 ),\n"
//     "vec2( 0.94558609, -0.76890725 ),\n"
//     "vec2( -0.094184101, -0.92938870 ),\n"
//     "vec2( 0.34495938, 0.29387760 )\n"
// ");\n"

"vec2 poissonDisk[16] = vec2[](\n"
"vec2( -0.94201624, -0.39906216 ),\n"
"vec2( 0.94558609, -0.76890725 ),\n"
"vec2( -0.094184101, -0.92938870 ),\n"
"vec2( 0.34495938, 0.29387760 ),\n"
"vec2( -0.91588581, 0.45771432 ),\n"
"vec2( -0.81544232, -0.87912464 ),\n"
"vec2( -0.38277543, 0.27676845 ),\n"
"vec2( 0.97484398, 0.75648379 ),\n"
"vec2( 0.44323325, -0.97511554 ),\n"
"vec2( 0.53742981, -0.47373420 ),\n"
"vec2( -0.26496911, -0.41893023 ),\n"
"vec2( 0.79197514, 0.19090188 ),\n"
"vec2( -0.24188840, 0.99706507 ),\n"
"vec2( -0.81409955, 0.91437590 ),\n"
"vec2( 0.19984126, 0.78641367 ),\n"
"vec2( 0.14383161, -0.14100790 )\n"
"); \n"

"void main(){\n"

"if(useShadows){\n"
"vec4 visibility = vec4(1,1,1,1);\n"
"vec4 sCoord = ShadowCoord;"

"for(int k = 0; k < 16; k++){\n"
"visibility -= 0.05*(1.0-shadow2D(shadowMap,\n"
"vec3((sCoord.xy + poissonDisk[k]/100) / sCoord.w, (sCoord.z-0.005)/sCoord.w)).r);\n"
"}\n"

// "visibility.xyz -= 0.2*(1.0-shadow2D(shadowMap, vec3(sCoord.xy / sCoord.w, (sCoord.z-0.005)/sCoord.w)).r);\n"

"if(useColorAttrib)\n"
"gl_FragData[0] = uniColor * Color * visibility;\n"
"else\n"
"gl_FragData[0] = uniColor * visibility;\n"

"} else { if(useColorAttrib) gl_FragData[0] = uniColor * Color; else gl_FragData[0] = uniColor; }\n"

"gl_FragData[2] = vec4(Position_CamSpace,1);\n"
"gl_FragData[1] = vec4(normalize(Normal_CamSpace),1);\n"
"gl_FragData[3] = vec4(blurAmount, reflectionAmount, glowAmount, 1);\n"
"}";

static const char *texturedVSource = "#version 130\n"
"in vec3 pos;\n"
"in vec2 coord;\n"
"in vec3 norm;\n"
"out vec2 TexCoord;\n"
"out vec3 Normal_Worldspace;\n"
"out vec3 Normal_CamSpace;\n"
"out vec4 Position;\n"
"out vec3 Position_CamSpace;\n"
"uniform mat4 depthMvp = mat4(1);\n"
"out vec4 ShadowCoord;\n"
"uniform mat4 model = mat4(1);\n"
"uniform mat4 view, projection;\n"
"uniform mat3 invTrans = mat3(1);\n"
"mat4 bias = mat4(0.5, 0, 0, 0,\n"
"0, 0.5, 0, 0,\n"
"0, 0, 0.5, 0,\n"
"0.5, 0.5, 0.5, 1);\n"
"void main(){\n"
"Normal_Worldspace = (invTrans * norm).xyz;\n"
"Normal_CamSpace = vec3(normalize(view * vec4(Normal_Worldspace,0)));\n"
"Position = model * vec4(pos,1);\n"
"Position_CamSpace = (view * Position).xyz;\n"
"TexCoord = coord;\n"
"gl_Position = projection * vec4(Position_CamSpace, 1);\n"
"ShadowCoord = bias * depthMvp * Position;\n"
"}";

static const char *hardShadowsFSource = "#version 130\n"
"in vec2 TexCoord;\n"
"in vec4 ShadowCoord;\n"
"in vec3 Normal_Worldspace;\n"
"in vec3 Normal_CamSpace;\n"
"in vec4 Position;\n"
"in vec3 Position_CamSpace;\n"
"uniform bool useShadows = false;\n"
"uniform sampler2D tex;\n"
"uniform sampler2DShadow shadowMap;\n"
"uniform vec4 uniColor = vec4(0,0,0,1);\n"
"uniform vec3 invLightDir;\n"
"uniform mat4 view, projection;\n"
"uniform float blurAmount = 0.0;\n"
"uniform float reflectionAmount = 0.0;\n"
"uniform float glowAmount = 0.0;\n"

// "vec2 poissonDisk[4] = vec2[](\n"
//     "vec2( -0.94201624, -0.39906216 ),\n"
//     "vec2( 0.94558609, -0.76890725 ),\n"
//     "vec2( -0.094184101, -0.92938870 ),\n"
//     "vec2( 0.34495938, 0.29387760 )\n"
// ");\n"

"vec2 poissonDisk[16] = vec2[](\n"
"vec2( -0.94201624, -0.39906216 ),\n"
"vec2( 0.94558609, -0.76890725 ),\n"
"vec2( -0.094184101, -0.92938870 ),\n"
"vec2( 0.34495938, 0.29387760 ),\n"
"vec2( -0.91588581, 0.45771432 ),\n"
"vec2( -0.81544232, -0.87912464 ),\n"
"vec2( -0.38277543, 0.27676845 ),\n"
"vec2( 0.97484398, 0.75648379 ),\n"
"vec2( 0.44323325, -0.97511554 ),\n"
"vec2( 0.53742981, -0.47373420 ),\n"
"vec2( -0.26496911, -0.41893023 ),\n"
"vec2( 0.79197514, 0.19090188 ),\n"
"vec2( -0.24188840, 0.99706507 ),\n"
"vec2( -0.81409955, 0.91437590 ),\n"
"vec2( 0.19984126, 0.78641367 ),\n"
"vec2( 0.14383161, -0.14100790 )\n"
"); \n"

"void main(){\n"
"if(useShadows){\n"
"vec4 visibility = vec4(1,1,1,1);\n"
"vec4 sCoord = ShadowCoord;"
"float cosTheta = dot(normalize(invLightDir  * -1), normalize(Normal_Worldspace));\n"
"float bias = 0.0025*tan(acos(clamp(cosTheta, 0, 1)));\n"
"bias = clamp(bias, 0, 0.0005);\n"

"float sX = sCoord.x/sCoord.w;\n"
"float sY = sCoord.y/sCoord.w;\n"

"if(sX >= 0 && sX <= 1 && sY >= 0 && sY <= 1){\n"
// "if(sCoord.z > bias){\n"

"for(int k = 0; k < 16; k++){\n"
"visibility.xyz -= 0.01 * (1.0-shadow2D(shadowMap,\n"
"vec3((sCoord.xy + (poissonDisk[k]/150.0)) / sCoord.w, (sCoord.z-bias)/sCoord.w)).r);\n"
"}\n"

"} else {\n"

"visibility.xyz -= 0.01*16;\n"
"}\n"

"gl_FragData[0] = visibility * (uniColor + (uniColor.a*texture(tex, TexCoord.xy)));\n"

"} else { gl_FragData[0] = vec4(uniColor.xyz, 1) + (uniColor.a*texture(tex, TexCoord.xy)); }\n"

"gl_FragData[2] = vec4(Position_CamSpace,1);\n"
"gl_FragData[1] = vec4(normalize(Normal_CamSpace),1);\n"
"gl_FragData[3] = vec4(blurAmount, reflectionAmount, glowAmount, 1);\n"
"}";

static const char *postProcessingVSource = "#version 130\n"
"in vec2 pos;\n"
"in vec2 coord;\n"
"out vec2 TexCoord;\n"
"uniform mat4 projection;\n"
"void main(){\n"
"TexCoord = coord;\n"
"gl_Position = projection * vec4(pos, 1, 1);\n"
"}";

static const char *postProcessingFSource = "#version 130\n"
"in vec2 TexCoord;\n"
"uniform sampler2D colorMap;\n"
"uniform sampler2D positionMap;\n"
"uniform sampler2D normalMap;\n"
"uniform sampler2D depthMap;\n"
"uniform sampler2D fragAttribMap;\n"
"uniform mat4 clipMatrix;\n"
"uniform float rgbSplitAmount = 0.0;\n"
"uniform vec4 uniColor = vec4(1,1,1,1);\n"

"vec3 cameraSpaceToScreenSpace(in vec3 camSpace){\n"
"vec4 clipSpace = clipMatrix * vec4(camSpace, 1);\n"
"return 0.5 * (clipSpace.xyz / clipSpace.w) + 0.5;\n"
"}\n"

"vec4 GetReflectionColor(){\n"
"vec3 cameraSpacePos = texture(positionMap, TexCoord.xy).xyz;"
"vec3 cameraSpaceNorm = texture(normalMap, TexCoord.xy).xyz;"

"vec3 viewDir = normalize(cameraSpacePos);\n"
"vec3 viewReflect = normalize(reflect(viewDir, cameraSpaceNorm));\n"
"vec3 screenSpacePos = cameraSpaceToScreenSpace(cameraSpacePos);\n"
"vec3 screenSpaceVectorPosition = cameraSpaceToScreenSpace(cameraSpacePos + viewReflect);\n"
// precompute when the ray will parch off the screen it's mostly just ceil(vec3(1,1,1) - rayPos) / screenReflect)
// for the >1 case and ceil(vec3(0,0,0) - raypos / screenreflect) for the <0 cast.
// Instead of 0.01 use vec3(1.0 / depthBufferWidth, 1.0 / depthBufferHeight, 0.01)
// Another way to speed up is to take larger steps and then when u find an intersection to do a binary refinement. blur the result reflection.
"vec3 screenReflect = 0.005*normalize(screenSpaceVectorPosition.xyz - screenSpacePos.xyz);\n"
// "vec3 screenReflect = vec3(8.0/(1920.0/2.0),8.0/(1080.0/2.0), 0.01)*normalize(screenSpaceVectorPosition.xyz - screenSpacePos.xyz);\n"
"vec3 oldRayPos = screenSpacePos.xyz;\n"
"vec3 rayPos = oldRayPos + (screenReflect * 3.0);\n"
"int numRefinements = 0;\n"

"int k = 0;"
"for(k = 0; k < 200; k++){\n"
"if(rayPos.x > 1 || rayPos.x < 0 || rayPos.y > 1 || rayPos.y < 0 || rayPos.z > 1 || rayPos.z < 0){ k = 200; numRefinements = 3; }\n"

"float depth = texture(depthMap, rayPos.xy).r;\n"

"if(depth <= rayPos.z && rayPos.z - depth < length(screenReflect)){\n"
"screenReflect *= 0.4;\n"
"rayPos = oldRayPos;\n"
"numRefinements++;\n"
"}\n"

"if(numRefinements >= 3) break;\n"

"oldRayPos = rayPos;\n"
"rayPos += screenReflect;\n"
"}\n"

"if(k < 200) return texture(colorMap, rayPos.xy);\n"

"return texture(colorMap, TexCoord.xy);\n"
"}\n"

"void main(){\n"

"vec4 color = texture(colorMap, TexCoord.xy);\n"
"float reflectionAmount = texture(fragAttribMap, TexCoord.xy).g;\n"

"if(reflectionAmount > 0) color = (color*(1-reflectionAmount)) + (GetReflectionColor()*reflectionAmount);\n"

"if(rgbSplitAmount != 0){\n"
"color.r /= 2;\n"
"color.b /= 2;\n"
"color.r += texture(colorMap, TexCoord + vec2(rgbSplitAmount, 0)).r/2;\n"
"color.b += texture(colorMap, TexCoord + vec2(-rgbSplitAmount, 0)).b/2;\n"
"}\n"

// "float glowAmount = texture(fragAttribMap, TexCoord).b/5;\n"
// "glowAmount += texture(fragAttribMap, vec2(TexCoord.x+(0.01), TexCoord.y)).b/5;\n"
// "glowAmount += texture(fragAttribMap, vec2(TexCoord.x-(0.01), TexCoord.y)).b/5;\n"
// "glowAmount += texture(fragAttribMap, vec2(TexCoord.x, TexCoord.y+(0.01))).b/5;\n"
// "glowAmount += texture(fragAttribMap, vec2(TexCoord.x, TexCoord.y-(0.01))).b/5;\n"
// "color += glowAmount;\n"

"gl_FragColor = uniColor * color;\n"
"}";

static const char *softShadowsFSource = "#version 130\n"
"#define LIGHT_FRUSTUM_WIDTH 1.75\n"
"#define LIGHT_WORLD_SIZE 0.5\n"
"#define NEAR_PLANE 0.1\n"
"#define PCF_NUM_SAMPLES 16\n"
"#define BLOCKER_SEARCH_NUM_SAMPLES 16\n"

"uniform bool useShadows = true;\n"
"uniform sampler2D tex;\n"
"uniform sampler2DShadow shadowMap;\n"
"uniform vec4 uniColor = vec4(1,1,1,1);\n"
"uniform vec3 invLightDir;\n"
"uniform mat4 view, projection;\n"
"uniform float blurAmount = 0.0;\n"
"uniform float reflectionAmount = 0.0;\n"
"uniform float glowAmount = 0.0;\n"

"in vec2 TexCoord;\n"
"in vec4 ShadowCoord;\n"
"in vec3 Normal_Worldspace;\n"
"in vec3 Normal_CamSpace;\n"
"in vec4 Position;\n"
"in vec3 Position_CamSpace;\n"

"vec2 poissonDisk[16] = vec2[](\n"
"vec2( -0.94201624, -0.39906216 ),\n"
"vec2( 0.94558609, -0.76890725 ),\n"
"vec2( -0.094184101, -0.92938870 ),\n"
"vec2( 0.34495938, 0.29387760 ),\n"
"vec2( -0.91588581, 0.45771432 ),\n"
"vec2( -0.81544232, -0.87912464 ),\n"
"vec2( -0.38277543, 0.27676845 ),\n"
"vec2( 0.97484398, 0.75648379 ),\n"
"vec2( 0.44323325, -0.97511554 ),\n"
"vec2( 0.53742981, -0.47373420 ),\n"
"vec2( -0.26496911, -0.41893023 ),\n"
"vec2( 0.79197514, 0.19090188 ),\n"
"vec2( -0.24188840, 0.99706507 ),\n"
"vec2( -0.81409955, 0.91437590 ),\n"
"vec2( 0.19984126, 0.78641367 ),\n"
"vec2( 0.14383161, -0.14100790 )\n"
"); \n"

"float PenumbraSize(float zReceiver, float zBlocker){\n"
"return (zReceiver - zBlocker) / zReceiver;\n"
"}\n"

"void FindBlocker(out float avgBlockerDepth, inout float numBlockers, vec2 uv, float zReceiver){\n"
"float searchWidth = (LIGHT_WORLD_SIZE / LIGHT_FRUSTUM_WIDTH) * (zReceiver - NEAR_PLANE) / zReceiver;\n"
"float blockerSum = 0;\n"
"numBlockers = 0;\n"
"for(int k = 0; k < BLOCKER_SEARCH_NUM_SAMPLES; k++){\n"
"float shadowMapDepth = shadow2D(shadowMap, vec3((uv + poissonDisk[k] * searchWidth), zReceiver)).r;\n"
"if(shadowMapDepth < zReceiver){\n"
"numBlockers++;\n"
"blockerSum += shadowMapDepth;\n"
"}\n"
"}\n"

"avgBlockerDepth = blockerSum / numBlockers;\n"
"}\n"

"float PCF_Filter(vec2 uv, float zReceiver, float filterRadiusUV){\n"
"float sum = 0.0f;\n"
"for(int k = 0; k < PCF_NUM_SAMPLES; k++){\n"
"vec2 offset = (poissonDisk[k]/5) * filterRadiusUV;\n"
"sum += (1.0-shadow2D(shadowMap, vec3(uv + offset, zReceiver)).r);\n"
"}\n"

"return sum / PCF_NUM_SAMPLES;\n"
"}\n"

"void main(){\n"
"vec4 visibility = vec4(1,1,1,1);\n"
"if(useShadows){\n"
"vec4 sCoord = ShadowCoord;"

"float cosTheta = dot(normalize(invLightDir), normalize(Normal_Worldspace));\n"
"float bias = 0.0025*tan(acos(clamp(cosTheta, 0, 1)));\n"
"bias = clamp(bias, 0, 0.0005);\n"

"if(sCoord.z > bias){\n"
"float zReceiver = (sCoord.z-bias)/sCoord.w;\n"
"float avgBlockerDepth = 0;\n"
"float numBlockers = 0;\n"
"FindBlocker(avgBlockerDepth, numBlockers, sCoord.xy/sCoord.w, zReceiver);\n"
"if(numBlockers != 0){ \n"
"float penumbraRatio = PenumbraSize(zReceiver, avgBlockerDepth);\n"
"float filterRadiusUV = penumbraRatio * (LIGHT_WORLD_SIZE / LIGHT_FRUSTUM_WIDTH) * NEAR_PLANE / zReceiver;\n"
"visibility.xyz -= 0.3*PCF_Filter(sCoord.xy/sCoord.w, zReceiver, filterRadiusUV);\n"
"}\n"
"} else {\n"
"visibility.xyz -= 0.3;\n"
"}\n"
"}\n"
"gl_FragData[0] = visibility * (vec4(uniColor.xyz, 1) + vec4(uniColor.a*texture(tex, TexCoord.xy).xyz,1));\n"
"gl_FragDepth = gl_FragCoord.z;\n"
"gl_FragData[2] = vec4(Position_CamSpace,1);\n"
"gl_FragData[1] = vec4(normalize(Normal_CamSpace),1);\n"
"gl_FragData[3] = vec4(blurAmount, reflectionAmount, glowAmount, 1);\n"
"}";

static const char *skeletalVSource = "#version 130\n"
"in vec3 pos;\n"
"in vec2 coord;\n"
"in vec3 norm;\n"
"out vec2 TexCoord;\n"
"out vec4 Position;\n"
"out vec3 Normal_Worldspace;\n"
"in vec3 boneIndices;\n"
"in vec3 boneWeights;\n"
"uniform mat4 depthMvp = mat4(1);\n"
"uniform mat4 model = mat4(1);\n"
"uniform mat4 view;\n"
"uniform mat3 invTrans = mat3(1);\n"
"uniform mat4 projection;\n"
"uniform mat4 bones[100];\n"
"out vec4 ShadowCoord;\n"
"mat4 bias = mat4(0.5, 0, 0, 0,\n"
"0, 0.5, 0, 0,\n"
"0, 0, 0.5, 0,\n"
"0.5, 0.5, 0.5, 1);\n"
"void main(){\n"
"mat4 matTranform  = bones[int(boneIndices.x)] * boneWeights.x;\n"
"matTranform      += bones[int(boneIndices.y)] * boneWeights.y;\n"
"matTranform      += bones[int(boneIndices.z)] * boneWeights.z;\n"
"vec4 newpos = model * matTranform * vec4(pos.xyz,1);\n"
"TexCoord = coord;\n"
"gl_Position = projection * view * newpos;\n"
"Position = vec4(model * vec4(pos, 1));\n"
"Normal_Worldspace = ((matTranform * mat4(invTrans)) * vec4(norm, 0)).xyz;\n"
// "Normal_Worldspace = (invTrans * norm).xyz;\n"
"ShadowCoord = (bias * depthMvp) * newpos;\n"
"}";

static const char *particleVSource = "#version 130\n"
"in vec2 pos;\n"
"in vec2 billboardSize;\n"
"in vec3 billboardCenter;\n"
"in vec4 color;\n"
"out vec2 TexCoord;\n"
"out vec4 colorFromVShader;\n"
// "uniform vec3 cameraRight;\n"
// "uniform vec3 cameraUp;\n"
"uniform vec3 rotateAxis = vec3(1,1,1);\n"
"uniform mat4 model = mat4(1);\n"
"uniform mat4 view, projection;\n"
"void main(){\n"
"vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);\n"
"vec3 cameraUp    = vec3(view[0][1], view[1][1], view[2][1]);\n"

"vec3 right = vec3(cameraRight * pos.x * billboardSize.x);\n"
"vec3 up = vec3(cameraUp * pos.y * billboardSize.y);\n"

"vec3 newpos = vec3(0,0,0);\n"
"if(rotateAxis.x != 0)\n"
"newpos.x = billboardCenter.x + right.x + up.x;\n"
"else\n"
"newpos.x = billboardCenter.x + (pos.x * billboardSize.x);\n"

"if(rotateAxis.y != 0)\n"
"newpos.y = billboardCenter.y + right.y + up.y;\n"
"else\n"
"newpos.y = billboardCenter.y + (pos.y * billboardSize.y);\n"

"if(rotateAxis.z != 0)\n"
"newpos.z = billboardCenter.z + right.z + up.z;\n"
"else\n"
"newpos.z = billboardCenter.z;\n"

"TexCoord = pos + 0.5;\n"
"gl_Position = projection * view * model * vec4(newpos,1);\n"
"colorFromVShader = color;\n"
"}";

static const char *particleFSource = "#version 130\n"
"in vec2 TexCoord;\n"
"uniform sampler2D tex;\n"
"uniform vec4 uniColor = vec4(1,1,1,1);\n"
"in vec4 colorFromVShader;\n"
"void main(){\n"
"gl_FragData[0] = uniColor * colorFromVShader * texture(tex, TexCoord);\n"
"}";

static const char *waterVSource = "#version 130\n"
"in vec3 pos;\n"
"in vec2 coord;\n"
"in vec3 norm;\n"
"out vec2 TexCoord;\n"
"out vec3 Normal_Worldspace;\n"
"out vec4 Position;\n"
"uniform mat3 invTrans = mat3(1);"
"uniform mat4 model = mat4(1);\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main(){\n"
"TexCoord = coord;\n"
"gl_Position = projection * view * model * vec4(pos,1);\n"
"Position = vec4(model * vec4(pos, 1));\n"
"Normal_Worldspace = (invTrans * norm).xyz;\n"
"}";

static const char *waterFSource = "#version 130\n"
"in vec2 TexCoord;\n"
"in vec3 Normal_Worldspace;\n"
"in vec4 Position;\n"
"uniform float time;\n"
"uniform mat4 invView;\n"
"uniform sampler2D tex;\n"
"uniform vec3 lightPos = vec3(-10,-2,-5);\n"
"uniform mat4 model, view, projection;\n"
"uniform vec4 uniColor = vec4(0.1,0.1,0.15,0.3);\n"
"void main(){\n"

"vec3 viewDirection = normalize(vec3(invView * vec4(0,0,0,1) - Position));\n"
"vec3 lightDir = normalize(lightPos - Position.xyz);\n"

// "float attenuation = 1.0 / length(lightPos - Position.xyz);\n"
"float attenuation = 1.0;\n"

"vec4 t;\n"
"if(1-TexCoord.y < time)"
"t = texture2D(tex, vec2(TexCoord.x, 1-((1-TexCoord.y) + (1-time))));\n"
"else\n"
"t = texture2D(tex, vec2(TexCoord.x, TexCoord.y+time));\n"

"vec3 normal = normalize(normalize(vec3((t.x*2)-1, (t.y*2)-1, (t.z*2)-1)) * normalize(Normal_Worldspace));\n"

"float cosTheta = attenuation * max(0, dot(normal, lightDir));\n"

"vec3 specular = vec3(0, 0, 0);\n"
"if(dot(normal, lightDir) >= 0) specular = vec3(0.1, 0.1, 0.1) * attenuation * pow(max(0, dot(reflect(-lightDir, normal), viewDirection)), 5);\n"

"float c = 0.1 * cosTheta;\n"
"gl_FragData[0] = vec4((specular + vec3(c, c, c)) + uniColor.xyz, uniColor.w);\n"
"}";

static struct {
	const char **vSource;
	const char **fSource;
} sources[TEXTURED_2D_SHADER + 1] = {
	{ &texturelessVSource, &texturelessFSource },
	{ &texturedVSource, &hardShadowsFSource },
	{ &skeletalVSource, &hardShadowsFSource },
	{ &particleVSource, &particleFSource },
	{ &waterVSource, &waterFSource },
	{ &text2DVSource, &text2DFSource },
	{ &text3DVSource, &text3DFSource },
	{ &textureless2DVSource, &textureless2DFSource },
	{ &postProcessingVSource, &postProcessingFSource },
	{ &textured2DVSource, &textured2DFSource },
};

void Shaders_Init(){
	for (int k = 0; k < TEXTURED_2D_SHADER + 1; k++)
		CreateShader(&shaders[k], *sources[k].vSource, *sources[k].fSource);
}

unsigned int Shaders_GetProgram(int program){
	return shaders[program].program;
}

void Shaders_Close(){
	for (int k = 0; k < TEXTURED_2D_SHADER + 1; k++){
		glDeleteProgram(shaders[k].program);
		glDeleteShader(shaders[k].fShader);
		glDeleteShader(shaders[k].vShader);
	}
}

void Shaders_SetViewMatrix(float *matrix){
	memcpy(&viewMatrix[0], matrix, 16 * sizeof(float));
	memcpy(&invViewMatrix[0], matrix, 16 * sizeof(float));
	Math_InverseMatrix(invViewMatrix);
}

unsigned int Shaders_GetBonesLocation(){
	return shaders[activeProgram].bonesLoc;
}

void Shaders_UpdateViewMatrix(){
	if (activeProgram == (GLuint)-1) return;
	glUniformMatrix4fv(shaders[activeProgram].viewLoc, 1, GL_TRUE, viewMatrix);

	if (shaders[activeProgram].invViewLoc != (GLuint)-1)
		glUniformMatrix4fv(shaders[activeProgram].invViewLoc, 1, GL_TRUE, invViewMatrix);

	// if(shaders[activeProgram].camRightLoc != (GLuint)-1 && shaders[activeProgram].camUpLoc != (GLuint)-1){
	//     Vector3 cameraRight = (Vector3){viewMatrix[0], viewMatrix[4], viewMatrix[8]};
	//     Vector3 cameraUp    = (Vector3){viewMatrix[1], viewMatrix[5], viewMatrix[9]};
	//     glUniform3fv(shaders[activeProgram].camRightLoc, 1, &cameraRight.x);
	//     glUniform3fv(shaders[activeProgram].camUpLoc, 1, &cameraUp.x);
	// }
}

void Shaders_GetViewMatrix(float *matrix){
	memcpy(matrix, &viewMatrix[0], 16 * sizeof(float));
}

void Shaders_GetInvViewMatrix(float *matrix){
	memcpy(matrix, &invViewMatrix[0], 16 * sizeof(float));
}

void Shaders_SetProjectionMatrix(float *matrix){
	memcpy(&projMatrix[0], matrix, 16 * sizeof(float));
}

void Shaders_UpdateProjectionMatrix(){
	if (activeProgram == (GLuint)-1) return;
	glUniformMatrix4fv(shaders[activeProgram].projLoc, 1, GL_TRUE, projMatrix);
}

void Shaders_GetProjectionMatrix(float *matrix){
	memcpy(matrix, &projMatrix[0], 16 * sizeof(float));
}

void Shaders_SetModelMatrix(float *matrix){
	memcpy(&modelMatrix[0], matrix, 16 * sizeof(float));
	Math_Mat4ToMat3(modelMatrix, invTransMatrix);
	Math_InverseMatrixMat3(invTransMatrix);
	Math_TransposeMatrix3x3(invTransMatrix);
}

void Shaders_UpdateModelMatrix(){
	if (activeProgram == (GLuint)-1) return;
	glUniformMatrix4fv(shaders[activeProgram].modelLoc, 1, GL_TRUE, modelMatrix);

	if (shaders[activeProgram].mInvTransLoc != (GLuint)-1)
		glUniformMatrix3fv(shaders[activeProgram].mInvTransLoc, 1, GL_TRUE, invTransMatrix);
}

void Shaders_GetModelMatrix(float *matrix){
	memcpy(matrix, &modelMatrix[0], 16 * sizeof(float));
}

void Shaders_SetDepthMvpMatrix(float *matrix){
	memcpy(&depthMvpMatrix[0], matrix, 16 * sizeof(float));
}

void Shaders_UpdateDepthMvpMatrix(){
	if (activeProgram == (GLuint)-1 || shaders[activeProgram].depthMvpLoc == (GLuint)-1) return;
	glUniformMatrix4fv(shaders[activeProgram].depthMvpLoc, 1, GL_TRUE, depthMvpMatrix);
	if (activeProgram == (GLuint)-1 || shaders[activeProgram].lightInvDirLoc == (GLuint)-1) return;
	glUniform3f(shaders[activeProgram].lightInvDirLoc, lightInvDir.x, lightInvDir.y, lightInvDir.z);
}

void Shaders_GetDepthMvpMatrix(float *matrix){
	memcpy(matrix, &depthMvpMatrix[0], 16 * sizeof(float));
}

void Shaders_DisableShadows(){
	if (activeProgram == (GLuint)-1 || shaders[activeProgram].useShadowsLoc == (GLuint)-1) return;
	glUniform1i(shaders[activeProgram].useShadowsLoc, 0);
}

void Shaders_SetLightInvDir(const Vector3 &dir){
	lightInvDir = dir;
}

void Shaders_EnableShadows(){
	if (activeProgram == (GLuint)-1 || shaders[activeProgram].useShadowsLoc == (GLuint)-1) return;
	glUniform1i(shaders[activeProgram].useShadowsLoc, 1);
}

void Shaders_DisableColorAttrib(){
	if (activeProgram == (GLuint)-1 || shaders[activeProgram].useColorAttribLoc == (GLuint)-1) return;
	glUniform1i(shaders[activeProgram].useColorAttribLoc, 0);
}

void Shaders_EnableColorAttrib(){
	if (activeProgram == (GLuint)-1 || shaders[activeProgram].useColorAttribLoc == (GLuint)-1) return;
	glUniform1i(shaders[activeProgram].useColorAttribLoc, 1);
}

void Shaders_UseProgram(int program){
	activeProgram = program;
	glUseProgram(Shaders_GetProgram(program));
}

void Shaders_SetUniformColor(Vector4 color){
	glUniform4f(shaders[activeProgram].uniColorLoc, color.x, color.y, color.z, color.w);
}

void Shaders_SetCameraClipMatrix(float *matrix){
	memcpy(&cameraClipMatrix[0], matrix, 16 * sizeof(float));
}

void Shaders_UpdateCameraClipMatrix(){
	if (activeProgram == (GLuint)-1 || shaders[activeProgram].clipMatrixLoc == (GLuint)-1) return;
	glUniformMatrix4fv(shaders[activeProgram].clipMatrixLoc, 1, GL_TRUE, cameraClipMatrix);
}

void Shaders_GetCameraClipMatrix(float *matrix){
	memcpy(matrix, &cameraClipMatrix[0], 16 * sizeof(float));
}

void Shaders_SetRgbSplitAmount(float a){
	if (activeProgram == (GLuint)-1 || shaders[activeProgram].rgbSplitAmountLoc == (GLuint)-1) return;
	glUniform1f(shaders[activeProgram].rgbSplitAmountLoc, a);
}

void Shaders_SetGlowAmount(float a){
	if (activeProgram == (GLuint)-1 || shaders[activeProgram].glowAmountLoc == (GLuint)-1) return;
	glUniform1f(shaders[activeProgram].glowAmountLoc, a);
}

void Shaders_SetBlurAmount(float a){
	if (activeProgram == (GLuint)-1 || shaders[activeProgram].blurAmountLoc == (GLuint)-1) return;
	glUniform1f(shaders[activeProgram].blurAmountLoc, a);
}

void Shaders_SetReflectionAmount(float a){
	if (activeProgram == (GLuint)-1 || shaders[activeProgram].reflectionAmountLoc == (GLuint)-1) return;
	glUniform1f(shaders[activeProgram].reflectionAmountLoc, a);
}

void Text2DShader::Init(){

	vao.Create();
	vao.Bind();

	posVbo.Create(VboType::Vertex);
	coordVbo.Create(VboType::Vertex);
	colorVbo.Create(VboType::Vertex);

	Shaders_UseProgram(TEXT_2D_SHADER);
	unsigned int posLoc = glGetAttribLocation(Shaders_GetProgram(TEXT_2D_SHADER), SHADERS_POSITION_ATTRIB);
	unsigned int uvLoc = glGetAttribLocation(Shaders_GetProgram(TEXT_2D_SHADER), SHADERS_COORD_ATTRIB);
	unsigned int colorLoc = glGetAttribLocation(Shaders_GetProgram(TEXT_2D_SHADER), SHADERS_COLOR_ATTRIB);

	glEnableVertexAttribArray(posLoc);
	glEnableVertexAttribArray(uvLoc);
	glEnableVertexAttribArray(colorLoc);

	posVbo.Bind();
	glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

	coordVbo.Bind();
	glVertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);

	colorVbo.Bind();
	glVertexAttribPointer(colorLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
}

void Text2DShader::UseAndBind(){
	Shaders_UseProgram(TEXT_2D_SHADER);
	vao.Bind();
}

void PostProcessingShader::Init(){

	vao.Create();
	vao.Bind();

	posVbo.Create(VboType::Vertex);

	Shaders_UseProgram(POST_PROCESSING_SHADER);
	unsigned int posLoc = glGetAttribLocation(Shaders_GetProgram(POST_PROCESSING_SHADER), SHADERS_POSITION_ATTRIB);

	glEnableVertexAttribArray(posLoc);

	posVbo.Bind();
	glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	Vector2 positions[] = { { -1, -1 }, { 1, -1 }, { 1, 1 }, { 1, 1 }, { -1, 1 }, { -1, -1 } };
	posVbo.SetData(sizeof(Vector2) * 6, &positions[0].x, GL_STATIC_DRAW);

}

void PostProcessingShader::Render(){
	Shaders_UseProgram(POST_PROCESSING_SHADER);
	glCullFace(GL_BACK);
	vao.Bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
}