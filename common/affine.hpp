#ifndef AFFINE_H
#define AFFINE_H
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

/*
 * An affine matrix A can be factored as A = TL. You need to fill up two function named 'linearFact' and 'transFact'
 */

// TODO: Fill up linearFact function
// input: A (4 x 4 matrix)
// output: L (4 x 4 matrix)
glm::mat4 linearFact(glm::mat4 A){
	glm::mat4 L = glm::mat4();
	for(int col = 0; col < 3; col++){
		for(int row = 0; row < 4; row++){
			L[col][row] = A[col][row];
		}
	}

	return L;
}

// TODO: Fill up transFact function
// input: A (4 x 4 matrix)
// output: T (4 x 4 matrix)
glm::mat4 transFact(glm::mat4 A){
	glm::mat4 T = glm::mat4();
	for(int a = 0; a < 3; a++){
		T[3][a] = A[3][a];
	}

	return T;
}
#endif
