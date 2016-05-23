#version 330 core

flat in vec3 fragmentPosition;
flat in vec3 fragmentColor;
flat in vec3 fragmentNormal;

// Output data
out vec3 color;





void main(){
	//TODO: Phong reflection model
	vec3 tolight = normalize(uLight - fragmentPosition);
	vec3 toV = -normalize(vec3(fragmentPosition));
	vec3 h = normalize(toV + tolight);
	vec3 normal = normalize(fragmentNormal);
	float specular = pow(max(0.0, dot(h, normal)), 64.0);
	float diffuse = max(0.0, dot(normal, tolight));
	vec3 intensity = fragmentColor * diffuse + vec3(0.6, 0.6, 0.6) * specular;
	color = pow(intensity, vec3(1.0 / 2.2));
}
