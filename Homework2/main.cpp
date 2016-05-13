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

GLint lightLocGround, lightLocRed, lightLocGreen, lightLocArc;

// View properties
glm::mat4 Projection;
float windowWidth = 1024.0f;
float windowHeight = 768.0f;
int frameBufferWidth = 0;
int frameBufferHeight = 0;
float fov = 45.0f;
float fovy = fov;
int select_viewpoint = 0, number_of_viewpoints = 3, select_object = 0, number_of_objects = 3;
bool isWorldSky = true;

// Model properties
Model ground, redCube, greenCube;
glm::mat4 skyRBT;
glm::mat4 g_objectRbt[2] = {glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 0.5f, 0.0f)) * glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0.0f, 1.0f, 0.0f)), // RBT for redCube
							glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.5f, 0.0f)) * glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 1.0f, 0.0f))}; // RBT for greenCube
glm::mat4 eyeRBT;
glm::mat4 worldRBT = glm::mat4(1.0f);
glm::mat4 aFrame, objectRBT, objectRBTPrev;

// Arcball manipulation
Model arcBall;
glm::mat4 arcballRBT = glm::mat4(1.0f);
float arcBallScreenRadius = 0.25f * min(windowWidth, windowHeight); // for the initial assignment
float arcBallScale = 0.01f, screenToEyeScale, fallBackScale = 0.01f;
vec2 mousePos = vec2(), mousePosPrev = vec2(), arcBallScreenPos = vec2();
vec3 arcBallEyePos = vec3(), mouseSpherePos = vec3();
bool arcBallExists, mousePressed, mouseReleased, mouseFirst, mouseSecond = false,
leftMousePressed = false, rightMousePressed = false, middleMousePressed = false,
leftMouseReleased = false, rightMouseReleased = false, middleMouseReleased = false,
leftMouseFirst = false, rightMouseFirst = false, middleMouseFirst = false;

// Function definition
static void window_size_callback(GLFWwindow*, int, int);
static void mouse_button_callback(GLFWwindow*, int, int, int);
static void cursor_pos_callback(GLFWwindow*, double, double);
static void keyboard_callback(GLFWwindow*, int, int, int, int);
void update_fovy(void);

void printMat4(mat4 source){
	for(int r = 0; r < 4; r++){
		for(int c = 0; c < 4; c++){
			std::cout << source[r][c] << "\t";
		}
		std::cout << std::endl;
	}
}

void copyMat4(mat4 *from, mat4 *to){
	for(int c = 0; c < 4; c++){
		for(int r = 0; r < 4; r++){
			(*to)[r][c] = (*from)[r][c];
		}
	}
}

void updateMouseBools(){
	mousePressed = leftMousePressed || rightMousePressed || middleMousePressed;
	mouseReleased = leftMouseReleased || rightMouseReleased || middleMouseReleased;
	mouseFirst = leftMouseFirst || rightMouseFirst || middleMouseFirst;
}

// Helper function: Update the vertical field-of-view(float fovy in global)
void update_fovy(){
	if(frameBufferWidth >= frameBufferHeight){
		fovy = fov;
	}
	else{
		const float RAD_PER_DEG = 0.5f * glm::pi<float>() / 180.0f;
		fovy = (float) atan2(sin(fov * RAD_PER_DEG) * ((float) frameBufferHeight / (float) frameBufferWidth), cos(fov * RAD_PER_DEG)) / RAD_PER_DEG;
	}
}

// TODO: Modify GLFW window resized callback function
static void window_size_callback(GLFWwindow* window, int width, int height){
	// Get resized size and set to current window
	windowWidth = (float) width;
	windowHeight = (float) height;

	// glViewport accept pixel size, please use glfwGetFramebufferSize rather than window size.
	// window size != framebuffer size
	glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);
	glViewport(0, 0, (GLsizei) frameBufferWidth, (GLsizei) frameBufferHeight);

	update_fovy();

	// Update projection matrix
	Projection = glm::perspective(fov, ((float) frameBufferWidth / (float) frameBufferHeight), 0.1f, 100.0f);

	arcBallScreenRadius = 0.25f * min(frameBufferWidth, frameBufferHeight);
}

// TODO: Fill up GLFW mouse button callback function
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
	if(action == GLFW_PRESS){
		bool arcBallStatus = !arcBallExists || arcBallExists && distance(eye_to_screen(arcBallEyePos, Projection, frameBufferWidth, frameBufferHeight), mousePos) <= arcBallScreenRadius;
		switch(button){
			case GLFW_MOUSE_BUTTON_LEFT:
				leftMousePressed = arcBallStatus;
				leftMouseFirst = arcBallStatus;
				break;
			case GLFW_MOUSE_BUTTON_RIGHT:
				rightMousePressed = arcBallStatus;
				rightMouseFirst = arcBallStatus;
				break;
			case GLFW_MOUSE_BUTTON_MIDDLE:
				middleMousePressed = arcBallStatus;
				middleMouseFirst = arcBallStatus;
				break;
			default:
				break;
		}
	}
	else{
		switch(button){
			case GLFW_MOUSE_BUTTON_LEFT:
				leftMousePressed = false;
				leftMouseReleased = true;
				break;
			case GLFW_MOUSE_BUTTON_RIGHT:
				rightMousePressed = false;
				rightMouseReleased = true;
				break;
			case GLFW_MOUSE_BUTTON_MIDDLE:
				middleMousePressed = false;
				middleMouseReleased = true;
				break;
			default:
				break;
		}
	}
	updateMouseBools();
}

// TODO: Fill up GLFW cursor position callback function
static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos){
	float dx, dy, z;
	quat aQuat = quat();
	vec3 arcBallTrans = vec3(), mouseSpherePosCur;
	vec2 mpc;
	mat4 transformM, transformMInv, transformMInvLinear;
	if(mousePressed){
		if(leftMousePressed && !rightMousePressed && !middleMousePressed){
			if(arcBallExists){
				mpc = mousePos - arcBallScreenPos;
				z = arcBallScreenRadius * arcBallScreenRadius - length2(mpc), z = z > 0.0f ? sqrtf(z) : 0.0f;
			}
			else{
				if(!all(equal(mousePosPrev, vec2()))){
					mpc = mousePos - mousePosPrev;
					z = powf(max(windowWidth, windowHeight) * 0.5f, 2.0f) - length2(mpc), z = z >= 0.0f ? sqrtf(z) : -sqrtf(-z);
				}
			}
			mouseSpherePosCur = !arcBallExists && all(equal(mousePosPrev, vec2())) ? vec3() : normalize(vec3(mpc.x, mpc.y, z));

			if(!all(equal(mouseSpherePos, vec3()))){
				aQuat = quat(0.0f, mouseSpherePosCur) * quat(0.0f, -mouseSpherePos);
				aQuat = angleAxis(angle(aQuat) / 2.0f, axis(aQuat));
			}
		}
		else if(!leftMousePressed && rightMousePressed && !middleMousePressed){
			if(!all(equal(mousePosPrev, vec2()))){
				dx = xpos - mousePosPrev.x, dy = windowHeight - ypos - 1.0f - mousePosPrev.y;
				if(arcBallExists){
					arcBallTrans = vec3(dx, dy, 0.0f) * screenToEyeScale;
				}
				else{
					arcBallTrans = vec3(dx, dy, 0.0f) * -fallBackScale;
				}
			}
		}
		else if(leftMousePressed && rightMousePressed || middleMousePressed){
			if(!all(equal(mousePosPrev, vec2()))){
				dy = windowHeight - ypos - 1.0f - mousePosPrev.y;
				if(arcBallExists){
					arcBallTrans = vec3(0.0f, 0.0f, -dy) * screenToEyeScale;
				}
				else{
					arcBallTrans = vec3(0.0f, 0.0f, -dy) * -fallBackScale;
				}
			}
		}

		if(mouseSecond){
			if(leftMousePressed && !rightMousePressed && !middleMousePressed && !arcBallExists){
				set(mouseSpherePos, mouseSpherePosCur.x, mouseSpherePosCur.y, mouseSpherePosCur.z);
			}
			mouseSecond = false;
		}

		if(mouseFirst){
			set(mouseSpherePos, mouseSpherePosCur.x, mouseSpherePosCur.y, mouseSpherePosCur.z);
			set(mousePosPrev, mousePos.x, mousePos.y);
			leftMouseFirst = rightMouseFirst = middleMouseFirst = false;
			mouseSecond = true;

			switch(select_object){
				case 0:
					copyMat4(&g_objectRbt[0], &objectRBTPrev);
					break;
				case 1:
					copyMat4(&g_objectRbt[1], &objectRBTPrev);
					break;
				case 2:
					copyMat4(&skyRBT, &objectRBTPrev);
					break;
				default:
					break;
			}
		}

		transformM = translate(arcBallTrans) * mat4_cast(aQuat);
		transformMInv = translate(-arcBallTrans) * mat4_cast(inverse(aQuat));
		transformMInvLinear = translate(arcBallTrans) * mat4_cast(inverse(aQuat));

		switch(select_object){
			case 0:
				if(arcBallExists){
					g_objectRbt[0] = aFrame * transformM * inverse(aFrame) * objectRBTPrev;
				}
				else{
					g_objectRbt[0] = aFrame * transformMInvLinear * inverse(aFrame) * objectRBTPrev;
				}
				break;
			case 1:
				if(arcBallExists){
					g_objectRbt[1] = aFrame * transformM * inverse(aFrame) * objectRBTPrev;
				}
				else{
					g_objectRbt[1] = aFrame * transformMInvLinear * inverse(aFrame) * objectRBTPrev;
				}
				break;
			case 2:
				if(arcBallExists){
					skyRBT = aFrame * transformMInv * inverse(aFrame) * objectRBTPrev;
				}
				else{
					skyRBT = aFrame * transformMInvLinear * inverse(aFrame) * objectRBTPrev;
				}
				break;
			default:
				break;
		}
	}
	else if(mouseReleased){
		vec3 tmpv3 = vec3();
		set(mouseSpherePos, tmpv3.x, tmpv3.y, tmpv3.z);
		vec2 tmpv2 = vec2();
		set(mousePosPrev, tmpv2.x, tmpv2.y);
		leftMouseReleased = rightMouseReleased = middleMouseReleased = false;
	}
	set(mousePos, (float) xpos, windowHeight - (float) ypos - 1.0f);
	updateMouseBools();
}

static void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	if(action == GLFW_PRESS){
		switch(key){
			case GLFW_KEY_H:
				std::cout << "CS380 Homework Assignment 2" << std::endl;
				std::cout << "keymaps:" << std::endl;
				std::cout << "h\t\t Help command" << std::endl;
				std::cout << "v\t\t Change eye frame (your viewpoint)" << std::endl;
				std::cout << "o\t\t Change current manipulating object" << std::endl;
				std::cout << "m\t\t Change auxiliary frame between world-sky and sky-sky" << std::endl;
				std::cout << "c\t\t Change manipulation method" << std::endl;
				break;
			case GLFW_KEY_V:
				// TODO: Change viewpoint
				select_viewpoint = (select_viewpoint + 1) % number_of_viewpoints;
				break;
			case GLFW_KEY_O:
				// TODO: Change manipulating object
				select_object = (select_object + 1) % number_of_objects;
				break;
			case GLFW_KEY_M:
				// TODO: Change auxiliary frame between world-sky and sky-sky
				isWorldSky = !isWorldSky;
				break;
			case GLFW_KEY_C:
				// TODO: Add an additional manipulation method
				break;
			default:
				break;
		}
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
	window = glfwCreateWindow((int) windowWidth, (int) windowHeight, "Homework 2: ", NULL, NULL);
	if(window == NULL){
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = (GLboolean) true; // Needed for core profile
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
	// Update arcBallScreenRadius with framebuffer size
	arcBallScreenRadius = 0.25f * min((float) frameBufferWidth, (float) frameBufferHeight); // for the initial assignment

	// Clear with sky color
	glClearColor((GLclampf) (128. / 255.), (GLclampf) (200. / 255.), (GLclampf) (255. / 255.), (GLclampf) 0.);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Enable culling
	glEnable(GL_CULL_FACE);
	// Backface culling
	glCullFace(GL_BACK);

	Projection = glm::perspective(fov, ((float) frameBufferWidth / (float) frameBufferHeight), 0.1f, 100.0f);
	skyRBT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.25, 4.0));

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

	// initialize the red cube model
	redCube = Model();
	init_cube(redCube, glm::vec3(1.0, 0.0, 0.0));
	redCube.initialize(DRAW_TYPE::ARRAY, "VertexShader.glsl", "FragmentShader.glsl");
	redCube.set_projection(&Projection);
	redCube.set_eye(&eyeRBT);
	redCube.set_model(&g_objectRbt[0]);

	// initialize the green cube model
	greenCube = Model();
	init_cube(greenCube, glm::vec3(0.0, 1.0, 0.0));
	greenCube.initialize(DRAW_TYPE::ARRAY, "VertexShader.glsl", "FragmentShader.glsl");
	greenCube.set_projection(&Projection);
	greenCube.set_eye(&eyeRBT);
	greenCube.set_model(&g_objectRbt[1]);

	// TODO: Initialize arcBall
	// Initialize your arcBall with DRAW_TYPE::INDEX (it uses GL_ELEMENT_ARRAY_BUFFER to draw sphere)
	arcBall = Model();
	init_sphere(arcBall);
	arcBall.initialize(DRAW_TYPE::INDEX, "VertexShader.glsl", "FragmentShader.glsl");
	arcBall.set_projection(&Projection);
	arcBall.set_eye(&eyeRBT);
	arcBall.set_model(&arcballRBT);

	// Setting Light Vectors
	glm::vec3 lightVec = glm::vec3(0.0f, 1.0f, 0.0f);
	lightLocGround = glGetUniformLocation(ground.GLSLProgramID, "uLight");
	glUniform3f(lightLocGround, lightVec.x, lightVec.y, lightVec.z);

	lightLocRed = glGetUniformLocation(redCube.GLSLProgramID, "uLight");
	glUniform3f(lightLocRed, lightVec.x, lightVec.y, lightVec.z);

	lightLocGreen = glGetUniformLocation(greenCube.GLSLProgramID, "uLight");
	glUniform3f(lightLocGreen, lightVec.x, lightVec.y, lightVec.z);

	lightLocArc = glGetUniformLocation(arcBall.GLSLProgramID, "uLight");
	glUniform3f(lightLocArc, lightVec.x, lightVec.y, lightVec.z);

	do{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// TODO: Change Viewpoint with respect to your current view index
		eyeRBT = select_viewpoint == 0 ? skyRBT : select_viewpoint == 1 ? g_objectRbt[0] : g_objectRbt[1];

		switch(select_object){
			case 0:
				objectRBT = g_objectRbt[0];
				break;
			case 1:
				objectRBT = g_objectRbt[1];
				break;
			case 2:
				if(select_viewpoint == 0){
					objectRBT = skyRBT;
				}
				else{
					select_object = 0;
					objectRBT = g_objectRbt[0];
				}
				break;
			default:
				break;
		}
		if(select_viewpoint == 0 && select_object == 2){
			if(isWorldSky){
				aFrame = worldRBT * linearFact(eyeRBT);
			}
			else{
				aFrame = worldRBT * eyeRBT;
			}
		}
		else{
			aFrame = worldRBT * transFact(objectRBT) * linearFact(eyeRBT);
		}
		if(select_viewpoint != (select_object + 1) % number_of_objects){
			arcBallExists = true;
		}
		else{
			if(select_viewpoint == 0 && isWorldSky){
				arcBallExists = true;
			}
			else{
				arcBallExists = false;
			}
		}

		// TODO: Draw wireframe of arcball with dynamic radius
		if(arcBallExists){
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			mat4 eyeToAFrame = inverse(eyeRBT) * aFrame;
			screenToEyeScale = compute_screen_eye_scale(eyeToAFrame[3][2], fovy, frameBufferHeight);
			if(!(leftMousePressed && rightMousePressed || middleMousePressed)){
				arcBallScale = arcBallScreenRadius * screenToEyeScale;
			}
			arcballRBT = aFrame * scale(vec3(arcBallScale));
			set(arcBallEyePos, eyeToAFrame[3][0], eyeToAFrame[3][1], eyeToAFrame[3][2]);
			arcBallScreenPos = eye_to_screen(arcBallEyePos, Projection, frameBufferWidth, frameBufferHeight);
			arcBall.draw();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		redCube.draw();
		greenCube.draw();
		ground.draw();

		// Swap buffers (Double buffering)
		glfwSwapBuffers(window);
		glfwPollEvents();
	} // Check if the ESC key was pressed or the window was closed
	while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		  glfwWindowShouldClose(window) == 0);

	  // Clean up data structures and glsl objects
	ground.cleanup();
	redCube.cleanup();
	greenCube.cleanup();
	arcBall.cleanup();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
