#version 330 core

in vec3 fragmentPosition;
in vec3 fragmentColor;
in vec3 fragmentNormal;

// for Gouraud shading
in vec3 gcolor;

// Ouput data
out vec3 color;

uniform vec3 uLight;

void main(){
	
	//color = vec3(1.0, 1.0, 1.0);
	//TODO: Assign fragmentColor as a final fragment color
	//color = fragmentColor;
	//TODO:Assign fragmentNormal as a final fragment color
	//vec3 normal = normalize(fragmentNormal);
	//color = normal;
/*	//TODO: Phong reflection model
	vec3 tolight = normalize(uLight - fragmentPosition);
	vec3 toV = -normalize(vec3(fragmentPosition));
	vec3 h = normalize(toV + tolight);
	vec3 normal = normalize(fragmentNormal);
	float specular = pow(max(0.0, dot(h, normal)), 64.0);
	float diffuse = max(0.0, dot(normal, tolight));
	vec3 intensity = fragmentColor * diffuse + vec3(0.6, 0.6, 0.6) * specular;
	color = pow(intensity, vec3(1.0 / 2.2));*/
	
/*	// toon shading
	float tmp = color.x + color.y + color.z;
	if(tmp < 0.75){
		color = vec3(0.0, 0.125, 0.0);
	}
	else if(tmp < 1.5){
		color = vec3(0.0, 0.375, 0.0);
	}
	else if(tmp < 2.25){
		color = vec3(0.0, 0.625, 0.0);
	}
	else{
		color = vec3(0.0, 0.875, 0.0);
	}*/
	
	// Gouraud shading
	color = gcolor;
}
