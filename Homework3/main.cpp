// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stack>

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
#include <common/picking.hpp>

float g_groundSize = 100.0f;
float g_groundY = -2.5f;

GLint lightLocGround, lightLocArc;

// View properties
glm::mat4 Projection;
float windowWidth = 1024.0f;
float windowHeight = 768.0f;
int frameBufferWidth = 0;
int frameBufferHeight = 0;
float fov = 45.0f;
float fovy = fov;

// Model properties
Model ground;
glm::mat4 skyRBT;
glm::mat4 eyeRBT;
glm::mat4 worldRBT = glm::mat4(1.0f);
glm::mat4 aFrame;

// Arcball manipulation
Model arcBall;
glm::mat4 arcballRBT = glm::mat4(1.0f);
float arcBallScreenRadius = 0.25f * min(windowWidth, windowHeight); // for the initial assignment
float screenToEyeScale = 0.01f;
float arcBallScale = 0.01f;

// Types added (begin)
typedef std::vector<int> veci;
// Types added (end)

// Variables added (begin)
// For manipulation
int select_object = -1;
mat4 objectRBTPrev[3][3];
vec2 mousePos = vec2(), mousePosPrev = vec2(), arcBallScreenPos = vec2();
vec3 arcBallEyePos = vec3(), mouseSpherePos = vec3();
bool arcBallExists, isMainObject = true, isCenterYAxis = false, mousePressed = false, mouseReleased = false, mouseFirst = false,
leftMousePressed = false, rightMousePressed = false, middleMousePressed = false,
leftMouseReleased = false, rightMouseReleased = false, middleMouseReleased = false,
leftMouseFirst = false, rightMouseFirst = false, middleMouseFirst = false;
// For picking
bool isPicking = false;
int pickedTarget0, pickedTarget1, previous_object;
// For Rubik's cube
Model rubik[9];
mat4 rubikRBT[3][3];
bool rubikVisited[3][3];
GLint lightLocRubik[9];
std::stack<mat4> ms = std::stack<mat4>();
// Variables added (end)

// Function definition
static void window_size_callback(GLFWwindow*, int, int);
static void mouse_button_callback(GLFWwindow*, int, int, int);
static void cursor_pos_callback(GLFWwindow*, double, double);
static void keyboard_callback(GLFWwindow*, int, int, int, int);
void update_fovy(void);

// Functions added (begin)
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

//void resetRubik(){
//	for(int r = 0; r < 3; r++){
//		for(int c = 0; c < 3; c++){
//			rubikRBT[r][c] = translate((float) r * 1.02f / 3.0f - 1.02f / 3.0f, (float) c * 1.02f / 3.0f - 1.02f / 3.0f, 0.0f) * scale(vec3(1 / 3.0f, 1 / 3.0f, 1 / 3.0f));
//			rubik[r * 3 + c].clear_parent();
//			rubik[r * 3 + c].clear_children();
//		}
//	}
//	rubik[0].add_parent(veci{1, 3});
//	rubik[1].add_parent(veci{4});
//	rubik[2].add_parent(veci{1, 5});
//	rubik[3].add_parent(veci{4});
//	rubik[5].add_parent(veci{4});
//	rubik[6].add_parent(veci{3, 7});
//	rubik[7].add_parent(veci{4});
//	rubik[8].add_parent(veci{5, 7});
//	rubik[1].add_child(veci{0, 2});
//	rubik[3].add_child(veci{0, 6});
//	rubik[4].add_child(veci{1, 3, 5, 7});
//	rubik[5].add_child(veci{2, 8});
//	rubik[7].add_child(veci{6, 8});
//}
//
//void sever(int idSelf, int idSever){
//	rubik[idSelf].remove_parent(idSever);
//	rubik[idSever].remove_child(idSelf);
//}
//
//void create(int idSelf, int idCreate){
//	rubik[idSelf].add_parent(veci{idCreate});
//	rubik[idCreate].add_child(veci{idSelf});
//}
//
//void reconnect(int idSelf, int idSever, int idCreate){
//	sever(idSelf, idSever);
//	create(idSelf, idCreate);
//}
//
//int findCommonParent(int first, int second){
//	if(!rubik[second].parentsID.empty()){
//		for(auto a = rubik[second].parentsID.begin(); a != rubik[second].parentsID.end(); a++){
//			if(rubik[first].find_parent(*a) >= 0){
//				return *a;
//			}
//		}
//	}
//
//	return -1;
//}
//
//int checkPick(int first, int second){
//	int tmp;
//	isMainObject = false;
//	previous_object = select_object;
//	if((tmp = rubik[first].find_parent(second) >= 0) || rubik[second].find_parent(first) >= 0){
//		if(tmp >= 0){
//			if(second == 4 && (first == 3 || first == 5)){
//				isCenterYAxis = true;
//			}
//			else{
//				isCenterYAxis = false;
//			}
//			return second;
//		}
//		else{
//			if(first == 4 && (second == 3 || second == 5)){
//				isCenterYAxis = true;
//			}
//			else{
//				isCenterYAxis = false;
//			}
//			return first;
//		}
//	}
//	else if((tmp = findCommonParent(first, second)) >= 0){
//		if(tmp == 4){
//			if((first + second) / 2 == 4){
//				if(first == 3 || first == 5){
//					isCenterYAxis = true;
//				}
//				else{
//					isCenterYAxis = false;
//				}
//				return tmp;
//			}
//		}
//		else{
//			return tmp;
//		}
//	}
//	else if(first == second){
//		isMainObject = true;
//		return 4;
//	}
//
//	return -1;
//}

//void syncRubic(int topID){
//	if(topID >= 0){
//		if()
//	}
//}
// Functions added (end)

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

	// re-allocate textures with respect to new framebuffer width and height
	reallocate_picking_texture(frameBufferWidth, frameBufferHeight);

	update_fovy();

	// Update projection matrix
	Projection = glm::perspective(fov, ((float) frameBufferWidth / (float) frameBufferHeight), 0.1f, 100.0f);

	arcBallScreenRadius = 0.25f * min(frameBufferWidth, frameBufferHeight);
}

// TODO: Fill up GLFW mouse button callback function
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
	// Manipulation
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

	// Picking
	if(isPicking && button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		xpos = xpos / ((double) windowWidth) * ((double) frameBufferWidth);
		ypos = ypos / ((double) windowHeight) * ((double) frameBufferHeight);
		if(pickedTarget0 <= 0){
			pickedTarget0 = pick((int) xpos, (int) ypos, frameBufferWidth, frameBufferHeight);
		}
		else{
			pickedTarget1 = pick((int) xpos, (int) ypos, frameBufferWidth, frameBufferHeight);
			if(pickedTarget1 > 0){
				isPicking = false;
				//select_object = checkPick(pickedTarget0 - 1, pickedTarget1 - 1);
				pickedTarget0 = pickedTarget1 = -1;
				if(select_object < 0){
					std::cout << "Selection invalid." << std::endl;
					select_object = previous_object;
				}
				std::cout << "Selected object: " << select_object << std::endl;
			}
		}
	}
}

// TODO: Fill up GLFW cursor position callback function
static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos){
	float dx, dy, z;
	int sor = select_object / 3, soc = select_object % 3;
	quat aQuat = quat();
	vec3 arcBallTrans = vec3(), mouseSpherePosCur;
	vec2 mpc;
	mat4 transformM, transformMInvLinear;
	if(mousePressed){
		if(leftMousePressed && !rightMousePressed && !middleMousePressed){
			if(arcBallExists){
				mpc = mousePos - arcBallScreenPos;
				z = arcBallScreenRadius * arcBallScreenRadius - length2(mpc), z = z > 0.0f ? sqrtf(z) : 0.0f;
			}
			mouseSpherePosCur = normalize(vec3(mpc.x, mpc.y, z));

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
			}
		}
		else if(leftMousePressed && rightMousePressed || middleMousePressed){
			if(!all(equal(mousePosPrev, vec2()))){
				dy = windowHeight - ypos - 1.0f - mousePosPrev.y;
				if(arcBallExists){
					arcBallTrans = vec3(0.0f, 0.0f, -dy) * screenToEyeScale;
				}
			}
		}

		if(mouseFirst){
			set(mouseSpherePos, mouseSpherePosCur.x, mouseSpherePosCur.y, mouseSpherePosCur.z);
			set(mousePosPrev, mousePos.x, mousePos.y);
			leftMouseFirst = rightMouseFirst = middleMouseFirst = false;

			if(select_object >= 0){
				copyMat4(&rubikRBT[sor][soc], &objectRBTPrev[sor][soc]);
			}
		}

		transformM = translate(arcBallTrans) * mat4_cast(aQuat);
		transformMInvLinear = translate(arcBallTrans) * mat4_cast(inverse(aQuat));

		if(select_object >= 0){
			if(!isMainObject){
				rubikRBT[sor][soc] = transformM * objectRBTPrev[sor][soc];

				if(select_object == 4){
					if(isCenterYAxis){
						rubikRBT[sor - 1][soc] = transformMInvLinear * objectRBTPrev[sor - 1][soc];
						rubikRBT[sor + 1][soc] = transformMInvLinear * objectRBTPrev[sor + 1][soc];
					}
					else{
						rubikRBT[sor][soc - 1] = transformMInvLinear * objectRBTPrev[sor][soc - 1];
						rubikRBT[sor][soc + 1] = transformMInvLinear * objectRBTPrev[sor][soc + 1];
					}
				}
			}
			else{
				rubikRBT[sor][soc] = transformM * objectRBTPrev[sor][soc];
			}
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
				std::cout << "CS380 Homework Assignment 3" << std::endl;
				std::cout << "keymaps:" << std::endl;
				std::cout << "h\t\t Help command" << std::endl;
				std::cout << "p\t\t Enable/Disable picking" << std::endl;
				break;
			case GLFW_KEY_P:
				// TODO: Enable/Disable picking
				isPicking = !isPicking;
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
	window = glfwCreateWindow((int) windowWidth, (int) windowHeight, "Homework 3: Your Student ID - Your Name ", NULL, NULL);
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

	// Initialize framebuffer object and picking textures
	picking_initialize(frameBufferWidth, frameBufferHeight);

	Projection = glm::perspective(fov, ((float) frameBufferWidth / (float) frameBufferHeight), 0.1f, 100.0f);
	skyRBT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.25, 4.0));

	// initial eye frame = sky frame;
	eyeRBT = skyRBT;

	// Initialize Ground Model
	ground = Model();
	init_ground(ground);
	ground.initialize(DRAW_TYPE::ARRAY, "VertexShader.glsl", "FragmentShader.glsl"); //
	ground.set_projection(&Projection);
	ground.set_eye(&eyeRBT);
	glm::mat4 groundRBT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, g_groundY, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(g_groundSize, 1.0f, g_groundSize));
	ground.set_model(&groundRBT);

	for(int a = 0; a < 9; a++){
		rubik[a] = Model();
		vec3 tmp[6] = {vec3(1.0f, 0.0f, 0.0f),
			vec3(1.0f, 1.0f, 0.0f),
			vec3(0.0f, 1.0f, 0.0f),
			vec3(0.0f, 1.0f, 1.0f),
			vec3(0.0f, 0.0f, 1.0f),
			vec3(1.0f, 0.0f, 1.0f)};
		//init_rubik(rubik[a], tmp);
		rubik[a].initialize(DRAW_TYPE::ARRAY, "VertexShader.glsl", "FragmentShader.glsl");
		rubik[a].initialize_picking("PickingVertexShader.glsl", "PickingFragmentShader.glsl");
		rubik[a].set_projection(&Projection);
		rubik[a].set_eye(&eyeRBT);
		rubik[a].set_model(&rubikRBT[a / 3][a % 3]);

		rubik[a].objectID = a + 1;
	}
	//resetRubik();

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

	for(int a = 0; a < 9; a++){
		lightLocRubik[a] = glGetUniformLocation(rubik[a].GLSLProgramID, "uLight");
		glUniform3f(lightLocRubik[a], lightVec.x, lightVec.y, lightVec.z);
	}

	lightLocArc = glGetUniformLocation(arcBall.GLSLProgramID, "uLight");
	glUniform3f(lightLocArc, lightVec.x, lightVec.y, lightVec.z);

	do{
		// Manipulation
		switch(pickedTarget0){
			default:
				break;
		}

		// first pass: picking shader
		// binding framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, picking_fbo);
		// Background: RGB = 0x000000 => objectID: 0
		glClearColor((GLclampf) 0.0f, (GLclampf) 0.0f, (GLclampf) 0.0f, (GLclampf) 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// drawing objects in framebuffer (picking process)
		for(int a = 0; a < 9; a++){
			rubik[a].drawPicking();
		}

		// second pass: your drawing
		// unbinding framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor((GLclampf) (128. / 255.), (GLclampf) (200. / 255.), (GLclampf) (255. / 255.), (GLclampf)0.);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ground.draw();
		for(int a = 0; a < 9; a++){
			rubik[a].draw();
		}

		arcBallExists = !isPicking;
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

		// Swap buffers (Double buffering)
		glfwSwapBuffers(window);
		glfwPollEvents();
	} // Check if the ESC key was pressed or the window was closed
	while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		  glfwWindowShouldClose(window) == 0);

	  // Clean up data structures and glsl objects
	ground.cleanup();
	arcBall.cleanup();

	// Cleanup textures
	delete_picking_resources();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
