////////////////////////////////////////////////////////////////////////
// The scene class contains all the parameters needed to define and
// draw the (really) simple scene, including:
//   * Models (in a VAOs as)
//   * Light parameters
//   * Surface properties
//   * Viewport size parameters
//   * Viewing transformation values
//   * others ...
//
// Some of these parameters are set when the scene is built, and
// others are set by the framework in response to user mouse/keyboard
// interactions.  All of them may be used to draw the scene.

#include <glm/glm.hpp>
#include <glm/ext.hpp>
using namespace glm;

#include "models.h"
#include "shader.h"
#include "texture.h"
#include "fbo.h"
#include "FSQ.h"
#include "LocalLight.h"

#include <vector>
#include <random>

#define MAX_BLUR_WIDTH 100

enum GBufferDebugMode {
	G_POS,
	G_NORM,
	G_DIFF_XYZ,
	G_DIFF_W,
	G_SPEC,
	NONE
};

enum ShadowDebugMode {
	PIXEL_DEPTH,
	PIXEL_DEPTH_MAPPED,
	LIGHT_DEPTH,
	LIGHT_DEPTH_MAPPED,
	LIGHT_DEPTH_FROM_TEXTURE,
	SHADOW_COLOR,
	LIGHT_DEPTH_LOGARITHMIC,
	EXPONENTIAL_PIXEL_DEPTH,
	VISIBILITY,
	NONE_SHADOW,
	SHADOW_DEBUG_COUNT
};

class Scene
{
public:
    // Some user controllable parameters
    int mode;  // Communicated to the shaders as "mode".  Keys '0'-'9'
    int nSpheres;
	int nLights;
    bool drawSpheres;
    bool drawGround;
	bool drawShadows;
	bool drawObject;
	bool isForward;
	bool isShadowEnabled;
	bool isSSAOEnabled;
	bool isSSAOBlurred;
	bool isParallaxMappingProject;

	// Normal maps
	bool isNormalMapEnabled;
	bool drawDebugQuads;
	bool isParallaxMapEnabled;
	bool isSteepParallaxMappingEnabled;
	bool isParallaxOcclusionMappingEnabled;
	bool enhanceScaledViewVector;
	bool cropTextureMap;
	int depthLayerAmount;
	bool brick;

	GBufferDebugMode gBufDebug = NONE;
	ShadowDebugMode shadowDebug = NONE_SHADOW;

	// C value for Exponential Shadow Map
	float esmCValue;

	// blur data
	int blurHalfWidth, blurWidth;
	float blurWeightArray[MAX_BLUR_WIDTH+1];
	GLuint uniformBlockIDForBlurring;

	// height scaling for Parallax mapping
	float heightScale;

	// SSAO data
	std::uniform_real_distribution<GLfloat> randomNumbers; // random number distribution w.r.t uniform distribution
	std::default_random_engine randomNumberGenerator;
	std::vector<vec3> ssaoKernel;
	std::vector<vec3> ssaoNoise;
	Texture ssaoNoiseTexture;
	float ssaoRadius;

    int centralType;
    int centralModel;
	int lightIndex = 0;
    MAT4 centralTr;
	MAT4 SunModelTr;
	MAT4 SphereModelTr;

    // Viewing transformation parameters
	float spin, tilt;
	float tx, ty;
	float zoom;
	float ry, rx;
	float front, back;

	float shadowFront, shadowBack;
	float groundRadius;

	//Values needed for lighting calculation
	vec3 ambientColor;
	vec3 lightColor;

	// Local lights
	std::vector<LocalLight> localLights;

	// Global Lights
	vec3 lightPosition;
	float lightSpin;
	float lightTilt;
	float lightDist;
	vec3 lightDir;

	MAT4 WorldView, WorldProj, LightView, LightProjection;

	//FBO
	FBO gBuffer;
	FBO gBufferForSSAO;
	FBO shadowBufferObject;
	FBO ssaoFBO; // for the final floating-point result
	FBO ssaoBlurFBO;

    // Viewport
    int width, height;

    // Shader programs
	// PARALLAX
	ShaderProgram lightingShaderParallaxMapping;

	// SSAO
	ShaderProgram lightingShaderSSAO;
	ShaderProgram gBufferPassForSSAO;
	ShaderProgram ssaoOcclusionCalculatePass;
	ShaderProgram ssaoOcclusionBlurPass;

	// Deferred
	ShaderProgram deferredShaderGBufferPass;
	ShaderProgram deferredShaderAmbientPass;
	ShaderProgram deferredShaderLocalLightPass;

	// ESM
	ShaderProgram shadowShader;
	ShaderProgram blurShader;
	ShaderProgram verticalBlurShader;
	ShaderProgram lightingShaderWithShadow;

	// Debug program
	ShaderProgram debugging;

    // The polygon models (VAOs - Vertex Array Objects)
    Model* centralPolygons;
    Model* spherePolygons;
    Model* groundPolygons;
	Model* skyDome;

	FSQ fullScreenQuad;

    // Texture
    Texture groundTexture;
	Texture groundNormal;
	Texture groundDepthMap;

	Texture groundWooden;
	Texture groundWoodenNormal;
	Texture groundWoodenDepthMap;

	Texture tempImage;
	Texture blurImage;

    // Main methods
    void InitializeScene();
    void DrawScene();
	void InitializeLights(int nLights, bool randomized = false, bool allWhite = false);

    // Helper methods
    void SetCentralModel(const int i);
	void SetLightIndex(const int i) { lightIndex = i; };
    void DrawSun(unsigned int program);
	void DrawSpheres(unsigned int program);
    void DrawGround(unsigned int program);
	void DrawModel(const int program, Model * m);

private:
	// Deferred shading draws
	void DeferredShadingGeometryPass();
	void DeferredShadingAmbientPass();
	void DrawLocalLights();
	void DeferredShadingLightingPass();

	// Forward draws
	// ESM
	void DrawShadows();
	void BlurPass();
	void DrawLightingWithShadows();

	// PARALLAX
	void DrawLightingParallaxMapping();

	// SSAO
	void SSAOGeometryPass();
	void SSAOOcclusionCalculatePass();
	void SSAOOcclusionBlurPass();
	void DrawLightingSSAO();
	
	void ForwardShading();
	void DeferredShading();

	// ESM
	void BuildKernelWeights();
	float ComputeWeight(int counter);
	float NormalDistribution(float value, float mean, float deviation);
	void BuildKernelWeightsWithNormalDistribution();

	// SSAO
	void BuildSSAOSampleKernel();
	void BuildNoiseForSSAOKernel();

};

