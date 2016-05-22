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

// for Gouraud shading
out vec3 gcolor;

uniform mat4 ModelTransform;
uniform mat4 Eye;
uniform mat4 Projection;
uniform vec3 uLight;

void main(){

	// Output position of the vertex, in clip space : MVP * position
	mat4 MVM = inverse(Eye) * ModelTransform;
	
	vec4 wPosition = MVM * vec4(vertexPosition_modelspace, 1);
	fragmentPosition = wPosition.xyz;
	gl_Position = Projection * wPosition;
	
	// this variable needs to be defined here for Gouraud shading because GLSL is weird
	vec3 tolight = normalize(uLight - wPosition.xyz);
	
	//TODO: pass the interpolated color value to fragment shader 
	fragmentColor = vertexColor;
	//TODO: Calculate/Pass normal of the the vertex
	//transpose of inversed model view matrix
	mat4 invm = inverse(MVM);
	invm[0][3] = 0; invm[1][3] = 0; invm[2][3] = 0;
	mat4 NVM = transpose(invm);
	vec4 tnormal = vec4(vertexNormal_modelspace, 0.0);
	fragmentNormal = vec3(NVM * tnormal);
	
	// Gouraud shading
//	vec3 tolight = normalize(uLight - fragmentPosition);
	vec3 toV = -normalize(wPosition.wyz);
	vec3 h = normalize(toV + tolight);
	vec3 normal = normalize(vec3(NVM * tnormal));
	float specular = pow(max(0.0, dot(h, normal)), 64.0);
	float diffuse = max(0.0, dot(normal, tolight));
	vec3 intensity = vertexColor * diffuse + vec3(0.6, 0.6, 0.6) * specular;
	gcolor = pow(intensity, vec3(1.0 / 2.2));
}