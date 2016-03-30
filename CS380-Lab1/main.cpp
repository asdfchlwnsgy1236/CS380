// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <iterator>
#include <list>
#include <random>
#include <functional>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

// Shader library
#include <common/shader.hpp>

#define BUFFER_OFFSET( offset ) ((GLvoid*) (offset))

GLFWwindow* window;
int windowWidth, windowHeight;

GLuint programID, VAID, VBID, VBIDcolor;

std::vector<glm::vec3> g_vertex_buffer_data, g_color_buffer_data;
// special indexes in g_vertex_buffer_data and g_color_buffer_data
size_t snowflakeStart, backgroundStart, pipeStart, groundStart, sunStart, moonStart, totalEnd;

glm::mat4 Projection;
glm::mat4 View;

// random generator stuff
std::random_device rd;
std::mt19937 mt0(rd()), mt1(rd()), mt2(rd()), mt3(rd()), mt4(rd()), mt5(rd()), mt6(rd());
std::uniform_real_distribution<float>
dist0(-1.0f, 1.0f),
dist1(-0.25f, 0.25f),
dist2(0.0f, 360.0f),
dist3(0.05f, 0.075f),
dist4(-0.01f, 0.01f),
dist5(-0.1f, 0.1f),
dist6(0.1f, 0.45f);
auto rng0 = std::bind(dist0, mt0),
rng1 = std::bind(dist1, mt1),
rng2 = std::bind(dist2, mt2),
rng3 = std::bind(dist3, mt3),
rng4 = std::bind(dist4, mt4),
rng5 = std::bind(dist5, mt5),
rng6 = std::bind(dist6, mt6);

// data containers for the snowflakes, background, and other graphical objects
std::list<std::vector<float>> snowData; // 0: x, 1: y, 2: z, 3: angle, 4: scale, 5: resting height, 6: vx, 7: vy, 8: vz
std::vector<float> backgroundData, pipeData, groundData, sunData, moonData; // 0: x, 1: y, 2: z, 3: angle, 4: scale

// the koch curve function
void koch_line(glm::vec3 a, glm::vec3 b, int iter){
	if(iter <= 0){
		return;
	}

	float tmpd = distance(a, b) / 3.0f;
	vec3 p1 = (a * 2.0f + b) / 3.0f,
		p2 = vec3((a.x + b.x) / 2.0f - sqrtf(3.0f) / 6.0f * (b.y - a.y), (a.y + b.y) / 2.0f + sqrtf(3.0f) / 6.0f * (b.x - a.x), 0.0f),
		p3 = (a + b * 2.0f) / 3.0f;

	g_vertex_buffer_data.push_back(p1);
	g_vertex_buffer_data.push_back(p2);
	g_vertex_buffer_data.push_back(p3);

	g_color_buffer_data.push_back(vec3(0.1f, 0.9f, 1.0f));
	g_color_buffer_data.push_back(vec3(0.0f, 1.0f, 1.0f));
	g_color_buffer_data.push_back(vec3(0.2f, 0.8f, 1.0f));

	koch_line(a, p1, iter - 1);
	koch_line(p1, p2, iter - 1);
	koch_line(p2, p3, iter - 1);
	koch_line(p3, b, iter - 1);
}

// TODO: Initialize model
void init_model(void){
	g_vertex_buffer_data = std::vector<glm::vec3>();
	g_color_buffer_data = std::vector<vec3>();

	// add snowflake data
	snowflakeStart = g_vertex_buffer_data.size();

	g_vertex_buffer_data.push_back(glm::vec3(-0.5f, -0.25f, 0.0f));
	g_vertex_buffer_data.push_back(glm::vec3(0.0f, sqrt(0.75) - 0.25f, 0.0f));
	g_vertex_buffer_data.push_back(glm::vec3(0.5f, -0.25f, 0.0f));

	g_color_buffer_data.push_back(vec3(0.1f, 0.9f, 1.0f));
	g_color_buffer_data.push_back(vec3(0.0f, 1.0f, 1.0f));
	g_color_buffer_data.push_back(vec3(0.2f, 0.8f, 1.0f));

	vec3 a = g_vertex_buffer_data[0],
		b = g_vertex_buffer_data[1],
		c = g_vertex_buffer_data[2];
	koch_line(a, b, 5);
	koch_line(b, c, 5);
	koch_line(c, a, 5);

	// add background data
	backgroundStart = g_vertex_buffer_data.size();

	g_vertex_buffer_data.push_back(vec3(-5.0f, 2.0f, 0.0f));
	g_vertex_buffer_data.push_back(vec3(5.0f, 2.0f, 0.0f));
	g_vertex_buffer_data.push_back(vec3(-5.0f, -1.5f, 0.0f));
	g_vertex_buffer_data.push_back(vec3(5.0f, -1.5f, 0.0f));

	g_color_buffer_data.push_back(vec3(0.0f, 1.0f, 1.0f));
	g_color_buffer_data.push_back(vec3(0.0f, 1.0f, 1.0f));
	g_color_buffer_data.push_back(vec3(0.0f, 0.0f, 1.0f));
	g_color_buffer_data.push_back(vec3(0.0f, 0.0f, 1.0f));

	// add pipe data
	pipeStart = g_vertex_buffer_data.size();

	g_vertex_buffer_data.push_back(vec3(-0.05f, -0.5f, 0.0f));
	g_vertex_buffer_data.push_back(vec3(-0.025f, -0.55f, 0.0f));
	g_vertex_buffer_data.push_back(vec3(0.0f, -0.5f, 0.0f));
	g_vertex_buffer_data.push_back(vec3(0.025f, -0.55f, 0.0f));
	g_vertex_buffer_data.push_back(vec3(0.05f, -0.5f, 0.0f));

	g_color_buffer_data.push_back(vec3(0.8f, 0.8f, 0.9f));
	g_color_buffer_data.push_back(vec3(0.8f, 0.8f, 0.9f));
	g_color_buffer_data.push_back(vec3(0.8f, 0.8f, 0.9f));
	g_color_buffer_data.push_back(vec3(0.8f, 0.8f, 0.9f));
	g_color_buffer_data.push_back(vec3(0.8f, 0.8f, 0.9f));

	// add ground data
	groundStart = g_vertex_buffer_data.size();

	g_vertex_buffer_data.push_back(vec3(-4.0f, -0.65f, 0.0f));
	g_vertex_buffer_data.push_back(vec3(4.0f, -0.65f, 0.0f));
	g_vertex_buffer_data.push_back(vec3(-4.0f, -1.5f, 0.0f));
	g_vertex_buffer_data.push_back(vec3(4.0f, -1.5f, 0.0f));

	g_color_buffer_data.push_back(vec3(0.7f, 0.7f, 0.8f));
	g_color_buffer_data.push_back(vec3(0.7f, 0.7f, 0.8f));
	g_color_buffer_data.push_back(vec3(0.7f, 0.7f, 0.8f));
	g_color_buffer_data.push_back(vec3(0.7f, 0.7f, 0.8f));

	// add Sun data
	sunStart = g_vertex_buffer_data.size();

	g_vertex_buffer_data.push_back(vec3(0.0f, 0.0f, 0.0f));
	g_color_buffer_data.push_back(vec3(1.0f, 1.0f, 0.5f));
	const float pi = (float) std::_Pi, r = 0.1f;
	for(int a = 0; a <= 256; a++){
		g_vertex_buffer_data.push_back(vec3(r * cosf(a / 128.0f * pi), r * sinf(a / 128.0f * pi), 0.0f));
		g_color_buffer_data.push_back(vec3(1.0f, 0.8, 0.0f));
	}

	// add Moon data
	moonStart = g_vertex_buffer_data.size();

	g_vertex_buffer_data.push_back(vec3(0.0f, 0.0f, 0.0f));
	g_color_buffer_data.push_back(vec3(0.9f, 0.9f, 0.9f));
	for(int a = 0; a <= 256; a++){
		g_vertex_buffer_data.push_back(vec3(r * cosf(a / 128.0f * pi), r * sinf(a / 128.0f * pi), 0.0f));
		g_color_buffer_data.push_back(vec3(0.8f, 0.8f, 0.8f));
	}

	// mark the end of the data
	totalEnd = g_vertex_buffer_data.size();

	// Generates Vertex Array Objects in the GPU's memory and passes back their identifiers
	// Create a vertex array object that represents vertex attributes stored in a vertex buffer object.
	glGenVertexArrays(1, &VAID);
	glBindVertexArray(VAID);

	// Create and initialize a buffer object; generates our buffers in the GPU's memory
	glGenBuffers(1, &VBID);
	glBindBuffer(GL_ARRAY_BUFFER, VBID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * g_vertex_buffer_data.size(), &g_vertex_buffer_data[0], GL_STATIC_DRAW);

	// Create and initialize a buffer object for colors
	glGenBuffers(1, &VBIDcolor);
	glBindBuffer(GL_ARRAY_BUFFER, VBIDcolor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * g_color_buffer_data.size(), &g_color_buffer_data[0], GL_STATIC_DRAW);
}

// draws the snowflake
void draw_snowflake(mat4 SRT){
	glUseProgram(programID);
	glBindVertexArray(VAID);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, VBIDcolor);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), BUFFER_OFFSET(0));

	mat4 MVP = Projection * View * SRT;

	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

	glDrawArrays(GL_TRIANGLES, snowflakeStart, backgroundStart - snowflakeStart);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

// draws the background
void draw_background(mat4 SRT){
	glUseProgram(programID);
	glBindVertexArray(VAID);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, VBIDcolor);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), BUFFER_OFFSET(0));

	mat4 MVP = Projection * View * SRT;

	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

	glDrawArrays(GL_TRIANGLE_STRIP, backgroundStart, pipeStart - backgroundStart);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

// draws the pipe
void draw_pipe(mat4 SRT){
	glUseProgram(programID);
	glBindVertexArray(VAID);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, VBIDcolor);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), BUFFER_OFFSET(0));

	mat4 MVP = Projection * View * SRT;

	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

	glDrawArrays(GL_TRIANGLE_STRIP, pipeStart, groundStart - pipeStart);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

// draws the ground
void draw_ground(mat4 SRT){
	glUseProgram(programID);
	glBindVertexArray(VAID);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, VBIDcolor);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), BUFFER_OFFSET(0));

	mat4 MVP = Projection * View * SRT;

	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

	glDrawArrays(GL_TRIANGLE_STRIP, groundStart, sunStart - groundStart);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

// draws the Sun
void draw_sun(mat4 SRT){
	glUseProgram(programID);
	glBindVertexArray(VAID);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, VBIDcolor);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), BUFFER_OFFSET(0));

	mat4 MVP = Projection * View * SRT;

	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

	glDrawArrays(GL_TRIANGLE_FAN, sunStart, moonStart - sunStart);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

// draws the Moon
void draw_moon(mat4 SRT){
	glUseProgram(programID);
	glBindVertexArray(VAID);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, VBIDcolor);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), BUFFER_OFFSET(0));

	mat4 MVP = Projection * View * SRT;

	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

	glDrawArrays(GL_TRIANGLE_FAN, moonStart, totalEnd - moonStart);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

// a key callback function for triggering snowflake creation
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	if(key == GLFW_KEY_SPACE && (action == GLFW_PRESS || action == GLFW_REPEAT)){
		// 0: x, 1: y, 2: z, 3: angle, 4: scale, 5: resting height, 6: vx, 7: vy, 8: vz
		snowData.emplace_back(std::vector<float>{0.0f, -0.5f, 0.0f, rng2(), rng3(), -0.6f + rng4(), rng5(), rng6(), rng5()});
	}
}

// a framebuffer size callback function for readjusting the aspect ratio and viewport on resize
static void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
	Projection = perspective(45.0f, (float) width / height, 0.1f, 100.0f);
}

int main(int argc, char* argv[]){
	// Step 1: Initialization
	if(!glfwInit()){
		return -1;
	}
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// TODO: GLFW create window and context
	window = glfwCreateWindow(1024, 768, "CS380-Lab1 by Joonhyo Choi 20130809    ----    press space for snow", NULL, NULL);
	if(window == NULL){
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	// END

	// TODO: Initialize GLEW
	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK){
		return -1;
	}
	// END

	Projection = perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	View = glm::lookAt(glm::vec3(0, 0, 2),
					   glm::vec3(0, 0, 0),
					   glm::vec3(0, 1, 0));
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 MVP = Projection * View * Model;

	// TODO: Initialize OpenGL and GLSL
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
	glViewport(0, 0, windowWidth, windowHeight);

	programID = LoadShaders("TransformVertexShader.glsl", "ColorFragmentShader.glsl");
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	// END

	// initialize buffers
	init_model();

	// prepare for time adjustments to animation
	double previousTime = glfwGetTime(), currentTime;
	float elapsedTime = 0.0;

	// initialize data containers
	// 0: x, 1: y, 2: z, 3: angle, 4: scale, 5: resting height, 6: vx, 7: vy, 8: vz
	snowData = std::list<std::vector<float>>();
	// 0: x, 1: y, 2: z, 3: angle, 4: scale
	backgroundData = {0.0f, 0.0f, -2.0f, 0.0f, 1.0f};
	pipeData = {0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
	groundData = {0.0f, 0.0f, -1.0f, 0.0f, 1.0f};
	sunData = {1.0f, 0.0f, -1.5f, 0.0f, 1.0f};
	moonData = {1.0f, 0.0f, -1.5f, 180.0f, 1.0f};

	// prepare for custom iterator advancement through the list of snowflakes
	bool iterSnowAdvance = true;

	// print message about interaction in this program to console just in case people don't see the title
	std::cout << "Press space for snow." << std::endl;

	// Step 2: Main event loop
	do{
		// animation adjustment with time (0)
		currentTime = glfwGetTime();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// start messing with draw_model() here
		// apply transformations to each currently existing snowflake if any and draw them
		for(auto iterSnow = snowData.begin(); iterSnow != snowData.end(); /* increment done within loop due to erase operation in list */){
			if((*iterSnow)[1] > (*iterSnow)[5]){
				(*iterSnow)[0] += (*iterSnow)[6] * elapsedTime;
				(*iterSnow)[1] += (*iterSnow)[7] * elapsedTime;
				(*iterSnow)[2] += (*iterSnow)[8] * elapsedTime;
				(*iterSnow)[3] -= (*iterSnow)[6] * elapsedTime * 600.0f;
				iterSnowAdvance = true;
				(*iterSnow)[7] -= 0.1f * elapsedTime;
			}
			else{
				if((*iterSnow)[4] > 0.01f){
					(*iterSnow)[4] -= 0.02f * elapsedTime;
					iterSnowAdvance = true;
				}
				else{
					iterSnow = snowData.erase(iterSnow);
					iterSnowAdvance = false;
					if(iterSnow == snowData.end()){
						break;
					}
				}
			}
			draw_snowflake(translate(mat4(1.0f), vec3((*iterSnow)[0], (*iterSnow)[1], (*iterSnow)[2]))
						   * rotate((*iterSnow)[3], vec3(0.0f, 0.0f, 1.0f))
						   * scale(mat4(1.0f), vec3((*iterSnow)[4], (*iterSnow)[4], 0.0f)));
			if(iterSnowAdvance){
				iterSnow++;
			}
		}

		// draw the background
		draw_background(translate(mat4(1.0f), vec3(backgroundData[0], backgroundData[1], backgroundData[2]))
						* rotate(backgroundData[3], vec3(0.0f, 0.0f, 1.0f))
						* scale(mat4(1.0f), vec3(backgroundData[4], backgroundData[4], 0.0f)));

		// draw the pipe
		draw_pipe(translate(mat4(1.0f), vec3(pipeData[0], pipeData[1], pipeData[2]))
				  * rotate(pipeData[3], vec3(0.0f, 0.0f, 1.0f))
				  * scale(mat4(1.0f), vec3(pipeData[4], pipeData[4], 0.0f)));

		// draw the ground
		draw_ground(translate(mat4(1.0f), vec3(groundData[0], groundData[1], groundData[2]))
					* rotate(groundData[3], vec3(0.0f, 0.0f, 1.0f))
					* scale(mat4(1.0f), vec3(groundData[4], groundData[4], 0.0f)));

		// draw the Sun
		sunData[3] += 15.0f * elapsedTime;
		sunData[0] = 3.0f * cosf(radians(sunData[3]));
		sunData[1] = 3.0f * sinf(radians(sunData[3])) - 1.8f;
		draw_sun(translate(mat4(1.0f), vec3(sunData[0], sunData[1], sunData[2]))
				 * rotate(0.0f, vec3(0.0f, 0.0f, 1.0f))
				 * scale(mat4(1.0f), vec3(sunData[4], sunData[4], 0.0f)));

		// draw the Moon
		moonData[3] += 15.0f * elapsedTime;
		moonData[0] = 3.0f * cosf(radians(moonData[3]));
		moonData[1] = 3.0f * sinf(radians(moonData[3])) - 1.8f;
		draw_moon(translate(mat4(1.0f), vec3(moonData[0], moonData[1], moonData[2]))
				  * rotate(0.0f, vec3(0.0f, 0.0f, 1.0f))
				  * scale(mat4(1.0f), vec3(moonData[4], moonData[4], 0.0f)));
		// end messing with draw_model() here

		// animation adjustment with time (1)
		elapsedTime = currentTime - previousTime;
		previousTime = currentTime;

		// double buffering
		glfwSwapBuffers(window);

		// event polling
		glfwPollEvents();
	}
	while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && !glfwWindowShouldClose(window));

	// Step 3: Termination
	g_vertex_buffer_data.clear();
	g_color_buffer_data.clear();

	glDeleteBuffers(1, &VBID);
	glDeleteBuffers(1, &VBIDcolor);

	glDeleteProgram(programID);

	glDeleteVertexArrays(1, &VAID);

	glfwTerminate();

	return 0;
}
