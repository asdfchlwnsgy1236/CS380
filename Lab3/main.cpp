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

float dlIntensity, plIntensity, plAttenuationRatio, sIntensity, sAttenuationRatio, sConeAngle; // sConeAngle needs to be in radians
vec3 dlColor, dlDirection, plColor, plLocation, sColor, sLocation, sDirection;

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

	//GLfloat tmpf = 2.0f, *tmp0 = &tmpf, *tmp1 = &tmpf, *tmp2 = &tmpf, *tmp3 = &tmpf, *tmp4 = &tmpf;
	//glGetUniformfv(ground.GLSLProgramID, glGetUniformLocation(ground.GLSLProgramID, "lightFloats"), tmp0);
	//glGetUniformfv(object[0].GLSLProgramID, glGetUniformLocation(object[0].GLSLProgramID, "lightFloats"), tmp1);
	//glGetUniformfv(object[1].GLSLProgramID, glGetUniformLocation(object[0].GLSLProgramID, "lightFloats"), tmp2);
	//glGetUniformfv(object[2].GLSLProgramID, glGetUniformLocation(object[0].GLSLProgramID, "lightFloats"), tmp3);
	//glGetUniformfv(arcBall.GLSLProgramID, glGetUniformLocation(arcBall.GLSLProgramID, "lightFloats"), tmp4);
	//std::cout << *tmp0 << " " << *tmp1 << " " << *tmp2 << " " << *tmp3 << " " << *tmp4 << std::endl;
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
				std::cout << "h\t\t Help command" << std::endl;
				std::cout << "v\t\t Change eye matrix" << std::endl;
				std::cout << "o\t\t Change current manipulating object" << std::endl;
				std::cout << "m\t\t Change auxiliary frame between world-sky and sky-sky" << std::endl;
				break;
			case GLFW_KEY_V:

				break;
			case GLFW_KEY_O:

				break;
			case GLFW_KEY_M:

				break;
			default:
				break;
		}
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
	skyRBT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.25, 4.0));

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
	//dlIntensity = 1.0f, dlColor = vec3(1.0f), dlDirection = vec3(0.0f, -1.0f, 0.0f);
	//plIntensity = 1.0f, plAttenuationRatio = 0.25f, plColor = vec3(1.0f), plLocation = vec3(0.0f, 1.0f, 0.0f);
	sIntensity = 1.0f, sAttenuationRatio = 0.01f, sConeAngle = radians(60.0f), sColor = vec3(1.0f), sLocation = vec3(0.0f, 10.0f, 0.0f), sDirection = vec3(0.0f, -1.0f, 0.0f);
	setLightUniforms(dlIntensity, plIntensity, plAttenuationRatio, sIntensity, sAttenuationRatio, sConeAngle,
					 dlColor, dlDirection, plColor, plLocation, sColor, sLocation, sDirection);

	do{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		eyeRBT = (view_index == 0) ? skyRBT : objectRBT[1];

		//TODO: pass the light value to the shader
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
		setLightUniforms(dlIntensity, plIntensity, plAttenuationRatio, sIntensity, sAttenuationRatio, sConeAngle,
						 dlColor, dlDirection, plColor, plLocation, sColor, sLocation, sDirection);

		// TODO: draw OBJ model
		for(int a = 0; a < 3; a++){
			object[a].draw();
		}

		// Draw wireframe of arcBall with dynamic radius
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
