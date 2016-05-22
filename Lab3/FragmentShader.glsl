#version 330 core

in vec3 fragmentPosition;
in vec3 fragmentColor;
in vec3 fragmentNormal;

// for Gouraud shading
in vec3 gcolor;

// Ouput data
out vec3 color;





void main(){
	// Gouraud shading
	color = gcolor;
}
