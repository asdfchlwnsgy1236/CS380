// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/affine.hpp>
#include <common/geometry.hpp>
#include <common/arcball.hpp>

float g_groundSize = 100.0f;
float g_groundY = -2.5f;

// light property variables
float dlIntensity, plIntensity, plAttenuationRatio, sIntensity, sAttenuationRatio, sConeAngle; // sConeAngle needs to be in radians
vec3 dlColor, dlDirection, plColor, plLocation, sColor, sLocation, sDirection;

// light property constants
const float startIntensities[3] = {1.0f, 1.0f, 1.0f},
startMisc[3] = {0.25f, 0.01f, radians(3.5f)};
const vec3 startColors[3] = {vec3(1.0f), vec3(1.0f), vec3(1.0f)},
startLocsDirs[4] = {vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 2.0f, 0.0f), vec3(0.0f, 10.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)};

// light animation variables
float plStartTime, plPauseTime, sStartTime, sPauseTime, dlRotation = 60.0f;
bool dlRainbow = false, plRainbow = false, sRainbow = false, playing = true;
int dlRainbowPhase = 0, plRainbowPhase = 0, sRainbowPhase = 0;

// View properties
glm::mat4 Projection;
float windowWidth = 1024.0f;
float windowHeight = 768.0f;
int frameBufferWidth = 0;
int frameBufferHeight = 0;
float fov = 45.0f;
float fovy = fov;

// Model properties
Model ground, object[3];
glm::mat4 skyRBT;
glm::mat4 eyeRBT;
const glm::mat4 worldRBT = glm::mat4(1.0f);
glm::mat4 objectRBT[3] = {translate(vec3(-1.0f, 0.0f, 0.0f)) * rotate(10.0f, vec3(1.0f, 0.0f, 0.0f)) * scale(5.0f, 5.0f, 5.0f),
translate(vec3(0.0f, 0.0f, 0.0f)) * rotate(0.0f, vec3(1.0f, 0.0f, 0.0f)) * scale(5.0f, 5.0f, 5.0f),
translate(vec3(1.0f, 0.0f, 0.0f)) * rotate(-10.0f, vec3(1.0f, 0.0f, 0.0f)) * scale(5.0f, 5.0f, 5.0f)};
glm::mat4 arcballRBT = glm::mat4(1.0f);
glm::mat4 aFrame;

// Mouse interaction
bool MOUSE_LEFT_PRESS = false; bool MOUSE_MIDDLE_PRESS = false; bool MOUSE_RIGHT_PRESS = false;

// Transformation
glm::mat4 m = glm::mat4(1.0f);

// Manipulation index
int object_index = 0; int view_index = 0; int sky_type = 0;
int colorSet = 0;

// Arcball manipulation
Model arcBall;
float arcBallScreenRadius = 0.25f * min(windowWidth, windowHeight);
float arcBallScale = 0.01f; float ScreenToEyeScale = 0.01f;
float prev_x = 0.0f; float prev_y = 0.0f;

static bool non_ego_cube_manipulation(){
	return object_index != 0 && view_index != object_index;
}

static bool use_arcball(){
	return (object_index == 0 && sky_type == 0) || non_ego_cube_manipulation();
}

static void window_size_callback(GLFWwindow* window, int width, int height){
	// Get resized size and set to current window
	windowWidth = (float) width;
	windowHeight = (float) height;

	// glViewport accept pixel size, please use glfwGetFramebufferSize rather than window size.
	// window size != framebuffer size
	glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);
	glViewport(0, 0, frameBufferWidth, frameBufferHeight);

	arcBallScreenRadius = 0.25f * min(frameBufferWidth, frameBufferHeight);

	if(frameBufferWidth >= frameBufferHeight){
		fovy = fov;
	}
	else{
		const float RAD_PER_DEG = 0.5f * glm::pi<float>() / 180.0f;
		fovy = atan2(sin(fov * RAD_PER_DEG) * (float) frameBufferHeight / (float) frameBufferWidth, cos(fov * RAD_PER_DEG)) / RAD_PER_DEG;
	}

	// Update projection matrix
	Projection = glm::perspective(fov, windowWidth / windowHeight, 0.1f, 100.0f);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
	MOUSE_LEFT_PRESS |= (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS);
	MOUSE_RIGHT_PRESS |= (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS);
	MOUSE_MIDDLE_PRESS |= (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS);

	MOUSE_LEFT_PRESS &= !(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE);
	MOUSE_RIGHT_PRESS &= !(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE);
	MOUSE_MIDDLE_PRESS &= !(button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE);

	if(action == GLFW_RELEASE){
		prev_x = 0.0f; prev_y = 0.0f;
	}

	if(action == GLFW_PRESS){
		//GLfloat tmpf = 2.0f, *tmp0 = &tmpf, *tmp1 = &tmpf, *tmp2 = &tmpf, *tmp3 = &tmpf, *tmp4 = &tmpf;
		//glGetUniformfv(ground.GLSLProgramID, glGetUniformLocation(ground.GLSLProgramID, "lightFloats"), tmp0);
		//glGetUniformfv(object[0].GLSLProgramID, glGetUniformLocation(object[0].GLSLProgramID, "lightFloats"), tmp1);
		//glGetUniformfv(object[1].GLSLProgramID, glGetUniformLocation(object[0].GLSLProgramID, "lightFloats"), tmp2);
		//glGetUniformfv(object[2].GLSLProgramID, glGetUniformLocation(object[0].GLSLProgramID, "lightFloats"), tmp3);
		//glGetUniformfv(arcBall.GLSLProgramID, glGetUniformLocation(arcBall.GLSLProgramID, "lightFloats"), tmp4);
		//std::cout << *tmp0 << " " << *tmp1 << " " << *tmp2 << " " << *tmp3 << " " << *tmp4 << std::endl;
	}
}

void setWrtFrame(){
	switch(object_index){
		case 0:
			// world-sky: transFact(worldRBT) * linearFact(skyRBT), sky-sky: transFact(skyRBT) * linearFact(skyRBT)
			aFrame = (sky_type == 0) ? linearFact(skyRBT) : skyRBT;
			break;
		case 1:
			aFrame = transFact(objectRBT[1]) * linearFact(eyeRBT);
			break;
	}
}

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos){
	if(view_index != 0 && object_index == 0) return;
	// Convert mouse pointer into screen space. (http://gamedev.stackexchange.com/questions/83570/why-is-the-origin-in-computer-graphics-coordinates-at-the-top-left)
	xpos = xpos * ((float) frameBufferWidth / windowWidth);
	ypos = (float) frameBufferHeight - ypos * ((float) frameBufferHeight / windowHeight) - 1.0f;

	double dx_t = xpos - prev_x;
	double dy_t = ypos - prev_y;
	double dx_r = xpos - prev_x;
	double dy_r = ypos - prev_y;

	if(view_index == 0 && object_index == 0){
		if(sky_type == 0){
			dx_t = -dx_t; dy_t = -dy_t; dx_r = -dx_r; dy_r = -dy_r;
		}
		else{
			dx_r = -dx_r; dy_r = -dy_r;
		}
	}

	if(MOUSE_LEFT_PRESS){
		if(prev_x - 1e-16 < 1e-8 && prev_y - 1e-16 < 1e-8){
			prev_x = (float) xpos; prev_y = (float) ypos;
			return;
		}

		if(use_arcball()){
			// 1. Get eye coordinate of arcball and compute its screen coordinate
			glm::vec4 arcball_eyecoord = glm::inverse(eyeRBT) * arcballRBT * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			glm::vec2 arcballCenter = eye_to_screen(
				glm::vec3(arcball_eyecoord),
				Projection,
				frameBufferWidth,
				frameBufferHeight
			);

		// compute z index
			glm::vec2 p1 = glm::vec2(prev_x, prev_y) - arcballCenter;
			glm::vec2 p2 = glm::vec2(xpos, ypos) - arcballCenter;

			glm::vec3 v1 = glm::normalize(glm::vec3(p1.x, p1.y, sqrt(max(0.0f, pow(arcBallScreenRadius, 2) - pow(p1.x, 2) - pow(p1.y, 2)))));
			glm::vec3 v2 = glm::normalize(glm::vec3(p2.x, p2.y, sqrt(max(0.0f, pow(arcBallScreenRadius, 2) - pow(p2.x, 2) - pow(p2.y, 2)))));

			glm::quat w1, w2;
			// 2. Compute arcball rotation (Chatper 8)
			if(object_index == 0 && view_index == 0 && sky_type == 0){
				w1 = glm::quat(0.0f, -v1); w2 = glm::quat(0.0f, v2);
			}
			else{
				w1 = glm::quat(0.0f, v2); w2 = glm::quat(0.0f, -v1);
			}

// Arcball: axis k and 2*theta (Chatper 8)
			glm::quat w = w1 * w2;
			m = glm::toMat4(w);
		}
		else // ego motion
		{
			glm::quat xRotation = glm::angleAxis((float) -dy_r * 0.1f, glm::vec3(1.0f, 0.0f, 0.0f));
			glm::quat yRotation = glm::angleAxis((float) dx_r * 0.1f, glm::vec3(0.0f, 1.0f, 0.0f));

			glm::quat w = yRotation * xRotation;
			m = glm::toMat4(w);
		}

		// Apply transformation with auxiliary frame
		setWrtFrame();
		if(object_index == 0){
			skyRBT = aFrame * m * glm::inverse(aFrame) * skyRBT;
		}
		else{
			objectRBT[1] = aFrame * m * glm::inverse(aFrame) * objectRBT[1];
		}

		prev_x = (float) xpos; prev_y = (float) ypos;
	}
}

void toggleEyeMode(){
	view_index = (view_index + 1) % 2;
	if(view_index == 0){
		std::cout << "Using sky view" << std::endl;
	}
	else{
		std::cout << "Using object " << view_index << " view" << std::endl;
	}
}

void cycleManipulation(){
	object_index = (object_index + 1) % 2;
	if(object_index == 0){
		std::cout << "Manipulating sky frame" << std::endl;
	}
	else{
		std::cout << "Manipulating object " << object_index << std::endl;
	}
}

void cycleSkyAMatrix(){
	if(object_index == 0 && view_index == 0){
		sky_type = (sky_type + 1) % 2;
		if(sky_type == 0){
			std::cout << "world-sky" << std::endl;
		}
		else{
			std::cout << "sky-sky" << std::endl;
		}
	}
	else{
		std::cout << "Unable to change sky mode" << std::endl;
	}
}

static void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	glm::mat4 m;
	if(action == GLFW_PRESS){
		switch(key){
			case GLFW_KEY_H:
				std::cout << "CS380 Homework Assignment 2" << std::endl;
				std::cout << "keymaps:" << std::endl;
				std::cout << "h\t Help command" << std::endl;
				std::cout << "p\t Pause/resume all light animations (camera can still move)" << std::endl;
				std::cout << "c\t Cycle through different color sets for the bunnies." << std::endl;
				std::cout << "1\t Toggle directional light on/off state" << std::endl;
				std::cout << "2\t Toggle point light on/off state" << std::endl;
				std::cout << "3\t Toggle spotlight on/off state" << std::endl;
				std::cout << "4\t Toggle all lights on/off state" << std::endl;
				std::cout << "5\t Toggle directional light normal/rainbow mode" << std::endl;
				std::cout << "6\t Toggle point light normal/rainbow mode" << std::endl;
				std::cout << "7\t Toggle spotlight normal/rainbow mode" << std::endl;
				std::cout << "8\t Toggle all lights normal/rainbow mode" << std::endl;
				std::cout << "9\t Toggle all lights on/off state and normal/rainbow mode" << std::endl;
				break;
			case GLFW_KEY_P:
				playing = playing ?
					(plPauseTime = sPauseTime = glfwGetTime(), std::cout << "Light animation paused." << std::endl, false) :
					(plStartTime += glfwGetTime() - plPauseTime, sStartTime += glfwGetTime() - sPauseTime, std::cout << "Light animation resumed." << std::endl, true);
				break;
			case GLFW_KEY_C:
				colorSet = ++colorSet % 4;
				switch(colorSet){
					case 0:
						for(int a = 0; a < 3; a++){
							object[a].recolor(vec3(0.1f, 0.3f, 1.0f)), std::cout << "Recoloring object " << a << " to the original greenish blue color." << std::endl;
						}
						break;
					case 1:
						object[0].recolor(vec3(1.0f, 0.0f, 0.0f)), std::cout << "Recoloring object 0 to red." << std::endl;
						object[1].recolor(vec3(0.0f, 0.0f, 1.0f)), std::cout << "Recoloring object 1 to blue." << std::endl;
						object[2].recolor(vec3(0.0f, 1.0f, 0.0f)), std::cout << "Recoloring object 2 to green." << std::endl;
						break;
					case 2:
						object[0].recolor(vec3(1.0f, 0.0f, 1.0f)), std::cout << "Recoloring object 0 to magenta." << std::endl;
						object[1].recolor(vec3(0.0f, 1.0f, 1.0f)), std::cout << "Recoloring object 1 to cyan." << std::endl;
						object[2].recolor(vec3(1.0f, 1.0f, 0.0f)), std::cout << "Recoloring object 2 to yellow." << std::endl;
						break;
					case 3:
						object[0].recolor(vec3(0.5f, 0.5f, 0.5f)), std::cout << "Recoloring object 0 to gray." << std::endl;
						object[1].recolor(vec3(0.0f, 0.0f, 0.0f)), std::cout << "Recoloring object 1 to complete darkness." << std::endl;
						object[2].recolor(vec3(1.0f, 1.0f, 1.0f)), std::cout << "Recoloring object 2 to white." << std::endl;
						break;
					default:
						break;
				}
				break;
			case GLFW_KEY_1:
				dlIntensity = dlIntensity > 0.0f ?
					(dlDirection = startLocsDirs[0], std::cout << "Directional light turned off." << std::endl, 0.0f) :
					(std::cout << "Directional light turned on." << std::endl, startIntensities[0]);
				break;
			case GLFW_KEY_2:
				plIntensity = plIntensity > 0.0f ?
					(plLocation = startLocsDirs[1], std::cout << "Point light turned off." << std::endl, 0.0f) :
					(plStartTime = glfwGetTime(), std::cout << "Point light turned on." << std::endl, startIntensities[1]);
				break;
			case GLFW_KEY_3:
				sIntensity = sIntensity > 0.0f ?
					(sLocation = startLocsDirs[2], sDirection = startLocsDirs[3], std::cout << "Spotlight turned off." << std::endl, 0.0f) :
					(sStartTime = glfwGetTime(), std::cout << "Spotlight turned on." << std::endl, startIntensities[2]);
				break;
			case GLFW_KEY_4:
				if(dlIntensity > 0.0f && plIntensity > 0.0f && sIntensity > 0.0f){
					dlIntensity = plIntensity = sIntensity = 0.0f,
						dlDirection = startLocsDirs[0], plLocation = startLocsDirs[1], sLocation = startLocsDirs[2], sDirection = startLocsDirs[3],
						std::cout << "All lights turned off." << std::endl;
				}
				else{
					dlIntensity = startIntensities[0], plIntensity = startIntensities[1], sIntensity = startIntensities[2],
						plStartTime = sStartTime = glfwGetTime(),
						std::cout << "All lights turned on." << std::endl;
				}
				break;
			case GLFW_KEY_5:
				dlRainbow = dlRainbow ?
					(dlColor = startColors[0], std::cout << "Directional light mode changed to normal." << std::endl, false) :
					(std::cout << "Directional light mode changed to rainbow." << std::endl, true);
				break;
			case GLFW_KEY_6:
				plRainbow = plRainbow ?
					(plColor = startColors[1], std::cout << "Point light mode changed to normal." << std::endl, false) :
					(std::cout << "Point light mode changed to rainbow." << std::endl, true);
				break;
			case GLFW_KEY_7:
				sRainbow = sRainbow ?
					(sColor = startColors[2], std::cout << "Spotlight mode changed to normal." << std::endl, false) :
					(std::cout << "Spotlight mode changed to rainbow." << std::endl, true);
				break;
			case GLFW_KEY_8:
				if(dlRainbow && plRainbow && sRainbow){
					dlRainbow = plRainbow = sRainbow = false,
						dlColor = startColors[0], plColor = startColors[1], sColor = startColors[2],
						std::cout << "All lights' modes changed to normal." << std::endl;
				}
				else{
					dlRainbow = plRainbow = sRainbow = true, std::cout << "All lights' modes changed to rainbow." << std::endl;
				}
				break;
			case GLFW_KEY_9:
				if(dlIntensity > 0.0f && plIntensity > 0.0f && sIntensity > 0.0f && dlRainbow && plRainbow && sRainbow){
					dlIntensity = plIntensity = sIntensity = 0.0f,
						dlDirection = startLocsDirs[0], plLocation = startLocsDirs[1], sLocation = startLocsDirs[2], sDirection = startLocsDirs[3],
						dlRainbow = plRainbow = sRainbow = false,
						dlColor = startColors[0], plColor = startColors[1], sColor = startColors[2],
						std::cout << "All lights turned off and their modes changed to normal." << std::endl;
				}
				else{
					dlIntensity = startIntensities[0], plIntensity = startIntensities[1], sIntensity = startIntensities[2],
						plStartTime = sStartTime = glfwGetTime(),
						dlRainbow = plRainbow = sRainbow = true,
						std::cout << "All lights turned on and their modes changed to rainbow." << std::endl;
				}
				break;
			default:
				break;
		}
	}
}

void iterateRainbow(int *rp, vec3 *tc, float dt){
	switch(*rp){
		case 0:
			(*tc).y -= dt, (*tc).z -= dt, *rp += (*tc).y <= 0.0f && (*tc).z <= 0.0f ? ((*tc).y = 0.0f, (*tc).z = 0.0f, 1) : 0;
			break;
		case 1:
			(*tc).x += dt, *rp += (*tc).x >= 1.0f ? ((*tc).x = 1.0f, 1) : 0;
			break;
		case 2:
			(*tc).z -= dt, *rp += (*tc).z <= 0.0f ? ((*tc).z = 0.0f, 1) : 0;
			break;
		case 3:
			(*tc).y += dt, *rp += (*tc).y >= 1.0f ? ((*tc).y = 1.0f, 1) : 0;
			break;
		case 4:
			(*tc).x -= dt, *rp += (*tc).x <= 0.0f ? ((*tc).x = 0.0f, 1) : 0;
			break;
		case 5:
			(*tc).z += dt, *rp += (*tc).z >= 1.0f ? ((*tc).z = 1.0f, 1) : 0;
			break;
		case 6:
			(*tc).y -= dt, *rp += (*tc).y <= 0.0f ? ((*tc).y = 0.0f, -5) : 0;
			break;
		default:
			break;
	}
}

void setLightUniforms(float dli, float pli, float plar, float si, float sar, float sca,
					  vec3 dlc, vec3 dld, vec3 plc, vec3 pll, vec3 sc, vec3 sl, vec3 sd){
	Model models[5] = {ground, object[0], object[1], object[2], arcBall};
	GLfloat lightFloats[6] = {dli, pli, plar, si, sar, sca};
	GLfloat lightVec3s[7 * 3] = {dlc.x, dlc.y, dlc.z, dld.x, dld.y, dld.z,
		plc.x, plc.y, plc.z, pll.x, pll.y, pll.z,
		sc.x, sc.y, sc.z, sl.x, sl.y, sl.z, sd.x, sd.y, sd.z};

	for(int a = 0; a < 5; a++){
		glProgramUniform1fv(models[a].GLSLProgramID, glGetUniformLocation(models[a].GLSLProgramID, "lightFloats"), 6, lightFloats);
		glProgramUniform3fv(models[a].GLSLProgramID, glGetUniformLocation(models[a].GLSLProgramID, "lightVec3s"), 7, lightVec3s);
	}
}

int main(void){
	// Initialise GLFW
	if(!glfwInit()){
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow((int) windowWidth, (int) windowHeight, "Lab 3", NULL, NULL);
	if(window == NULL){
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if(glewInit() != GLEW_OK){
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetKeyCallback(window, keyboard_callback);

	glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

	// Clear with sky color
	glClearColor((GLclampf) (128. / 255.), (GLclampf) (200. / 255.), (GLclampf) (255. / 255.), (GLclampf) 0.);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	Projection = glm::perspective(fov, windowWidth / windowHeight, 0.1f, 100.0f);
	skyRBT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.25f, 4.0f));

	aFrame = linearFact(skyRBT);

	// initial eye frame = sky frame;
	eyeRBT = skyRBT;

	// Initialize Ground Model
	ground = Model();
	init_ground(ground);
	ground.initialize(DRAW_TYPE::ARRAY, "VertexShader.glsl", "FragmentShader.glsl");
	ground.set_projection(&Projection);
	ground.set_eye(&eyeRBT);
	glm::mat4 groundRBT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, g_groundY, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(g_groundSize, 1.0f, g_groundSize));
	ground.set_model(&groundRBT);

	//TODO: Initialize models by loading .obj file
	for(int a = 0; a < 3; a++){
		object[a] = Model();
		init_obj(object[a], "bunny.obj", vec3(0.1f, 0.3f, 1.0f));
		object[a].set_projection(&Projection);
		object[a].set_eye(&eyeRBT);
		object[a].set_model(&objectRBT[a]);
	}
	object[0].initialize(DRAW_TYPE::ARRAY, "VertexShader0.glsl", "FragmentShader0.glsl");
	object[1].initialize(DRAW_TYPE::ARRAY, "VertexShader1.glsl", "FragmentShader1.glsl");
	object[2].initialize(DRAW_TYPE::ARRAY, "VertexShader2.glsl", "FragmentShader2.glsl");

	arcBall = Model();
	init_sphere(arcBall);
	arcBall.initialize(DRAW_TYPE::INDEX, "VertexShader_.glsl", "FragmentShader_.glsl");
	arcBall.set_projection(&Projection);
	arcBall.set_eye(&eyeRBT);
	arcBall.set_model(&arcballRBT);

	//TODO Setting Light Vectors
	dlIntensity = 0.0f, dlColor = startColors[0], dlDirection = startLocsDirs[0];
	plIntensity = 0.0f, plAttenuationRatio = startMisc[0], plColor = startColors[1], plLocation = startLocsDirs[1];
	sIntensity = 0.0f, sAttenuationRatio = startMisc[1], sConeAngle = startMisc[2], sColor = startColors[2], sLocation = startLocsDirs[2], sDirection = startLocsDirs[3];

	float currtime = glfwGetTime(), prevtime = currtime, deltatime = currtime - prevtime;
	do{
		currtime = glfwGetTime(), deltatime = currtime - prevtime, prevtime = currtime;

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		eyeRBT = (view_index == 0) ? skyRBT : objectRBT[1];

		// Mess with the light variables before passing them to the shaders when playing animations
		if(playing){
			if(dlIntensity > 0.0f){
				if(dlRainbow){
					iterateRainbow(&dlRainbowPhase, &dlColor, deltatime);
				}
				float tmpa = degrees(acosf(dot(startLocsDirs[0], dlDirection)));
				if(tmpa >= 60.0f){
					dlRotation *= -1.0f;
					dlDirection = rotateZ(dlDirection, dlRotation * deltatime / 2.0f);
				}
				dlDirection = rotateZ(dlDirection, dlRotation * deltatime);
			}
			if(plIntensity > 0.0f){
				if(plRainbow){
					iterateRainbow(&plRainbowPhase, &plColor, deltatime);
				}
				float pltime = (currtime - plStartTime) * 2.0f;
				plLocation = vec3(plLocation.x, 2.0f * sinf(pltime), 2.0f * cosf(pltime));
			}
			if(sIntensity > 0.0f){
				if(sRainbow){
					iterateRainbow(&sRainbowPhase, &sColor, deltatime);
				}
				float stime = (currtime - sStartTime) * 2.0f;
				sDirection = vec3(sqrtf(2) * cosf(stime) / (powf(sinf(stime), 2.0f) + 1.0f), -sLocation.y, sqrtf(2) * cosf(stime) * sinf(stime) / (powf(sinf(stime), 2.0f) + 1.0f));
			}

			// Normalize the direction vectors after they have been messed with before passing them to the shaders
			if(length2(dlDirection) > 0.0f){
				dlDirection = normalize(dlDirection);
			}
			else{
				dlIntensity = 0.0f;
			}
			if(length2(sDirection) > 0.0f){
				sDirection = normalize(sDirection);
			}
			else{
				sIntensity = 0.0f;
			}
		}

		// TODO: pass the light value to the shader
		setLightUniforms(dlIntensity, plIntensity, plAttenuationRatio, sIntensity, sAttenuationRatio, sConeAngle,
						 dlColor, dlDirection, plColor, plLocation, sColor, sLocation, sDirection);

		// TODO: draw OBJ model
		for(int a = 0; a < 3; a++){
			object[a].draw();
		}

		// Draw wire frame of arcBall with dynamic radius
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		switch(object_index){
			case 0:
				arcballRBT = (sky_type == 0) ? worldRBT : skyRBT;
				break;
			case 1:
				arcballRBT = objectRBT[1];
				break;
			default:
				break;
		}

		ScreenToEyeScale = compute_screen_eye_scale(
			(glm::inverse(eyeRBT) * arcballRBT * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)).z,
			fovy,
			frameBufferHeight
		);
		arcBallScale = ScreenToEyeScale * arcBallScreenRadius;
		arcballRBT = arcballRBT * glm::scale(worldRBT, glm::vec3(arcBallScale, arcBallScale, arcBallScale));
		//arcBall.draw();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		ground.draw();
		// Swap buffers (Double buffering)
		glfwSwapBuffers(window);
		glfwPollEvents();
	} // Check if the ESC key was pressed or the window was closed
	while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		  glfwWindowShouldClose(window) == 0);

	  // Clean up data structures and glsl objects
	ground.cleanup();
	for(int a = 0; a < 3; a++){
		object[a].cleanup();
	}
	arcBall.cleanup();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
