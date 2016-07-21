///////////////////////////////////////////////////////////////////////
// Provides the framework for graphics projects.  Most of this small
// file contains the GLUT calls needed to open a window and hook up
// various callbacks for mouse/keyboard interaction and screen resizes
// and redisplays.
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
    // Includes for Windows
    #include <windows.h>
    #include <cstdlib>
    #include <fstream>
    #include <limits>
    #include <crtdbg.h>
#else
    // Includes for Linux
#endif

#include <glload/gl_3_3.h>
#include <glload/gl_load.hpp>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
using namespace glm;

#include "scene.h"
#include "AntTweakBar.h"

#ifndef PI
#define PI 3.14159f
#endif

Scene scene;

// Some globals used for mouse handling.
int mouseX, mouseY;
bool leftDown = false;
bool middleDown = false;
bool rightDown = false;

bool LKeyPressed = false;
bool SKeyPressed = false;

float mouseSpeedDampeningValue = 2.0f;
////////////////////////////////////////////////////////////////////////
// Called by GLUT when the scene needs to be redrawn.
void ReDraw()
{
    scene.DrawScene();
    TwDraw();
    glutSwapBuffers();

}

////////////////////////////////////////////////////////////////////////
// Called by GLUT when the window size is changed.
void ReshapeWindow(int w, int h)
{
    if (w && h) {
        glViewport(0, 0, w, h); }

    TwWindowSize(w,h);
    scene.width = w;
    scene.height = h;
	// Force a redraw
    glutPostRedisplay();

}

////////////////////////////////////////////////////////////////////////
// Called by GLut for keyboard actions.
void KeyboardDown(unsigned char key, int x, int y)
{
    if (TwEventKeyboardGLUT(key, x, y)) return;
	printf("%c", key);

    switch(key) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        scene.mode = key-'0';
        glutPostRedisplay();
        break;
	case 'l':
		LKeyPressed = true;
		break;
	case 's':
		SKeyPressed = true;
		break;


    case 27:                    // Escape key
    case 'q':
        exit(0);
    }
}

void KeyboardUp(unsigned char key, int x, int y)
{
	switch (key) {
	case 'l':
		LKeyPressed = false;
		break;
	case 's':
		SKeyPressed = false;
		break;
	}

}

////////////////////////////////////////////////////////////////////////
// Called by GLut when a mouse button changes state.
void MouseButton(int button, int state, int x, int y)
{
	if (TwEventMouseButtonGLUT(button, state, x, y)) return;
	mouseX = x;
	mouseY = y;

	if (state == GLUT_DOWN) {
		if (button == GLUT_LEFT_BUTTON) {
			leftDown = true;
		}
		else if (button == GLUT_RIGHT_BUTTON) {
			rightDown = true;
		}
		else if (button == GLUT_MIDDLE_BUTTON) {
			middleDown = true;
		}
		else if (button == 3) {
			if (LKeyPressed) {
				if (scene.isForward)
					scene.lightDist -= 5.0f;
				else
					scene.localLights[scene.lightIndex].lightPos.z += 2.0f;
			}
			else
				scene.zoom -= 10.0f;
		}
		else if (button == 4) {
			if (LKeyPressed) {
				if (scene.isForward)
					scene.lightDist += 5.0f;
				else
					scene.localLights[scene.lightIndex].lightPos.z -= 2.0f;
			}
				
			else
				scene.zoom += 10.0f;
		}
		else {
			leftDown = false;
			rightDown = false;
			middleDown = false;
		}
	}
	else if (state == GLUT_UP) {
		if (button == GLUT_LEFT_BUTTON) {
			leftDown = false;
		}
		else if (button == GLUT_RIGHT_BUTTON) {
			rightDown = false;
		}
		else if (button == GLUT_MIDDLE_BUTTON) {
			middleDown = false;
		}
	}

    // To determine if the shift was pressed:
    // if (glutGetModifiers() & GLUT_ACTIVE_SHIFT) ...

    // To determine which button changed state:
    // if (button == GLUT_LEFT_BUTTON)
    // if (button == GLUT_MIDDLE_BUTTON)
    // if (button == GLUT_RIGHT_BUTTON)

    // To determine its new state:
    // if (state == GLUT_UP)
    // if (state == GLUT_DOWN)


    glutPostRedisplay();

}

////////////////////////////////////////////////////////////////////////
// Called by GLut when a mouse moves (while a button is down)
void MouseMotion(int x, int y)
{
    if (TwEventMouseMotionGLUT(x,y)) return;

	if (leftDown) {
		if (x != mouseX) {
			if (LKeyPressed)
				if(scene.isForward)
					scene.lightSpin += (x - mouseX) / mouseSpeedDampeningValue;
				else
					scene.localLights[scene.lightIndex].lightPos.x += (x - mouseX) / 4.0f;
			else 
				scene.spin += (x - mouseX) / mouseSpeedDampeningValue;
			
			mouseX = x;
		}
		if (y != mouseY) {
			if (LKeyPressed)
				if(scene.isForward)
					scene.lightTilt += (y - mouseY) / mouseSpeedDampeningValue;
				else
					scene.localLights[scene.lightIndex].lightPos.y += (y - mouseY) / 4.0f;	
			else 
				scene.tilt += (y - mouseY) / mouseSpeedDampeningValue;

			mouseY = y;
		}
	}
	
	if (middleDown) {
		if (y > mouseY) {
			scene.zoom += 10.0f;
		}
		else if (y < mouseY) {
			scene.zoom -= 10.0f;
		}
		mouseY = y;
	}

	if (rightDown){
		if (x != mouseX) {
			scene.tx += (x - mouseX) / mouseSpeedDampeningValue;
			mouseX = x;
		}

		if (y != mouseY) {
			scene.ty -= (y - mouseY) / mouseSpeedDampeningValue;
			mouseY = y;
		}
	}

	std::cout << scene.localLights[scene.lightIndex].lightPos.x << " , " << scene.localLights[scene.lightIndex].lightPos.y << std::endl;
	

}

////////////////////////////////////////////////////////////////////////
// Functions called by AntTweakBar
void Quit(void *clientData)
{
    TwTerminate();
    glutLeaveMainLoop();
}

void TW_CALL ChangeRenderMode(void *clientData)
{
	scene.isForward = !scene.isForward;
}

void TW_CALL GBufferPosition(void *clientData)
{
	scene.gBufDebug = GBufferDebugMode::G_POS;
}

void TW_CALL GBufferNormal(void *clientData)
{
	scene.gBufDebug = GBufferDebugMode::G_NORM;
}

void TW_CALL GBufferDiffuseXYZ(void *clientData)
{
	scene.gBufDebug = GBufferDebugMode::G_DIFF_XYZ;
}

void TW_CALL GBufferDiffuseW(void *clientData)
{
	scene.gBufDebug = GBufferDebugMode::G_DIFF_W;
}

void TW_CALL GBufferSpecular(void *clientData)
{
	scene.gBufDebug = GBufferDebugMode::G_SPEC;
}

void TW_CALL GBufferClear(void *clientData) {
	scene.gBufDebug = GBufferDebugMode::NONE;
}

void TW_CALL ClearDebug(void *clientData)
{
	scene.gBufDebug = GBufferDebugMode::NONE;
}

void TW_CALL PixelDepthDebugMode(void *clientData)
{
	scene.shadowDebug = ShadowDebugMode::PIXEL_DEPTH;
}

void TW_CALL PixelDepthMappedDebugMode(void *clientData)
{
	scene.shadowDebug = ShadowDebugMode::PIXEL_DEPTH_MAPPED;
}

void TW_CALL LightDepthDebugMode(void *clientData)
{
	scene.shadowDebug = ShadowDebugMode::LIGHT_DEPTH;
}

void TW_CALL LightDepthMappedDebugMode(void *clientData)
{
	scene.shadowDebug = ShadowDebugMode::LIGHT_DEPTH_MAPPED;
}

void TW_CALL LightDepthFromTextureDebugMode(void *clientData)
{
	scene.shadowDebug = ShadowDebugMode::LIGHT_DEPTH_FROM_TEXTURE;
}

void TW_CALL ShadowColorDebugMode(void *clientData)
{
	scene.shadowDebug = ShadowDebugMode::SHADOW_COLOR;
}

void TW_CALL ClearShadowDebug(void *clientData)
{
	scene.shadowDebug = ShadowDebugMode::NONE_SHADOW;
}

void TW_CALL ToggleGround(void *clientData)
{
    scene.drawGround = !scene.drawGround;
} 

void TW_CALL ToggleSpheres(void *clientData)
{
    scene.drawSpheres = !scene.drawSpheres;
}

void TW_CALL SetModel(const void *value, void *clientData)
{
    int i = *(int*)value; // AntTweakBar forces this cast.
    scene.SetCentralModel(i);
}

void TW_CALL GetModel(void *value, void *clientData)
{
    *(int*)value = scene.centralModel;
}

void TW_CALL SetLight(const void *value, void *clientData)
{
	int i = *(int*)value; // AntTweakBar forces this cast.
	scene.SetLightIndex(i);
}

void TW_CALL GetLight(void *value, void *clientData)
{
	*(int*)value = scene.lightIndex;
}

void TW_CALL SetShadowDebugMode(const void *value, void *clientData)
{
	int i = *(int*)value; // AntTweakBar forces this cast.
	scene.shadowDebug = (ShadowDebugMode)i;
}

void TW_CALL GetShadowDebugMode(void *value, void *clientData)
{
	*(int*)value = scene.shadowDebug;
}

void TW_CALL SetLightColor(const void *value, void *clientData)
{
	//float i = *(glm::vec3*)value; // AntTweakBar forces this cast.
	scene.localLights[scene.lightIndex].lightColor = *(glm::vec3*)value;
}

void TW_CALL GetLightColor(void *value, void *clientData)
{
	*(glm::vec3*)value = scene.localLights[scene.lightIndex].lightColor;
}

void TW_CALL GetLightRange(void *value, void *clientData)
{
	*(float*)value = scene.localLights[scene.lightIndex].radius;
}

void TW_CALL GetAttenuationX(void *value, void *clientData)
{
	*(float*)value = scene.localLights[scene.lightIndex].attenuationVector[0];
}

void TW_CALL GetAttenuationY(void *value, void *clientData)
{
	*(float*)value = scene.localLights[scene.lightIndex].attenuationVector[1];
}

std::string ShadowDebugEnumToString(ShadowDebugMode debugMode) {
	switch (debugMode)
	{
	case PIXEL_DEPTH:
		return "Pixel Depth";
	case PIXEL_DEPTH_MAPPED:
		return "Pixel Depth Mapped";
	case LIGHT_DEPTH:
		return "Light Depth";
	case LIGHT_DEPTH_MAPPED:
		return "Light Depth Mapped";
	case LIGHT_DEPTH_FROM_TEXTURE:
		return "Light Depth From Texture";
	case SHADOW_COLOR:
		return "Shadow Color";
	case LIGHT_DEPTH_LOGARITHMIC:
		return "Light Depth Logarithmic";
	case EXPONENTIAL_PIXEL_DEPTH:
		return "Exponential Pixel Depth";
	case VISIBILITY:
		return "Visibility";
	case NONE_SHADOW:
	default:
		return "Default";
	}

	return "";
}

////////////////////////////////////////////////////////////////////////
// Do the OpenGL/GLut setup and then enter the interactive loop.
int main(int argc, char** argv)
{
	/* Original main */
	
    // Initialize GLUT and open a window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitContextVersion (3, 3);
    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
	scene.width = 1024;
	scene.height = 768;
    glutInitWindowSize(scene.width, scene.height);


    glutCreateWindow("CS562 Framework");
	//glutFullScreen();
	glutHideOverlay();
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
    // Initialize OpenGl
    glload::LoadFunctions();
    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("Rendered by: %s\n", glGetString(GL_RENDERER));
    fflush(stdout);

    // Hookup GLUT callback for all events we're interested in
    glutIgnoreKeyRepeat(true);
    glutDisplayFunc(&ReDraw);
    glutReshapeFunc(&ReshapeWindow);
    glutKeyboardFunc(&KeyboardDown);
    glutKeyboardUpFunc(&KeyboardUp);
    glutMouseFunc(&MouseButton);
    glutMotionFunc(&MouseMotion);
    glutPassiveMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
    glutSpecialFunc((GLUTspecialfun)TwEventSpecialGLUT);

    // Initialize the tweakbar with a few tweaks.  
    TwInit(TW_OPENGL, NULL);
    TwGLUTModifiersFunc((int(TW_CALL*)())glutGetModifiers);
    TwBar *bar = TwNewBar("Tweaks");
    TwDefine(" Tweaks size='300 400' ");
    TwAddButton(bar, "quit", (TwButtonCallback)Quit, NULL, " label='Quit' key=q ");
	
	//TwAddButton(bar, "Render Mode", (TwButtonCallback)ChangeRenderMode, NULL, " label='Toggle Render Mode' ");
	TwAddVarRW(bar, "ToggleRenderMode", TW_TYPE_BOOLCPP, &scene.isForward, " label='Toggle Render Mode' true='Forward' false='Deferred' ");

	TwAddSeparator(bar, NULL, NULL);

	// Game objects
	TwAddSeparator(bar, NULL, " group='GameObjects' ");
    TwAddVarCB(bar, "centralModel", TwDefineEnum("CentralModel", NULL, 0),
               SetModel, GetModel, NULL, " enum='0 {Teapot}, 1 {Bunny}, 2 {Dragon}, 3 {Sphere}' group='GameObjects' ");
    TwAddButton(bar, "Spheres", (TwButtonCallback)ToggleSpheres, NULL, " label='Spheres' group='GameObjects' ");
    TwAddButton(bar, "Ground", (TwButtonCallback)ToggleGround, NULL, " label='Ground' group='GameObjects' ");
	TwDefine(" Tweaks/GameObjects opened=false ");

	TwAddSeparator(bar, NULL, " group='DeferredShadingDebug' ");
	TwAddButton(bar, "GBufferPosition", (TwButtonCallback)GBufferPosition, NULL, " label='GBuffer Position' group='DeferredShadingDebug' ");
	TwAddButton(bar, "GBufferNormal", (TwButtonCallback)GBufferNormal, NULL, " label='GBuffer Normals' group='DeferredShadingDebug' ");
	TwAddButton(bar, "GBufferDiffuseXYZ", (TwButtonCallback)GBufferDiffuseXYZ, NULL, " label='GBuffer Diffuse XYZ' group='DeferredShadingDebug' ");
	TwAddButton(bar, "GBufferDiffuseW", (TwButtonCallback)GBufferDiffuseW, NULL, " label='GBuffer Diffuse W' group='DeferredShadingDebug' ");
	TwAddButton(bar, "GBufferSpecular", (TwButtonCallback)GBufferSpecular, NULL, " label='GBuffer Specular' group='DeferredShadingDebug' ");
	TwAddButton(bar, "GBufferClear", (TwButtonCallback)GBufferClear, NULL, " label='GBuffer clear' group='DeferredShadingDebug' ");
	TwAddVarRW(bar, "AmbientColor", TW_TYPE_COLOR3F, &scene.ambientColor, " colormode=rgb ");
	TwDefine(" Tweaks/DeferredShadingDebug opened=false ");


	scene.InitializeLights(16, true, false);

	TwAddSeparator(bar, NULL, " group='LocalLights' ");
	TwAddVarCB(bar, "LightColor", TW_TYPE_COLOR3F, SetLightColor, GetLightColor, NULL, " colorMode=rgb group='LocalLights' ");
	TwAddVarCB(bar, "LightRange", TW_TYPE_FLOAT, NULL, GetLightRange, NULL, " label='Light Range' group='LocalLights' ");
	TwAddVarCB(bar, "LightAttenuationX", TW_TYPE_FLOAT, NULL, GetAttenuationX, NULL, " label='Light AttenuationX' group='LocalLights' ");
	TwAddVarCB(bar, "LightAttenuationY", TW_TYPE_FLOAT, NULL, GetAttenuationY, NULL, " label='Light AttenuationY' group='LocalLights' ");


	std::string lights = " enum=";
	std::string lightString = "'";
	for (unsigned int i = 0; i < scene.localLights.size(); ++i) {
		lightString += std::to_string(i);
		lightString += " {Light";
		lightString += std::to_string(i);
		lightString += "}";
		if (i + 1 != scene.localLights.size())
			lightString += ", ";
	}
	lightString += "' group='LocalLights' ";

	lights += lightString;
	TwDefine(" Tweaks/LocalLights opened=false ");

	TwAddVarCB(bar, "LocalLightList", TwDefineEnum("Light", NULL, 0), SetLight, GetLight, NULL, lights.c_str());

	TwAddSeparator(bar, NULL, " group='ESM' ");
	TwAddVarRW(bar, "ESMCVal", TW_TYPE_FLOAT, &scene.esmCValue, " label='Constant C' group='ESM' ");

	std::string shadowDebugModes = " enum=";
	std::string shadowDebugs = "'";
	for (unsigned int i = 0; i < ShadowDebugMode::SHADOW_DEBUG_COUNT; ++i) {
		shadowDebugs += std::to_string(i);
		shadowDebugs += " {";
		shadowDebugs += ShadowDebugEnumToString((ShadowDebugMode)i);
		shadowDebugs += "}";
		if (i + 1 != ShadowDebugMode::SHADOW_DEBUG_COUNT)
			shadowDebugs += ", ";
	}
	shadowDebugs += "' group='ESM' ";

	shadowDebugModes += shadowDebugs;

	// shadow
	TwAddVarCB(bar, "ShadowDebugModeList", TwDefineEnum("ShadowDebugModes", NULL, 0), SetShadowDebugMode, GetShadowDebugMode, NULL, shadowDebugModes.c_str());
	TwAddVarRW(bar, "ShadowMapToggle", TW_TYPE_BOOLCPP, &scene.isShadowEnabled, " label='Toggle Shadow Map' group='ESM' ");
	TwDefine(" Tweaks/ESM opened=false ");

	// Parallax
	TwAddVarRW(bar, "NormalMapToggle", TW_TYPE_BOOLCPP, &scene.isNormalMapEnabled, " label='Toggle Normal Map' group='ParallaxMapping' ");
	TwAddVarRW(bar, "ParallaxMapToggle", TW_TYPE_BOOLCPP, &scene.isParallaxMapEnabled, " label='Toggle Parallax Map' group='ParallaxMapping' ");
	TwAddVarRW(bar, "HeightScaleParallax", TW_TYPE_FLOAT, &scene.heightScale, " label='Parallax Height Scale' group='ParallaxMapping' step=0.1 ");
	TwAddVarRW(bar, "CropImage", TW_TYPE_BOOLCPP, &scene.cropTextureMap, " label='Crop Parallax Map' group='ParallaxMapping' ");
	TwAddVarRW(bar, "EnhanceVectorP", TW_TYPE_BOOLCPP, &scene.enhanceScaledViewVector, " label='Enhance View Vector' group='ParallaxMapping' ");
	TwAddVarRW(bar, "SteepParallaxMapToggle", TW_TYPE_BOOLCPP, &scene.isSteepParallaxMappingEnabled, " label='Toggle Steep Parallax Map' group='ParallaxMapping' ");
	TwAddVarRW(bar, "ParallaxOcclusionMapToggle", TW_TYPE_BOOLCPP, &scene.isParallaxOcclusionMappingEnabled, " label='Toggle Parallax Occlusion Map' group='ParallaxMapping' ");
	TwAddVarRW(bar, "LayerAmount", TW_TYPE_INT32, &scene.depthLayerAmount, " label='Steep Layer Amount' group='ParallaxMapping' min=1 max=100 ");
	TwAddVarRW(bar, "ParallaxMappingProject", TW_TYPE_BOOLCPP, &scene.isParallaxMappingProject, " label='Parallax Mapping Project' group='ParallaxMapping' ");
	TwDefine(" Tweaks/ParallaxMapping opened=false ");
	
	// SSAO
	TwAddVarRW(bar, "SSAOToggle", TW_TYPE_BOOLCPP, &scene.isSSAOEnabled, " label='Toggle SSAO' group='SSAO' true='Enabled' false='Disabled' ");
	TwAddVarRW(bar, "SSAOBlurToggle", TW_TYPE_BOOLCPP, &scene.isSSAOBlurred, " label='Blur SSAO' group='SSAO' true='Blurred' false='Non-Blurred' ");
	TwAddVarRW(bar, "SSAORADIUS", TW_TYPE_FLOAT, &scene.ssaoRadius, " label='SSAO Radius' group='SSAO' step=0.05  ");
	TwAddSeparator(bar, NULL, NULL);
	TwAddVarRW(bar, "DebugQuadToggle", TW_TYPE_BOOLCPP, &scene.drawDebugQuads, " label='Draw Debug Quads?' ");

    // Initialize our scene
    scene.InitializeScene();

	TwAddVarRW(bar, "GroundShininess", TW_TYPE_FLOAT, &scene.groundPolygons->shininess, " label='Ground Shininess' step=1.0 ");
	TwAddVarRW(bar, "ToggleObject", TW_TYPE_BOOLCPP, &scene.drawObject, " label='Middle Object' ");
	TwAddVarRW(bar, "ToggleTexture", TW_TYPE_BOOLCPP, &scene.brick, " label='Draw Brick' ");
    // Enter the event loop.
    glutMainLoop();

}
