#ifndef MODEL_HPP
#define MODEL_HPP

#include <GL/glew.h>
#include <vector>
#include <glm/glm.hpp>

enum DRAW_TYPE{
	ARRAY,
	INDEX
};

class Model{
	std::vector<glm::vec3> vertices;
	std::vector<unsigned int> indices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> colors;

	glm::mat4* Projection;
	glm::mat4* Eye;
	glm::mat4* ModelTransform;

	DRAW_TYPE type;

	GLuint VertexArrayID;
	GLuint VertexBufferID;
	GLuint IndexBufferID;
	GLuint NormalBufferID;
	GLuint ColorBufferID;
	public:
	GLuint GLSLProgramID;
	GLuint PickingProgramID;
	int objectID = -1;

	// For Homework 3 (begin)
	std::vector<int> parentsID;
	std::vector<int> childrenID;
	// For Homework 3 (end)

	Model();
	void add_vertex(float, float, float);
	void add_vertex(glm::vec3);
	void add_normal(float, float, float);
	void add_normal(glm::vec3);
	void add_color(float, float, float);
	void add_color(glm::vec3);
	void add_index(unsigned int);
	void set_projection(glm::mat4*);
	void set_eye(glm::mat4*);
	glm::mat4* get_model(void);
	void set_model(glm::mat4*);
	void initialize(DRAW_TYPE, const char *, const char *);
	void initialize_picking(const char *, const char *);
	void draw(void);
	void drawPicking(void);
	void cleanup(void);
	// For Homework 3 (begin)
	void add_parent(std::vector<int>);
	int remove_parent(int);
	void clear_parent();
	int find_parent(int);
	void add_child(std::vector<int>);
	int remove_child(int);
	void clear_children();
	int find_child(int);
	// For Homework 3 (end)
};

#endif
