#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
//TODO: grab color value from the application
layout(location = 2) in vec3 vertexColor;
//TODO: grab normal value from the application
layout(location = 1) in vec3 vertexNormal_modelspace;

// Output data ; will be interpolated for each fragment.
out vec3 fragmentPosition;
out vec3 fragmentColor;
out vec3 fragmentNormal;

uniform mat4 ModelTransform;
uniform mat4 Eye;
uniform mat4 Projection;

void main(){
	// Output position of the vertex, in clip space : MVP * position
	vec4 wPosition = ModelTransform * vec4(vertexPosition_modelspace, 1.0);
	fragmentPosition = wPosition.xyz;
	gl_Position = Projection * inverse(Eye) * wPosition;
	
	//TODO: pass the interpolated color value to fragment shader 
	fragmentColor = vertexColor;
	
	//TODO: Calculate/Pass normal of the the vertex
	//transpose of inversed model matrix
	mat4 NM = transpose(inverse(ModelTransform));
	vec4 tnormal = vec4(vertexNormal_modelspace, 0.0);
	fragmentNormal = vec3(NM * tnormal);
}
