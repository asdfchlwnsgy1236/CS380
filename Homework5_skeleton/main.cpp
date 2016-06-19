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

#include <common/shader.hpp>
#include <common/affine.hpp>
#include <common/geometry.hpp>
#include <common/arcball.hpp>
#include <common/texture.hpp>

using namespace glm;

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

GLuint isSky, isEye;

GLuint addPrograms[3];
GLuint texture[9];
GLuint textureID[3][9];
GLuint bumpTex;
GLuint bumpTexID;
GLuint cubeTex;
GLuint cubeTexID;

// View properties
glm::mat4 Projection;
float windowWidth = 1024.0f;
float windowHeight = 768.0f;
int frameBufferWidth = 0;
int frameBufferHeight = 0;
float fov = 45.0f;
float fovy = fov;

// Model properties
glm::mat4 skyRBT;
glm::mat4 eyeRBT;
const glm::mat4 worldRBT = glm::mat4(1.0f);
glm::mat4 arcballRBT = glm::mat4(1.0f);
glm::mat4 aFrame;
// cubes
glm::mat4 objectRBT[9];
Model cubes[9];
int program_cnt = 1;
// cube animation
bool rot_first_col = false;

// Sky box
Model skybox;
glm::mat4 skyboxRBT = glm::translate(0.0f, 0.0f, 0.0f);// Will be fixed(cause it is the sky)

vec3 eyePosition = vec3(0.0, 0.25, 6.0);
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

GLenum  cube[6] = {GL_TEXTURE_CUBE_MAP_POSITIVE_X,
GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};

void init_cubeRBT(){
	objectRBT[0] = glm::scale(0.7f, 0.7f, 0.7f)*glm::translate(-1.2f, 1.2f, .0f);
	objectRBT[1] = glm::scale(0.7f, 0.7f, 0.7f)*glm::translate(0.0f, 1.2f, .0f);
	objectRBT[2] = glm::scale(0.7f, 0.7f, 0.7f)*glm::translate(1.2f, 1.2f, .0f);
	objectRBT[3] = glm::scale(0.7f, 0.7f, 0.7f)*glm::translate(-1.2f, 0.0f, .0f);
	objectRBT[4] = glm::scale(0.7f, 0.7f, 0.7f)*glm::translate(0.0f, 0.0f, .0f); // Center
	objectRBT[5] = glm::scale(0.7f, 0.7f, 0.7f)*glm::translate(1.2f, 0.0f, .0f);
	objectRBT[6] = glm::scale(0.7f, 0.7f, 0.7f)*glm::translate(-1.2f, -1.2f, .0f);
	objectRBT[7] = glm::scale(0.7f, 0.7f, 0.7f)*glm::translate(0.0f, -1.2f, .0f);
	objectRBT[8] = glm::scale(0.7f, 0.7f, 0.7f)*glm::translate(1.2f, -1.2f, .0f);
}

void set_program(int p){
	for(int i = 0; i < 9; i++){
		cubes[i].GLSLProgramID = addPrograms[p];
	}
}

void init_shader(int idx, const char * vertexShader_path, const char * fragmentShader_path){
	addPrograms[idx] = LoadShaders(vertexShader_path, fragmentShader_path);
	glUseProgram(addPrograms[idx]);
}

void init_cubemap(const char * baseFileName, int size){
	glActiveTexture(GL_TEXTURE0 + 10);
	glGenTextures(1, &cubeTex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);
	const char * suffixes[] = {"posx", "negx", "posy", "negy", "posz", "negz"};
	GLuint targets[] = {
		GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	};
	GLint w, h;
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA8, size, size);
	for(int i = 0; i < 6; i++){
		std::string texName = std::string(baseFileName) + "_" + suffixes[i] + ".bmp";
		unsigned char* data = loadBMP_cube(texName.c_str(), &w, &h);
		glTexSubImage2D(targets[i], 0, 0, 0, w, h,
						GL_BGR, GL_UNSIGNED_BYTE, data);
		delete[] data;
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 10);
}

void init_texture(void){
	// Initialize cube texture
	texture[0] = loadBMP_custom("vocaloid_final.bmp");
	for(int a = 1; a < 9; a++){
		texture[a] = texture[0];
	}
	for(int a = 0; a < 3; a++){
		textureID[a][0] = glGetUniformLocation(addPrograms[a], "myTextureSampler");
		for(int b = 1; b < 9; b++){
			textureID[a][b] = textureID[a][0];
		}
	}

	// Initialize bump texture
	bumpTex = loadBMP_custom("bump_map_final.bmp");
	bumpTexID = glGetUniformLocation(addPrograms[1], "myBumpSampler");

	// Initialize cubemap texture
	init_cubemap("yokohama3", 2048);
}

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
}

void setWrtFrame(){
	switch(object_index){
		case 0:
			// world-sky: transFact(worldRBT) * linearFact(skyRBT), sky-sky: transFact(skyRBT) * linearFact(skyRBT)
			aFrame = (sky_type == 0) ? linearFact(skyRBT) : skyRBT;
			break;
		case 1:
			aFrame = transFact(objectRBT[0]) * linearFact(eyeRBT);
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
			glm::quat w0 = w1 * w2;
			// Halve the angle
			// glm::quat w = glm::angleAxis(glm::angle(w0) / 2.0f, glm::axis(w0));
			m = glm::toMat4(w0);
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
			objectRBT[0] = aFrame * m * glm::inverse(aFrame) * objectRBT[0];
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
				std::cout << "CS380 Homework Assignment 5" << std::endl;
				std::cout << "keymaps:" << std::endl;
				std::cout << "h\t Help command" << std::endl;
				std::cout << "p\t Cycle through the programs" << std::endl;
				std::cout << "q\t Rotate first column" << std::endl;
				std::cout << ".\t Pause/resume all light animations (camera can still move)" << std::endl;
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
				program_cnt++;
				if(program_cnt > 2)
					program_cnt = 0;
				set_program(program_cnt);
				break;
			case GLFW_KEY_Q:
				if(!rot_first_col){
					rot_first_col = true;
				}
				break;
			case GLFW_KEY_PERIOD:
				playing = playing ?
					(plPauseTime = sPauseTime = glfwGetTime(), std::cout << "Light animation paused." << std::endl, false) :
					(plStartTime += glfwGetTime() - plPauseTime, sStartTime += glfwGetTime() - sPauseTime, std::cout << "Light animation resumed." << std::endl, true);
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
	Model models[10];
	for(int a = 0; a < 9; a++){
		models[a] = cubes[a];
	}
	models[9] = arcBall;
	GLfloat lightFloats[6] = {dli, pli, plar, si, sar, sca};
	GLfloat lightVec3s[7 * 3] = {dlc.x, dlc.y, dlc.z, dld.x, dld.y, dld.z,
		plc.x, plc.y, plc.z, pll.x, pll.y, pll.z,
		sc.x, sc.y, sc.z, sl.x, sl.y, sl.z, sd.x, sd.y, sd.z};

	for(int a = 0; a < 10; a++){
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
	window = glfwCreateWindow((int) windowWidth, (int) windowHeight, "Homework5", NULL, NULL);
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
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	Projection = glm::perspective(fov, windowWidth / windowHeight, 0.1f, 100.0f);
	skyRBT = glm::translate(glm::mat4(1.0f), eyePosition);

	aFrame = linearFact(skyRBT);
	// initial eye frame = sky frame;
	eyeRBT = skyRBT;

	// init shader
	init_shader(0, "VertexShader.glsl", "FragmentShader.glsl");
	init_shader(1, "BumpVertexShader.glsl", "BumpFragmentShader.glsl");
	init_shader(2, "EnvVertexShader.glsl", "EnvFragmentShader.glsl");

	// TODO: Initialize cube models by calling textured cube models
	init_cubeRBT();
	cubes[0] = Model();
	init_texture_cube(cubes[0]);
	cubes[0].initialize(DRAW_TYPE::ARRAY, addPrograms[0]);

	cubes[0].set_projection(&Projection);
	cubes[0].set_eye(&eyeRBT);
	cubes[0].set_model(&objectRBT[0]);
	for(int i = 1; i < 9; i++){
		cubes[i] = Model();
		cubes[i].initialize(DRAW_TYPE::ARRAY, cubes[0]);

		cubes[i].set_projection(&Projection);
		cubes[i].set_eye(&eyeRBT);
		cubes[i].set_model(&objectRBT[i]);
	}

	// Initialize skybox
	skybox = Model();
	init_skybox(skybox);
	skybox.initialize(DRAW_TYPE::ARRAY, addPrograms[2]);
	skybox.set_projection(&Projection);
	skybox.set_eye(&eyeRBT);
	skybox.set_model(&skyboxRBT);

	// Initialize arcball
	arcBall = Model();
	init_sphere(arcBall);
	arcBall.initialize(DRAW_TYPE::INDEX, cubes[0].GLSLProgramID);

	arcBall.set_projection(&Projection);
	arcBall.set_eye(&eyeRBT);
	arcBall.set_model(&arcballRBT);

	// init textures
	init_texture();

	mat4 oO[9];
	for(int i = 0; i < 9; i++) oO[i] = objectRBT[i];
	program_cnt = 0;
	set_program(0);

	// first column rotation
	int ani_count = 0;
	float ani_angle = 0.0f;
	mat4 curRBT[9];
	for(int i = 0; i < 9; i++) curRBT[i] = objectRBT[i];

	// Set light properties
	dlIntensity = 0.0f, dlColor = startColors[0], dlDirection = startLocsDirs[0];
	plIntensity = 0.0f, plAttenuationRatio = startMisc[0], plColor = startColors[1], plLocation = startLocsDirs[1];
	sIntensity = 0.0f, sAttenuationRatio = startMisc[1], sConeAngle = startMisc[2], sColor = startColors[2], sLocation = startLocsDirs[2], sDirection = startLocsDirs[3];

	float currtime = glfwGetTime(), prevtime = currtime, deltatime = currtime - prevtime;
	do{
		currtime = glfwGetTime(), deltatime = currtime - prevtime;

		if(deltatime > 0.008){
			prevtime = currtime;

			// Clear the screen
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			eyeRBT = (view_index == 0) ? skyRBT : objectRBT[0];

			// cube rotation
			if(rot_first_col){
				ani_angle += 0.3f;
				objectRBT[0] = glm::rotate(glm::mat4(1.0f), ani_angle * 10, glm::vec3(1.0f, 0.0f, 0.0f)) * curRBT[0];
				objectRBT[3] = glm::rotate(glm::mat4(1.0f), ani_angle * 10, glm::vec3(1.0f, 0.0f, 0.0f)) * curRBT[3];
				objectRBT[6] = glm::rotate(glm::mat4(1.0f), ani_angle * 10, glm::vec3(1.0f, 0.0f, 0.0f)) * curRBT[6];

				ani_count++;
				if(ani_count > 59){
					curRBT[0] = objectRBT[0];
					curRBT[3] = objectRBT[3];
					curRBT[6] = objectRBT[6];
					rot_first_col = false;
					ani_angle = 0.0f;
					ani_count = 0;
				}
			}

			/* // TODO: draw OBJ models
			// TODO: pass the light value (uniform variables) to shader
			// TODO: pass the texture value to shader */

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

			// TODO: pass the first texture value to shader
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture[0]);
			glUniform1i(textureID[program_cnt][0], 0);

			// Draw the cube models
			glUseProgram(cubes[0].GLSLProgramID);
			cubes[0].draw();
			for(int i = 1; i < 9; i++){
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, texture[i]);
				glUniform1i(textureID[program_cnt][i], i);

				glUseProgram(cubes[i].GLSLProgramID);
				cubes[i].draw2(cubes[0]);
			}

			// TODO: pass bump(normalmap) texture value to shader
			if(program_cnt == 1){
				glActiveTexture(GL_TEXTURE0 + 9);
				glBindTexture(GL_TEXTURE_2D, bumpTex);
				glUniform1i(bumpTexID, 9);
			}

			if(program_cnt == 2){
				// Pass the cubemap texture and eye position
				glUseProgram(addPrograms[2]);
				isEye = glGetUniformLocation(addPrograms[2], "WorldCameraPosition");
				glUniform3f(isEye, eyePosition.x, eyePosition.y, eyePosition.z);
				isSky = glGetUniformLocation(addPrograms[2], "DrawSkyBox");
				glUniform1i(isSky, 1);
				cubeTexID = glGetUniformLocation(addPrograms[2], "cubemap");
				glActiveTexture(GL_TEXTURE0 + 10);
				glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);
				glUniform1i(cubeTexID, 10);
				glDepthMask(GL_FALSE);
				skybox.draw();
				glDepthMask(GL_TRUE);
				glUniform1i(isSky, 0);
			}
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			switch(object_index){
				case 0:
					arcballRBT = (sky_type == 0) ? worldRBT : skyRBT;
					break;
				case 1:
					arcballRBT = objectRBT[0];
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
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	} // Check if the ESC key was pressed or the window was closed
	while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		  glfwWindowShouldClose(window) == 0);

	  // Clean up data structures and glsl objects
	for(int i = 0; i < 9; i++) cubes[i].cleanup();
	skybox.cleanup();
	arcBall.cleanup();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
