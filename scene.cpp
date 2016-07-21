
//////////////////////////////////////////////////////////////////////
// Defines and draws a scene.  There are two main procedures here:
//
// (1) void Scene::InitializeScene(): is called before the main loop
// is entered, and is expected to setup all OpenGL objects needed for
// the rendering loop.
//
// (2) void Scene::DrawScene(): Is called each time the screen needs
// to be refreshed. (Which will be whenever the screen is
// expose/resized, anytime the periodic animation clock ticks, or
// anytime there is user mouse/keyboard interaction.
////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <stdlib.h>
#include <stdio.h>

#include <glload/gl_3_3.h>
#include <glload/gl_load.hpp>
#include <GL/freeglut.h>
#include <glimg/glimg.h>
 
#include "scene.h"

static MAT4 Identity = MAT4();
const float PI = 3.14159f;
const float rad = PI/180.0f;
const std::string fragmentShaderExtension = ".frag";
const std::string vertexShaderExtension = ".vert";
const std::string computeShaderExtension = ".comp";

const std::string shaderFolderPath = "shaders//";
// DEFERRED
const std::string gBufferPassDeferred = "deferredGBuffer";
const std::string ambientPassDeferred = "deferredAmbient";
const std::string lightingPassDeferred = "deferredLocalLights";

// PARALLAX
const std::string lightingPassParallax = "lightingParallaxMapping";

// ESM
const std::string lightingPassESM = "lightingSoftShadow";
const std::string shadowShaderName = "shadow";
const std::string blurShaderName = "blur";
const std::string verticalBlurShaderName = "blurVertical";

// SSAO
const std::string gBufferPassSSAOName = "gBufferSSAO";
const std::string ssaoOcclusionValuePassName = "ssaoOcclusionCalculationPass";
const std::string ssaoOcclusionBlurPassName = "ssaoOcclusionBlurPass";
const std::string lightingPassSSAO = "lightingSSAO";

// UTILITY
const std::string debuggingShaderName = "debugWindow";

////////////////////////////////////////////////////////////////////////
// This macro makes it easy to sprinkle checks for OpenGL errors
// through your code.  Most OpenGL calls can record errors, and a
// careful programmer will check the error status *often*, perhaps as
// often as after each OpenGL call.  At the very least, once per
// refresh will tell you if something is going wrong.
#define CHECKERROR {int err = glGetError(); if (err) { fprintf(stderr, "OpenGL error in scene.cpp at line %d: %s\n", __LINE__, gluErrorString(err)); getchar(); exit(-1);} }
#define MAX_SAMPLE_VALUES_SSAO 64
#define NOISE_SIZE 4

#define Sqrt2Pi 2.5066282746310005024157652848110452530069867406099383

////////////////////////////////////////////////////////////////////////
// A small function to provide a more friendly method of defining
// colors.  The parameters are hue (0..1: fraction of distance around
// the color wheel; red at 0, green at 1/3, blue at 2/3), saturation
// (0..1: achromatic to saturated), and value (0..1: brightness).
vec3 HSV2RGB(const float h, const float s, const float v)
{
    if (s == 0.0)
        return vec3(v,v,v);

    int i = (int)(h*6.0);
    float f = (h*6.0f) - i;
    float p = v*(1.0f - s);
    float q = v*(1.0f - s*f);
    float t = v*(1.0f - s*(1.0f-f));
    if (i%6 == 0)     return vec3(v,t,p);
    else if (i == 1)  return vec3(q,v,p);
    else if (i == 2)  return vec3(p,v,t);
    else if (i == 3)  return vec3(p,q,v);
    else if (i == 4)  return vec3(t,p,v);
    else if (i == 5)  return vec3(v,p,q);
}

////////////////////////////////////////////////////////////////////////
// Called regularly to update the rotation of the surrounding sphere
// environment.  Set to rotate once every two minutes.
float atime = 0.0;
void animate(int value)
{
	atime = 360.0f*glutGet(GLUT_ELAPSED_TIME) / 120000.0f;
	glutPostRedisplay();
}

void Scene::InitializeLights(int nLights, bool randomized /*= false*/, bool allWhite /*= false*/)
{
	nLights = nLights;

	if (nLights == 0)
		return;

	srand(time(NULL));
	// Light colors and position parameters
	ambientColor = vec3(0.2f);
	lightColor = vec3(0.2f, 1.0f, 1.0f);

	for (int i = 0; i < nLights; ++i) {
		localLights.push_back(LocalLight(randomized, false, allWhite));
	}
}

// Helper function for creating program
void CreateProgram(ShaderProgram& program, std::string shaderName) {
	program.CreateProgram();

	std::string vertexShader = shaderFolderPath + shaderName + vertexShaderExtension;
	std::string fragmentShader = shaderFolderPath + shaderName + fragmentShaderExtension;
	program.CreateShader(vertexShader.c_str(), GL_VERTEX_SHADER);
	program.CreateShader(fragmentShader.c_str(), GL_FRAGMENT_SHADER);

	glBindAttribLocation(program.program, 0, "vertPosition");
	glBindAttribLocation(program.program, 1, "vertColor");
	glBindAttribLocation(program.program, 2, "vertNormal");
	glBindAttribLocation(program.program, 3, "vertTexCoord");

	program.LinkProgram();
}

////////////////////////////////////////////////////////////////////////
// InitializeScene is called once during setup to create all the
// textures, model VAOs, render target FBOs, and shader programs as
// well as a number of other parameters.
void Scene::InitializeScene()
{
    CHECKERROR;

    //////////////////////////////////////////////////////////////////////
    // Initialize various scene parameters.

    // Scene creation parameters
    mode = 0;
    nSpheres = 12;
    drawSpheres = false;
    drawGround = true;
	drawShadows = false;
	drawObject = false;
	isForward = true;
	isShadowEnabled = false;
	isSSAOEnabled = true;
	isSSAOBlurred = false;
	isNormalMapEnabled = false;
	drawDebugQuads = false;
	isParallaxMapEnabled = false;
	isSteepParallaxMappingEnabled = false;
	isParallaxOcclusionMappingEnabled = false;
	enhanceScaledViewVector = false;
	cropTextureMap = false;
	brick = true;
	isParallaxMappingProject = false;
	depthLayerAmount = 10;

	// Exponential shadow constant
	esmCValue = 60.0f;

	// Height scale constant for parallax mapping
	heightScale = 0.1f;

	// blur data
	memset(blurWeightArray, 0, (MAX_BLUR_WIDTH+1) * sizeof(float)); //clear the array
	blurHalfWidth = 32;
	blurWidth = 2 * blurHalfWidth;
	BuildKernelWeightsWithNormalDistribution();
	//BuildKernelWeights();
	tempImage.GenerateTexture(1024, 1024, GL_R32F, GL_RED);
	blurImage.GenerateTexture(1024, 1024, GL_R32F, GL_RED);

	glGenBuffers(1, &uniformBlockIDForBlurring);

	// SSAO
	randomNumbers = std::uniform_real_distribution<GLfloat>(0.0f, 1.0f);
	BuildSSAOSampleKernel();
	BuildNoiseForSSAOKernel();
	ssaoRadius = 1.0f;

    // Scene transformation parameters
	spin = -90.0f;
	tilt = 0.0f;
	tx = 0;
	ty = 0;
	zoom = 150.0f;
	ry = 0.2f;
	front = 0.1f;
	back = 1000.0f;
	shadowFront = 1.0f;
	shadowBack = 1000.0f;
	groundRadius = 50.0f;
	lightColor = vec3(1.0f, 1.0f, 1.0f);
	lightSpin = -90.0f;
	lightTilt = -60.0f;
	lightDist = 60.0f;


	//gBuffer.CreateFBOForDeferredShading(width, height);
	gBuffer.CreateFBOForDeferredShading(width, height);
	shadowBufferObject.CreateFBO(1024, 1024, GL_R32F, GL_RED);
	gBufferForSSAO.CreateFBOForSSAO(width, height);
	ssaoFBO.CreateFBOForSSAOColorBuffer(width, height);
	//ssaoFBO.CreateFBO(width, height, GL_RED, GL_RGB);
	ssaoBlurFBO.CreateFBOForSSAOColorBuffer(width, height);

    // Enable OpenGL depth-testing
    glEnable(GL_DEPTH_TEST);

    //////////////////////////////////////////////////////////////////////
    // Create the models which will compose the scene.  These are all
    // built (in models.cpp) as Vertex Array Objects (VAO's) and sent
    // to the graphics card.
    spherePolygons = new Sphere(32);
    groundPolygons = new Ground(groundRadius, 100);
    SetCentralModel(0);         // Teapot, sphere, or some PLY model, or ...

	// SHADERS
	std::string vertexShader, fragmentShader;

    // Create the program
	deferredShaderGBufferPass.CreateProgram();
    // Read and compile the source from two files.
	vertexShader = shaderFolderPath + gBufferPassDeferred + vertexShaderExtension;
	fragmentShader = shaderFolderPath + gBufferPassDeferred + fragmentShaderExtension;

	deferredShaderGBufferPass.CreateShader(vertexShader.c_str(), GL_VERTEX_SHADER);
	deferredShaderGBufferPass.CreateShader(fragmentShader.c_str(), GL_FRAGMENT_SHADER);

    glBindAttribLocation(deferredShaderGBufferPass.program, 0, "vertex");
    glBindAttribLocation(deferredShaderGBufferPass.program, 1, "vertexNormal");
    glBindAttribLocation(deferredShaderGBufferPass.program, 2, "vertexTexture");
    glBindAttribLocation(deferredShaderGBufferPass.program, 3, "vertexTangent");

	// Link the shader (checking for errors and aborting if necessary).
	deferredShaderGBufferPass.LinkProgram();

	// DEFERRED AMBIENT
	CreateProgram(deferredShaderAmbientPass, ambientPassDeferred);
	CHECKERROR;

	// DEFERRED SHADING LOCAL LIGHT PASS
	CreateProgram(deferredShaderLocalLightPass, lightingPassDeferred);
	CHECKERROR;

	// DEFERRED AMBIENT FULL SCREEN QUAD
	fullScreenQuad.Init();
	fullScreenQuad.LoadToGFX();

	// FORWARD SHADING PART

	// SSAO
	// geometry
	gBufferPassForSSAO.CreateProgram();

	vertexShader = shaderFolderPath + gBufferPassSSAOName + vertexShaderExtension;
	fragmentShader = shaderFolderPath + gBufferPassSSAOName + fragmentShaderExtension;

	gBufferPassForSSAO.CreateShader(vertexShader.c_str(), GL_VERTEX_SHADER);
	gBufferPassForSSAO.CreateShader(fragmentShader.c_str(), GL_FRAGMENT_SHADER);

	glBindAttribLocation(gBufferPassForSSAO.program, 0, "vertex");
	glBindAttribLocation(gBufferPassForSSAO.program, 1, "vertexNormal");
	glBindAttribLocation(gBufferPassForSSAO.program, 2, "vertexTexture");
	glBindAttribLocation(gBufferPassForSSAO.program, 3, "vertexTangent");

	gBufferPassForSSAO.LinkProgram();

	//ssao calculate
	CreateProgram(ssaoOcclusionCalculatePass, ssaoOcclusionValuePassName);

	// ssao blur
	CreateProgram(ssaoOcclusionBlurPass, ssaoOcclusionBlurPassName);

	// Lighting
	CreateProgram(lightingShaderSSAO, lightingPassSSAO);
	/*lightingShaderSSAO.CreateProgram();

	vertexShader = shaderFolderPath + lightingPassSSAO + vertexShaderExtension;
	fragmentShader = shaderFolderPath + lightingPassSSAO + fragmentShaderExtension;

	lightingShaderSSAO.CreateShader(vertexShader.c_str(), GL_VERTEX_SHADER);
	lightingShaderSSAO.CreateShader(fragmentShader.c_str(), GL_FRAGMENT_SHADER);

	glBindAttribLocation(lightingShaderSSAO.program, 0, "vertex");
	glBindAttribLocation(lightingShaderSSAO.program, 1, "vertexNormal");
	glBindAttribLocation(lightingShaderSSAO.program, 2, "vertexTexture");
	glBindAttribLocation(lightingShaderSSAO.program, 3, "vertexTangent");

	lightingShaderSSAO.LinkProgram();*/

	CHECKERROR

	// LIGHTING WITH PARALLAX
	lightingShaderParallaxMapping.CreateProgram();

	vertexShader = shaderFolderPath + lightingPassParallax + vertexShaderExtension;
	fragmentShader = shaderFolderPath + lightingPassParallax + fragmentShaderExtension;

	lightingShaderParallaxMapping.CreateShader(vertexShader.c_str(), GL_VERTEX_SHADER);
	lightingShaderParallaxMapping.CreateShader(fragmentShader.c_str(), GL_FRAGMENT_SHADER);

	glBindAttribLocation(lightingShaderParallaxMapping.program, 0, "vertex");
	glBindAttribLocation(lightingShaderParallaxMapping.program, 1, "vertexNormal");
	glBindAttribLocation(lightingShaderParallaxMapping.program, 2, "vertexTexture");
	glBindAttribLocation(lightingShaderParallaxMapping.program, 3, "vertexTangent");

	lightingShaderParallaxMapping.LinkProgram();

	// LIGHTING WITH SHADOW
	lightingShaderWithShadow.CreateProgram();

	vertexShader = shaderFolderPath + lightingPassESM + vertexShaderExtension;
	fragmentShader = shaderFolderPath + lightingPassESM + fragmentShaderExtension;

	lightingShaderWithShadow.CreateShader(vertexShader.c_str(), GL_VERTEX_SHADER);
	lightingShaderWithShadow.CreateShader(fragmentShader.c_str(), GL_FRAGMENT_SHADER);

	glBindAttribLocation(lightingShaderWithShadow.program, 0, "vertex");
	glBindAttribLocation(lightingShaderWithShadow.program, 1, "vertexNormal");
	glBindAttribLocation(lightingShaderWithShadow.program, 2, "vertexTexture");
	glBindAttribLocation(lightingShaderWithShadow.program, 3, "vertexTangent");

	lightingShaderWithShadow.LinkProgram();

	// SHADOW
	shadowShader.CreateProgram();

	vertexShader = shaderFolderPath + shadowShaderName + vertexShaderExtension;
	fragmentShader = shaderFolderPath + shadowShaderName + fragmentShaderExtension;

	shadowShader.CreateShader(vertexShader.c_str(), GL_VERTEX_SHADER);
	shadowShader.CreateShader(fragmentShader.c_str(), GL_FRAGMENT_SHADER);

	glBindAttribLocation(shadowShader.program, 0, "vertex");
	glBindAttribLocation(shadowShader.program, 1, "vertexNormal");
	glBindAttribLocation(shadowShader.program, 2, "vertexTexture");
	glBindAttribLocation(shadowShader.program, 3, "vertexTangent");

	shadowShader.LinkProgram();

	// BLUR
	blurShader.CreateProgram();

	std::string computeShader = shaderFolderPath + blurShaderName + computeShaderExtension;

	blurShader.CreateShader(computeShader.c_str(), GL_COMPUTE_SHADER);

	blurShader.LinkProgram();

	// BLUR VERTICAL
	verticalBlurShader.CreateProgram();

	computeShader = shaderFolderPath + verticalBlurShaderName + computeShaderExtension;

	verticalBlurShader.CreateShader(computeShader.c_str(), GL_COMPUTE_SHADER);

	verticalBlurShader.LinkProgram();
	// DEBUG
	CreateProgram(debugging, debuggingShaderName);
	CHECKERROR;

	//////////////////////////////////////////////////////////////////////
	// Read a texture and store its id in groundTexture.  Abort
	// program on error.

	// Bricks
	groundTexture.Read("images/bricks2.jpg");
	groundNormal.Read("images/bricks2_normal.jpg");
	groundDepthMap.Read("images/bricks2_disp.jpg");

	// Wooden toy
	groundWooden.Read("images/wood.png");
	groundWoodenNormal.Read("images/toy_box_normal.png");
	groundWoodenDepthMap.Read("images/toy_box_disp.png");

	/*groundTexture.Read("images/6670-diffuse.jpg");
	groundNormal.Read("images/6670-normal.jpg");*/
	CHECKERROR;
}


void Scene::SetCentralModel(const int i)
{
    delete centralPolygons;
    centralPolygons = NULL;
    centralModel = i;

    if (centralModel==0) {
        centralPolygons =  new Teapot(12);    
        float s = static_cast<float>(3.0/centralPolygons->size);
        centralTr =
            Scale(s,s,s)*
            Translate(-centralPolygons->center); }

    else if (centralModel==1) {
        centralPolygons = new Ply("bunny.ply");
        float s = static_cast<float>(3.0/centralPolygons->size);
        centralTr =
            Rotate(2, 180.0f)
            *Rotate(0, 90.0f)
            *Scale(s,s,s)
            *Translate(-centralPolygons->center); }

    else if (centralModel==2) {
        centralPolygons = new Ply("dragon.ply");
        float s = static_cast<float>(3.0/centralPolygons->size);
        centralTr =
            Rotate(2, 180.0f)
            *Rotate(0, 90.0f)
            *Scale(s,s,s)
            *Translate(-centralPolygons->center); }

    else {       // fall back model
        centralPolygons = new Sphere(32);
        float s = static_cast<float>(3.0/centralPolygons->size);
        centralTr = Rotate(0, 180.0f)*Scale(-s,s,s); }
}

////////////////////////////////////////////////////////////////////////
// Procedure DrawScene is called whenever the scene needs to be drawn.
void Scene::DrawScene()
{
	// Calculate the light's position.
	lightPosition = vec3(lightDist*cos(lightSpin*rad)*sin(lightTilt*rad),
		lightDist*sin(lightSpin*rad)*sin(lightTilt*rad),
		lightDist*cos(lightTilt*rad));

	WorldView = Translate(tx, ty, -zoom) * Rotate(0, tilt - 90) * Rotate(2, spin);

	rx = ry * (static_cast<float>(width) / height); //We're initializing this here to make sure that resizing doesn't affect the orientation
	WorldProj = Perspective(rx, ry, front, back);

	// Light View and Projection
	// I might calculate this by hand later instead of using lookAt - "http://learnopengl.com/#!Getting-started/Camera"
	vec3 upDir(0, 0, 1);

	glm::mat4 tempView = glm::lookAt(lightPosition, lightDir, upDir);
	tempView = glm::transpose(tempView);
	glm::mat4 tempProj = glm::frustum(-1.0f, 1.0f, -1.0f, 1.0f, shadowFront, shadowBack);
	//glm::mat4 tempProj = glm::perspective(1.0f, 16.0f / 9.0f, 0.1f, back);
	//glm::mat4 tempProj = glm::infinitePerspective(90.0f, static_cast<float>(width / height), 1.0f);
	tempProj = glm::transpose(tempProj);

	// We actually don't need to do this, sending &tempLight[0] would suffice.
	// However, leave it for now for learning purposes.
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			LightView[i][j] = tempView[i][j];
			LightProjection[i][j] = tempProj[i][j];
		}
	}

	SphereModelTr = Rotate(2, atime);
	SunModelTr = Translate(lightPosition);
	if (isForward)
		ForwardShading();
	else
		DeferredShading();

	// After all drawing, schedule a call to the animate procedure in 10 ms.
	glutTimerFunc(10, animate, 1);

}

void Scene::ForwardShading()
{

	if (isShadowEnabled) {
		DrawShadows();
		BlurPass();
		DrawLightingWithShadows();
	}
	else if (isParallaxMappingProject) {
		DrawLightingParallaxMapping();
	}
	else {
		SSAOGeometryPass();
		//SSAOOcclusionCalculatePass();
		/*SSAOOcclusionBlurPass();
		DrawLightingSSAO();*/
	}

}

void Scene::DeferredShading()
{
	DeferredShadingGeometryPass();

	//DeferredShadingLightingPass();
}

void Scene::BuildKernelWeights()
{
	float total = 0.0f;

	float weight;

	for (int i = 0; i < blurHalfWidth; ++i) {
		weight = ComputeWeight(i);
		blurWeightArray[i] = blurWeightArray[blurWidth - i] = weight; //filling the array from both sides
		total += 2.0f * weight;
	}

	// Now calculate for the 0th index
	blurWeightArray[blurHalfWidth] = ComputeWeight(blurHalfWidth);
	total += blurWeightArray[blurHalfWidth];

	float arrayTotal = 0.0f;
	std::cout << "Index\t\tValue" << std::endl;
	// Normalize
	for (int i = 0; i <= blurWidth; ++i) {
		std::cout << i << " \t\t " << blurWeightArray[i] << std::endl;
		blurWeightArray[i] /= total;
		arrayTotal += blurWeightArray[i];
	}
	std::cout << "\t\tNEW ARRAY\t\t" << std::endl;
	std::cout << "Index\t\tValue" << std::endl;
	// Print new array
	for (int i = 0; i <= blurWidth; ++i) {
		std::cout << i << " \t\t " << blurWeightArray[i] << std::endl;
	}
	std::cout << "Array Total: " << arrayTotal << std::endl;
}

float Scene::ComputeWeight(int counter)
{
	float exponentialSecondPart = pow(static_cast<float>(counter - blurHalfWidth) / (blurHalfWidth / 2.0f), 2);
	return exp(-0.5f * exponentialSecondPart);
}

float Scene::NormalDistribution(float value, float mean, float deviation)
{
	value -= mean;
	float valueSquared = value*value;
	float variance = deviation * deviation;
	return std::exp(-valueSquared / (2.0f * variance)) / (Sqrt2Pi * deviation);
}

void Scene::BuildKernelWeightsWithNormalDistribution()
{
	float total = 0.0f;
	float current;

	for (int i = 0; i < blurHalfWidth; ++i) {
		current = NormalDistribution(float(blurHalfWidth - i), 0.0f, 2.0f);
		blurWeightArray[i] = blurWeightArray[(blurWidth - i)] = current;
		total += 2.0f * current;
	}

	blurWeightArray[blurHalfWidth] = NormalDistribution(0.0f, 0.0f, 2.0f);
	total += blurWeightArray[blurHalfWidth];

	// Normalize the values so that they sum to 1
	for (int i = 0; i <= blurWidth; ++i)
	{
		blurWeightArray[i] /= total;
	}
}

void Scene::BuildSSAOSampleKernel()
{
	GLfloat x, y, z;
	GLfloat scale;
	for (GLuint i = 0; i < MAX_SAMPLE_VALUES_SSAO; ++i) {
		// Get three random values
		// x = [-1, 1], y = [-1, 1], z = [0, 1]
		// Why those values = for unit circle
		// Why z is not those values = we want a hemisphere, z =[-1, 1] makes it a sphere
		x = randomNumbers(randomNumberGenerator) * 2.0f - 1.0f;
		y = randomNumbers(randomNumberGenerator) * 2.0f - 1.0f;
		z = randomNumbers(randomNumberGenerator);
		vec3 kernelValue(x, y, z);
		kernelValue = normalize(kernelValue);
		kernelValue *= randomNumbers(randomNumberGenerator);
		//Scaling
		scale = GLfloat(i) / GLfloat(MAX_SAMPLE_VALUES_SSAO); // ->If we leave it like this, they are evenly distributed
		// We are also doing the one below because we want them to be more dense near the center of the hemisphere
		scale = lerp(0.1f, 1.0f, scale * scale); // 0.1f + scale * (0.9) => this is how it gets closer
		kernelValue *= scale;
		ssaoKernel.push_back(kernelValue);
	}
}

void Scene::BuildNoiseForSSAOKernel()
{
	// We use this to introduce some randomness for better results
	// Otherwise we need a lot more sampling to make it look realistic
	GLfloat x, y;
	float arraySize = NOISE_SIZE * NOISE_SIZE;
	for (GLuint i = 0; i < arraySize; ++i) { //16 = 4 x 4 array of vectors
		x = randomNumbers(randomNumberGenerator) * 2.0f - 1.0f;
		y = randomNumbers(randomNumberGenerator) * 2.0f - 1.0f;
		ssaoNoise.push_back(vec3(x, y, 0.0f)); // z = 0 because this is oriented in tangent space and we're rotating around z
	}

	ssaoNoiseTexture.GenerateTextureForSSAONoise(&ssaoNoise[0]);
}

////////////////////////////////////////////////////////////////////////
// A small helper function to draw a model after settings its lighting
// and modeling parameters.
void Scene::DrawModel(const int program, Model* m)
{
	int loc;
	loc = glGetUniformLocation(program, "ModelMatrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, centralTr.Pntr());

	//This is the N parameter used in lighting calculation
	loc = glGetUniformLocation(program, "NormalMatrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, centralTr.inverse().Pntr());

	//Material properties
	//Kd = diffuse constant
	loc = glGetUniformLocation(program, "diffuse");
	glUniform3fv(loc, 1, &m->diffuseColor[0]);

	//Ks = specular constant
	loc = glGetUniformLocation(program, "specular");
	glUniform3fv(loc, 1, &m->specularColor[0]);

	//shininess - the one used in Phong equation (0..infinite, infinite being mirror-like)
	loc = glGetUniformLocation(program, "shininess;");
	glUniform1f(loc, m->shininess);

	loc = glGetUniformLocation(program, "isTextured");
	glUniform1i(loc, false);

	m->DrawVAO();
}


////////////////////////////////////////////////////////////////////////
// A small helper function for DrawScene to draw all the environment
// spheres.
void Scene::DrawSpheres(unsigned int program)
{
    CHECKERROR;
    float t = 1.0;
    float s = 200.0;
    
    int loc;

	loc = glGetUniformLocation(program, "specular");
	glUniform3fv(loc, 1, &spherePolygons->specularColor[0]);

	loc = glGetUniformLocation(program, "shininess");
	glUniform1f(loc, spherePolygons->shininess);

    for (int i=0;  i<2*nSpheres;  i+=2) {
        float u = float(i)/(2*nSpheres);

        for (int j=2;  j<=nSpheres/2;  j+=2) {
            float v = float(j)/(nSpheres);
            vec3 color = HSV2RGB(u, 1.0f-2.0f*fabs(v-0.5f), 1.0f);

            float s = 3.0f* sin(v*3.14f);
            MAT4 M = SphereModelTr*Rotate(2, 360.0f*u)*Rotate(1, 180.0f*v)
                     *Translate(0.0f, 0.0f, 30.0f)*Scale(s,s,s) ;
            loc = glGetUniformLocation(program, "ModelMatrix");
            glUniformMatrix4fv(loc, 1, GL_TRUE, M.Pntr());
            
            loc = glGetUniformLocation(program, "NormalMatrix");
            glUniformMatrix4fv(loc, 1, GL_FALSE, M.inverse().Pntr());

            loc = glGetUniformLocation(program, "diffuse");
            glUniform3fv(loc, 1, &color[0]);

			loc = glGetUniformLocation(program, "isTextured");
			glUniform1i(loc, false);

            spherePolygons->DrawVAO(); } }

	loc = glGetUniformLocation(program, "ModelMatrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, Identity.Pntr());
	loc = glGetUniformLocation(program, "NormalMatrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, Identity.Pntr());

    CHECKERROR;
}

void Scene::DrawGround(unsigned int program)
{
	int loc;
    loc = glGetUniformLocation(program, "diffuse");
    glUniform3fv(loc, 1, &groundPolygons->diffuseColor[0]);

    loc = glGetUniformLocation(program, "specular");
    glUniform3fv(loc, 1, &groundPolygons->specularColor[0]);

    loc = glGetUniformLocation(program, "shininess");
    glUniform1f(loc, groundPolygons->shininess);

	//glEnable(GL_CULL_FACE);

	if (brick) {
		groundTexture.Bind(0);      // Choose texture unit 1
		loc = glGetUniformLocation(program, "groundTexture");
		glUniform1i(loc, 0);        // Tell the shader about unit 1

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, groundNormal.textureId);
		loc = glGetUniformLocation(program, "groundNormal");
		glUniform1i(loc, 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, groundDepthMap.textureId);
		loc = glGetUniformLocation(program, "depthMap");
		glUniform1i(loc, 2);
	}
	else {
		groundWooden.Bind(0);      // Choose texture unit 1
		loc = glGetUniformLocation(program, "groundTexture");
		glUniform1i(loc, 0);        // Tell the shader about unit 1

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, groundWoodenNormal.textureId);
		loc = glGetUniformLocation(program, "groundNormal");
		glUniform1i(loc, 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, groundWoodenDepthMap.textureId);
		loc = glGetUniformLocation(program, "depthMap");
		glUniform1i(loc, 2);
	}
	

    loc = glGetUniformLocation(program, "ModelMatrix");
    glUniformMatrix4fv(loc, 1, GL_TRUE, Identity.Pntr());
    loc = glGetUniformLocation(program, "NormalMatrix");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Identity.inverse().Pntr());

	loc = glGetUniformLocation(program, "isTextured");
	glUniform1i(loc, true);

    groundPolygons->DrawVAO();

	glActiveTexture(GL_TEXTURE0);
	groundTexture.Unbind();
	glActiveTexture(GL_TEXTURE1);
	groundNormal.Unbind();
	glActiveTexture(GL_TEXTURE2);
	groundDepthMap.Unbind();

}

void Scene::DrawSun(unsigned int program)
{
	vec3 white(100, 1, 1);
	int loc;

	/*loc = glGetUniformLocation(program, "direct");
	glUniform1i(loc, 0);*/

	loc = glGetUniformLocation(program, "diffuse");
	glUniform3fv(loc, 1, &white[0]);

	loc = glGetUniformLocation(program, "ModelMatrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, SunModelTr.Pntr());

	loc = glGetUniformLocation(program, "isReflective");
	glUniform1i(loc, spherePolygons->isReflective);

	//Ii
	loc = glGetUniformLocation(program, "Light");
	glUniform3fv(loc, 1, &(lightColor[0]));

	//Ia
	loc = glGetUniformLocation(program, "Ambient");
	glUniform3fv(loc, 1, &(ambientColor[0]));

	loc = glGetUniformLocation(program, "isTextured");
	glUniform1i(loc, false);

	spherePolygons->DrawVAO();
	CHECKERROR;
}

// DEFERRED
void Scene::DeferredShadingGeometryPass()
{
	gBuffer.Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glViewport(0, 0, width, height);
	glEnable(GL_DEPTH_TEST);

	// Use lighting pass shader
	deferredShaderGBufferPass.Use();
	int program = deferredShaderGBufferPass.program;

	// Send the perspective and viewing matrices to the shader
	int loc = glGetUniformLocation(program, "ProjectionMatrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldProj.Pntr());
	loc = glGetUniformLocation(program, "ViewMatrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldView.Pntr());

	// Draw the scene objects.

	if (drawSpheres) DrawSpheres(program);
	DrawSun(program);
	if (drawGround) DrawGround(program);
	DrawModel(program, centralPolygons);
	CHECKERROR;

	gBuffer.Unbind();
	groundTexture.Unbind();
	// Done with shader program
	deferredShaderGBufferPass.Unuse();

	/*debugging.Use();
	program = debugging.program;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	MAT4 DebugMatrix = Translate(0.65f, 0.67f, 0.5f) * Scale(0.3f, 0.3f, 0.3f);
	int location = glGetUniformLocation(program, "DebugMatrix");
	glUniformMatrix4fv(location, 1, GL_TRUE, DebugMatrix.Pntr());

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gPosition);
	loc = glGetUniformLocation(program, "fboToDebug");
	glUniform1i(loc, 1);

	fullScreenQuad.Draw();

	debugging.Unuse();*/
}

void Scene::DeferredShadingLightingPass()
{
	DeferredShadingAmbientPass();
	DrawLocalLights();
}


void Scene::DrawShadows()
{
	int program = shadowShader.program;

	shadowShader.Use();
	shadowBufferObject.Bind();

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glViewport(0, 0, 1024, 1024);

	//Light View Matrix
	int loc = glGetUniformLocation(program, "LightViewMatrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, LightView.Pntr());

	//Light Proj Matrix
	loc = glGetUniformLocation(program, "LightProjectionMatrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, LightProjection.Pntr());

	// Constant value for ESM
	loc = glGetUniformLocation(program, "C");
	glUniform1f(loc, esmCValue);

	// Front - back values for mapping ESM depth value
	loc = glGetUniformLocation(program, "groundRadius");
	glUniform1f(loc, groundRadius);

	loc = glGetUniformLocation(program, "lightDistance");
	glUniform1f(loc, lightDist);

	//Draw geo
	if (drawSpheres) DrawSpheres(program);
	if (drawGround) DrawGround(program); 
	DrawModel(program, centralPolygons);
	DrawSun(program);

 	shadowBufferObject.Unbind();
	shadowShader.Unuse();
}

void Scene::BlurPass()
{
	int program = blurShader.program;

	blurShader.Use();

	// Blur width information
	int loc = glGetUniformLocation(program, "BlurHalfWidth");
	glUniform1i(loc, blurHalfWidth);
	loc = glGetUniformLocation(program, "BlurWidth");
	glUniform1i(loc, blurWidth);

	// Sending calculated weights
	loc = glGetUniformBlockIndex(program, "Kernel");
	glUniformBlockBinding(program, loc, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBlockIDForBlurring);
	glBufferData(GL_UNIFORM_BUFFER, (MAX_BLUR_WIDTH + 1) * sizeof(float), blurWeightArray, GL_STATIC_DRAW);

	ivec2 direction(1, 0);
	loc = glGetUniformLocation(program, "Direction");
	glUniform2iv(loc, 1, &direction[0]);

	// Sending input - output images
	int imageUnit = 0;

	// Input
	loc = glGetUniformLocation(program, "OriginalShadowMap");
	glBindImageTexture(imageUnit, shadowBufferObject.texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	++imageUnit;
	//Output
	loc = glGetUniformLocation(program, "BlurredShadowMap");
	glBindImageTexture(imageUnit, tempImage.textureId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

	glDispatchCompute(shadowBufferObject.width / 128, shadowBufferObject.height, 1);

	direction = ivec2(0, 1);
	loc = glGetUniformLocation(program, "Direction");
	glUniform2iv(loc, 1, &direction[0]);
	imageUnit = 0;
	// Input
	loc = glGetUniformLocation(program, "OriginalShadowMap");
	glBindImageTexture(imageUnit, tempImage.textureId, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	++imageUnit;
	//Output
	loc = glGetUniformLocation(program, "BlurredShadowMap");
	glBindImageTexture(imageUnit, blurImage.textureId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glDispatchCompute(shadowBufferObject.width / 128, shadowBufferObject.height, 1);
	blurShader.Unuse();
}


void Scene::DrawLightingWithShadows()
{
	// Reset the viewport, and clear the screen
	glViewport(0, 0, width, height);
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use lighting pass shader
	lightingShaderWithShadow.Use();
	int program = lightingShaderWithShadow.program;

	// Send the screen height and width to the shader
	int loc = glGetUniformLocation(program, "WIDTH");
	glUniform1i(loc, width);
	loc = glGetUniformLocation(program, "HEIGHT");
	glUniform1i(loc, height);

	// Send the perspective and viewing matrices to the shader
	loc = glGetUniformLocation(program, "ProjectionMatrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldProj.Pntr());
	loc = glGetUniformLocation(program, "ViewMatrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldView.Pntr());
	loc = glGetUniformLocation(program, "ViewInverse");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldView.inverse().Pntr());

	// Shadow stuff
	// Shadow Matrix
	MAT4 ShadowTextureCoord = Translate(0.5f, 0.5f, 0.5f) * Scale(0.5f, 0.5f, 0.5f);
	MAT4 ShadowMatrix = ShadowTextureCoord * LightProjection * LightView;

	loc = glGetUniformLocation(program, "ShadowMatrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, ShadowMatrix.Pntr());

	// Shadow Texture
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, shadowBufferObject.texture);
	loc = glGetUniformLocation(program, "shadowMap");
	glUniform1i(loc, 5);

	// Blurred shadow texture
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, blurImage.textureId);
	loc = glGetUniformLocation(program, "blurredShadowMap");
	glUniform1i(loc, 6);

	// Front - back values for mapping ESM depth value
	loc = glGetUniformLocation(program, "groundRadius");
	glUniform1f(loc, groundRadius);

	loc = glGetUniformLocation(program, "lightDistance");
	glUniform1f(loc, lightDist);

	// Shadow debug
	loc = glGetUniformLocation(program, "shadowDebug");
	glUniform1i(loc, shadowDebug);

	// Constant value for ESM
	loc = glGetUniformLocation(program, "C");
	glUniform1f(loc, esmCValue);

	//Light position (L)
	loc = glGetUniformLocation(program, "lightPos");
	glUniform3fv(loc, 1, &lightPosition[0]);

	// Send mode to the shader (used to choose alternate shading
	// strategies in the shader)
	loc = glGetUniformLocation(program, "mode");
	glUniform1i(loc, mode);

	// Draw the scene objects.
	DrawSun(program);
	if (drawSpheres) DrawSpheres(program);
	if (drawGround) DrawGround(program);
	DrawModel(program, centralPolygons);

	CHECKERROR;

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, 0);
	// Done with shader program
	lightingShaderWithShadow.Unuse();

	debugging.Use();
	program = debugging.program;

	glClearColor(0.0, 0.0, 0.0, 1.0);

	MAT4 DebugMatrix = Translate(0.65f, 0.65f, 0.5f) * Scale(0.3f, 0.3f, 0.3f);
	int location = glGetUniformLocation(program, "DebugMatrix");
	glUniformMatrix4fv(location, 1, GL_TRUE, DebugMatrix.Pntr());

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, shadowBufferObject.texture);
	loc = glGetUniformLocation(program, "fboToDebug");
	glUniform1i(loc, 7);

	fullScreenQuad.Draw();

	DebugMatrix = Translate(0.65f, -0.25f, 0.5f) * Scale(0.3f, 0.3f, 0.3f);
	location = glGetUniformLocation(program, "DebugMatrix");
	glUniformMatrix4fv(location, 1, GL_TRUE, DebugMatrix.Pntr());

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, blurImage.textureId);
	loc = glGetUniformLocation(program, "fboToDebug");
	glUniform1i(loc, 7);

	fullScreenQuad.Draw();

	DebugMatrix = Translate(-0.65f, -0.25f, 0.5f) * Scale(0.3f, 0.3f, 0.3f);
	location = glGetUniformLocation(program, "DebugMatrix");
	glUniformMatrix4fv(location, 1, GL_TRUE, DebugMatrix.Pntr());

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, tempImage.textureId);
	loc = glGetUniformLocation(program, "fboToDebug");
	glUniform1i(loc, 7);

	fullScreenQuad.Draw();

	debugging.Unuse();

	CHECKERROR;
}

void Scene::DrawLightingParallaxMapping()
{
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use lighting pass shader
	lightingShaderParallaxMapping.Use();
	int program = lightingShaderParallaxMapping.program;


	/*rightNormalMap.Bind();
	loc = glGetUniformLocation(program, "isRight");
	glUniform1i(loc, true);*/
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Send the screen height and width to the shader
	int loc = glGetUniformLocation(program, "WIDTH");
	glUniform1i(loc, width);
	loc = glGetUniformLocation(program, "HEIGHT");
	glUniform1i(loc, height);

	// Send the perspective and viewing matrices to the shader
	loc = glGetUniformLocation(program, "ProjectionMatrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldProj.Pntr());
	loc = glGetUniformLocation(program, "ViewMatrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldView.Pntr());
	loc = glGetUniformLocation(program, "ViewInverse");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldView.inverse().Pntr());

	//Light position (L)
	loc = glGetUniformLocation(program, "lightPos");
	glUniform3fv(loc, 1, &lightPosition[0]);

	// Send mode to the shader (used to choose alternate shading
	// strategies in the shader)
	loc = glGetUniformLocation(program, "mode");
	glUniform1i(loc, mode);

	loc = glGetUniformLocation(program, "isNormalMapped");
	glUniform1i(loc, isNormalMapEnabled);

	loc = glGetUniformLocation(program, "isParallaxMappingEnabled");
	glUniform1i(loc, isParallaxMapEnabled);

	loc = glGetUniformLocation(program, "enhanceViewScaling");
	glUniform1i(loc, enhanceScaledViewVector);

	loc = glGetUniformLocation(program, "isParallaxOcclusionMappingEnabled");
	glUniform1i(loc, isParallaxOcclusionMappingEnabled);

	loc = glGetUniformLocation(program, "isSteepParallaxMappingEnabled");
	glUniform1i(loc, isSteepParallaxMappingEnabled);

	loc = glGetUniformLocation(program, "cropTextureMap");
	glUniform1i(loc, cropTextureMap);

	loc = glGetUniformLocation(program, "depthLayerAmount");
	glUniform1i(loc, depthLayerAmount);

	loc = glGetUniformLocation(program, "heightScale");
	glUniform1f(loc, heightScale);

	// Draw the scene objects.
	DrawSun(program);
	if (drawSpheres) DrawSpheres(program);
	if (drawGround) DrawGround(program);
	if(drawObject) DrawModel(program, centralPolygons);

	CHECKERROR;

	// Done with shader program
	lightingShaderParallaxMapping.Unuse();

	if (drawDebugQuads) {
		debugging.Use();
		program = debugging.program;

		glClearColor(0.0, 0.0, 0.0, 1.0);

		MAT4 DebugMatrix = Translate(0.65f, 0.67f, 0.5f) * Scale(0.3f, 0.3f, 0.3f);
		int location = glGetUniformLocation(program, "DebugMatrix");
		glUniformMatrix4fv(location, 1, GL_TRUE, DebugMatrix.Pntr());

		int textureId;
		if (brick) {
			textureId = groundTexture.textureId;
		}
		else {
			textureId = groundWooden.textureId;
		}

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textureId);
		loc = glGetUniformLocation(program, "fboToDebug");
		glUniform1i(loc, 1);

		fullScreenQuad.Draw();

		DebugMatrix = Translate(0.65f, -0.50f, 0.5f) * Scale(0.3f, 0.3f, 0.3f);
		location = glGetUniformLocation(program, "DebugMatrix");
		glUniformMatrix4fv(location, 1, GL_TRUE, DebugMatrix.Pntr());

		if (brick) {
			textureId = groundNormal.textureId;
		}
		else {
			textureId = groundWoodenNormal.textureId;
		}

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textureId);
		loc = glGetUniformLocation(program, "fboToDebug");
		glUniform1i(loc, 1);

		fullScreenQuad.Draw();

		DebugMatrix = Translate(-0.65f, -0.50f, 0.5f) * Scale(0.3f, 0.3f, 0.3f);
		location = glGetUniformLocation(program, "DebugMatrix");
		glUniformMatrix4fv(location, 1, GL_TRUE, DebugMatrix.Pntr());

		if (brick) {
			textureId = groundDepthMap.textureId;
		}
		else {
			textureId = groundWoodenDepthMap.textureId;
		}

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textureId);
		loc = glGetUniformLocation(program, "fboToDebug");
		glUniform1i(loc, 1);

		fullScreenQuad.Draw();

		debugging.Unuse();

		CHECKERROR;
	}
}

void Scene::SSAOGeometryPass()
{
	gBufferPassForSSAO.Use();

	gBufferForSSAO.Bind();
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
	glViewport(0, 0, width, height);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int program = gBufferPassForSSAO.program; 

	int loc = glGetUniformLocation(program, "ViewMatrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldView.Pntr());

	loc = glGetUniformLocation(program, "ProjectionMatrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldProj.Pntr());

	/*loc = glGetUniformLocation(program, "back");
	glUniform1f(loc, back);

	loc = glGetUniformLocation(program, "front");
	glUniform1f(loc, front);*/

	if (drawSpheres) DrawSpheres(program);
	DrawSun(program);
	if (drawGround) DrawGround(program);
	DrawModel(program, centralPolygons);
	CHECKERROR;

	gBufferForSSAO.Unbind();
	// Done with shader program
	gBufferPassForSSAO.Unuse();

	debugging.Use();
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

	program = debugging.program;

	MAT4 DebugMatrix = Translate(0.65f, 0.65f, 0.5f) * Scale(0.3f, 0.3f, 0.3f);
	int location = glGetUniformLocation(program, "DebugMatrix");
	glUniformMatrix4fv(location, 1, GL_TRUE, DebugMatrix.Pntr());

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBufferForSSAO.gPositionDepth);
	loc = glGetUniformLocation(program, "fboToDebug");
	glUniform1i(loc, 1);

	fullScreenQuad.Draw();

	debugging.Unuse();
}

void Scene::SSAOOcclusionCalculatePass()
{
	ssaoFBO.Bind();
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT);
	ssaoOcclusionCalculatePass.Use();

	int program = ssaoOcclusionCalculatePass.program;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gBufferForSSAO.gPositionDepth);
	int loc = glGetUniformLocation(program, "gPositionDepth");
	glUniform1i(loc, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBufferForSSAO.gNormal);
	loc = glGetUniformLocation(program, "gNormal");
	glUniform1i(loc, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, ssaoNoiseTexture.textureId);
	loc = glGetUniformLocation(program, "ssaoNoise");
	glUniform1i(loc, 2);

	loc = glGetUniformLocation(program, "gBufDebug");
	glUniform1i(loc, gBufDebug);

	// SEND ACTUAL NOISE DATA
	for (GLuint i = 0; i < MAX_SAMPLE_VALUES_SSAO; ++i) {
		loc = glGetUniformLocation(program, ("SampleArray[" + std::to_string(i) + "]").c_str());
		glUniform3fv(loc, 1, &ssaoKernel[i][0]);
	}

	loc = glGetUniformLocation(program, "ProjectionMatrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldProj.Pntr());

	// Send the screen height and width to the shader
	loc = glGetUniformLocation(program, "Width");
	glUniform1f(loc, GLfloat(width));
	loc = glGetUniformLocation(program, "Height");
	glUniform1f(loc, GLfloat(height));

	loc = glGetUniformLocation(program, "NoiseSize");
	glUniform1f(loc, GLfloat(NOISE_SIZE));

	loc = glGetUniformLocation(program, "KernelSize");
	glUniform1i(loc, MAX_SAMPLE_VALUES_SSAO);

	loc = glGetUniformLocation(program, "Radius");
	glUniform1f(loc, ssaoRadius);

	fullScreenQuad.Draw();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);

	ssaoFBO.Unbind();
	ssaoOcclusionCalculatePass.Unuse();

	debugging.Use();
	program = debugging.program;
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	MAT4 DebugMatrix = Translate(0.65f, 0.65f, 0.5f) * Scale(0.3f, 0.3f, 0.3f);
	int location = glGetUniformLocation(program, "DebugMatrix");
	glUniformMatrix4fv(location, 1, GL_TRUE, DebugMatrix.Pntr());

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ssaoFBO.texture);
	loc = glGetUniformLocation(program, "fboToDebug");
	glUniform1i(loc, 1);

	fullScreenQuad.Draw();

	debugging.Unuse();
}

void Scene::SSAOOcclusionBlurPass()
{
	ssaoBlurFBO.Bind();

	glClear(GL_COLOR_BUFFER_BIT);
	ssaoOcclusionBlurPass.Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ssaoFBO.texture);
	int loc = glGetUniformLocation(ssaoOcclusionBlurPass.program, "ssaoTexture");
	glUniform1i(loc, 0);

	loc = glGetUniformLocation(ssaoOcclusionBlurPass.program, "noiseTextureSize");
	glUniform1i(loc, NOISE_SIZE);

	fullScreenQuad.Draw();

	ssaoBlurFBO.Unbind(); 
	ssaoOcclusionBlurPass.Unuse();

	/*debugging.Use();
	int program = debugging.program;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	MAT4 DebugMatrix = Translate(0.65f, 0.67f, 0.5f) * Scale(0.3f, 0.3f, 0.3f);
	int location = glGetUniformLocation(program, "DebugMatrix");
	glUniformMatrix4fv(location, 1, GL_TRUE, DebugMatrix.Pntr());

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ssaoBlurFBO.texture);
	loc = glGetUniformLocation(program, "fboToDebug");
	glUniform1i(loc, 1);

	fullScreenQuad.Draw();

	debugging.Unuse();*/
}

void Scene::DrawLightingSSAO()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Use lighting pass shader
	lightingShaderSSAO.Use();
	int program = lightingShaderSSAO.program;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gBufferForSSAO.gPositionDepth);
	int loc = glGetUniformLocation(program, "gPositionDepth");
	glUniform1i(loc, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBufferForSSAO.gNormal);
	loc = glGetUniformLocation(program, "gNormal");
	glUniform1i(loc, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gBufferForSSAO.gDifSpec);
	loc = glGetUniformLocation(program, "gDifSpec");
	glUniform1i(loc, 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gBufferForSSAO.gSpecular);
	loc = glGetUniformLocation(program, "gSpecular");
	glUniform1i(loc, 3);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, ssaoFBO.texture);
	loc = glGetUniformLocation(program, "ssaoFBO");
	glUniform1i(loc, 4);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, ssaoBlurFBO.texture);
	loc = glGetUniformLocation(program, "ssaoFBOBlurred");
	glUniform1i(loc, 5);

	loc = glGetUniformLocation(program, "LightPosition");
	glUniform3fv(loc, 1, &lightPosition[0]);

	loc = glGetUniformLocation(program, "Linear");
	glUniform1f(loc, 0.09f);

	loc = glGetUniformLocation(program, "Quadratic");
	glUniform1f(loc, 0.032f);

	loc = glGetUniformLocation(program, "ProjectionMatrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldProj.Pntr());
	loc = glGetUniformLocation(program, "ViewMatrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldView.Pntr());
	loc = glGetUniformLocation(program, "ViewInverse");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldView.inverse().Pntr());

	loc = glGetUniformLocation(program, "AmbientLight");
	glUniform3fv(loc, 1, &ambientColor[0]);

	loc = glGetUniformLocation(program, "Width");
	glUniform1i(loc, width);

	loc = glGetUniformLocation(program, "Height");
	glUniform1i(loc, height);

	//Ii
	loc = glGetUniformLocation(program, "LightColor");
	glUniform3fv(loc, 1, &(lightColor[0]));

	loc = glGetUniformLocation(program, "IsAOEnabled");
	glUniform1i(loc, isSSAOEnabled);

	loc = glGetUniformLocation(program, "IsBlurred");
	glUniform1i(loc, isSSAOBlurred);

	fullScreenQuad.Draw();

	CHECKERROR

	lightingShaderSSAO.Unuse();

	if (drawDebugQuads) {
		debugging.Use();
		program = debugging.program;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		MAT4 DebugMatrix = Translate(0.65f, 0.67f, 0.5f) * Scale(0.3f, 0.3f, 0.3f);
		int location = glGetUniformLocation(program, "DebugMatrix");
		glUniformMatrix4fv(location, 1, GL_TRUE, DebugMatrix.Pntr());

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, ssaoFBO.texture);
		loc = glGetUniformLocation(program, "fboToDebug");
		glUniform1i(loc, 1);

		fullScreenQuad.Draw();


		DebugMatrix = Translate(0.65f, -0.67f, 0.5f) * Scale(0.3f, 0.3f, 0.3f);
		location = glGetUniformLocation(program, "DebugMatrix");
		glUniformMatrix4fv(location, 1, GL_TRUE, DebugMatrix.Pntr());

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, ssaoBlurFBO.texture);
		loc = glGetUniformLocation(program, "fboToDebug");
		glUniform1i(loc, 1);

		fullScreenQuad.Draw();

		DebugMatrix = Translate(-0.65f, -0.67f, 0.5f) * Scale(0.3f, 0.3f, 0.3f);
		location = glGetUniformLocation(program, "DebugMatrix");
		glUniformMatrix4fv(location, 1, GL_TRUE, DebugMatrix.Pntr());

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, ssaoNoiseTexture.textureId);
		loc = glGetUniformLocation(program, "fboToDebug");
		glUniform1i(loc, 1);

		fullScreenQuad.Draw();

		debugging.Unuse();
	}
	
}

void Scene::DeferredShadingAmbientPass()
{
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	deferredShaderAmbientPass.Use();

	int program = deferredShaderAmbientPass.program;

	GLuint loc;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gPosition);
	loc = glGetUniformLocation(program, "gPositionMap");
	glUniform1i(loc, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gNormal);
	loc = glGetUniformLocation(program, "gNormalMap");
	glUniform1i(loc, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gSpecular);
	loc = glGetUniformLocation(program, "gSpecularMap");
	glUniform1i(loc, 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gDifSpec);
	loc = glGetUniformLocation(program, "gDifSpecMap");
	glUniform1i(loc, 3);

	loc = glGetUniformLocation(program, "gBufDebug");
	glUniform1i(loc, gBufDebug);

	loc = glGetUniformLocation(program, "ambientLight");
	glUniform3fv(loc, 1, &ambientColor[0]);

	fullScreenQuad.Draw();
	CHECKERROR;

	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	deferredShaderAmbientPass.Unuse();
}

void Scene::DrawLocalLights()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	deferredShaderLocalLightPass.Use();

	int program = deferredShaderLocalLightPass.program;

	GLuint loc = 0;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gPosition);
	loc = glGetUniformLocation(program, "gPositionMap");
	glUniform1i(loc, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gNormal);
	loc = glGetUniformLocation(program, "gNormalMap");
	glUniform1i(loc, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gSpecular);
	loc = glGetUniformLocation(program, "gSpecularMap");
	glUniform1i(loc, 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gBuffer.gDifSpec);
	loc = glGetUniformLocation(program, "gDifSpecMap");
	glUniform1i(loc, 3);

	loc = glGetUniformLocation(program, "gBufDebug");
	glUniform1i(loc, gBufDebug);

	loc = glGetUniformLocation(program, "AmbientLight");
	glUniform3fv(loc, 1, &ambientColor[0]);

	loc = glGetUniformLocation(program, "Width");
	glUniform1i(loc, width);

	loc = glGetUniformLocation(program, "Height");
	glUniform1i(loc, height);

	loc = glGetUniformLocation(program, "ProjectionMatrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldProj.Pntr());
	loc = glGetUniformLocation(program, "ViewMatrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldView.Pntr());
	loc = glGetUniformLocation(program, "ViewInverse");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldView.inverse().Pntr());

	for (unsigned int i = 0; i < localLights.size(); ++i) {

		LocalLight const * localLight = &localLights[i];

		loc = glGetUniformLocation(program, "LightPosition");
		glUniform3fv(loc, 1, &localLight->lightPos[0]);
		
		loc = glGetUniformLocation(program, "LightRange");
		glUniform1f(loc, localLight->radius);

		loc = glGetUniformLocation(program, "LightColor");
		glUniform3fv(loc, 1, &localLight->lightColor[0]);

		loc = glGetUniformLocation(program, "Attenuation");
		glUniform2fv(loc, 1, &localLight->attenuationVector[0]);

		// Model matrix will be just a radius and we'll be coloring inside that radius
		MAT4 ModelMatrix = Translate(localLight->lightPos)*Scale(vec3(localLight->radius));

		loc = glGetUniformLocation(program, "ModelMatrix");
		glUniformMatrix4fv(loc, 1, GL_TRUE, ModelMatrix.Pntr());

		localLight->lightModel->DrawVAO();
		
		//fullScreenQuad.Draw();
	}
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	deferredShaderLocalLightPass.Unuse();

}