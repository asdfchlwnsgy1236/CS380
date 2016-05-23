#version 330 core

in vec3 fragmentPosition;
in vec3 fragmentColor;
in vec3 fragmentNormal;

// Output data
out vec3 color;

uniform float lightFloats[4]; // dlIntensity, plIntensity, sIntensity, sConeAngle
uniform vec3 lightVec3s[7]; // dlColor, dlDirection, plColor, plLocation, sColor, sLocation, sDirection

vec3 shininess = vec3(0.5, 0.5, 0.5), preColor;

vec3 applyDL(){
	if(lightFloats[0] == 0.0){
		return vec3(0.0);
	}
	
	vec3 tolight = -lightVec3s[1], 
	toV = -normalize(fragmentPosition), 
	hv = normalize(toV + tolight), 
	normal = normalize(fragmentNormal);
	float diffuseCo = max(0.0, dot(normal, tolight));
	vec3 diffuse = diffuseCo * fragmentColor * lightVec3s[0] * lightFloats[0];
	float specularCo = pow(max(0.0, dot(hv, normal)), 64.0);
	vec3 specular = specularCo * shininess * lightVec3s[0] * lightFloats[0];
	
	return diffuse + specular;
}

vec3 applyPL(){
	if(lightFloats[1] == 0.0){
		return vec3(0.0);
	}
	
	vec3 tolight = normalize(lightVec3s[3] - fragmentPosition), 
	toV = -normalize(fragmentPosition), 
	hv = normalize(toV + tolight), 
	normal = normalize(fragmentNormal);
	float diffuseCo = max(0.0, dot(normal, tolight));
	vec3 diffuse = diffuseCo * fragmentColor * lightVec3s[2] * lightFloats[1];
	float specularCo = pow(max(0.0, dot(hv, normal)), 64.0);
	vec3 specular = specularCo * shininess * lightVec3s[2] * lightFloats[1];
	float dist = distance(lightVec3s[3], fragmentPosition), 
	attenuation = 1.0 / (1.0 + pow(dist, 2));
	
	return (diffuse + specular) * attenuation;
}

vec3 applyS(){
	if(lightFloats[2] == 0.0){
		return vec3(0.0);
	}
	
	vec3 tolight = normalize(lightVec3s[5] - fragmentPosition), 
	toV = -normalize(fragmentPosition), 
	hv = normalize(toV + tolight), 
	normal = normalize(fragmentNormal);
	float diffuseCo = max(0.0, dot(normal, tolight));
	vec3 diffuse = diffuseCo * fragmentColor * lightVec3s[4] * lightFloats[2];
	float specularCo = pow(max(0.0, dot(hv, normal)), 64.0);
	vec3 specular = specularCo * shininess * lightVec3s[4] * lightFloats[2];
	float dist = distance(lightVec3s[5], fragmentPosition), 
	attenuation = 1.0 / (1.0 + pow(dist, 2)), 
	ltofa = acos(dot(-tolight, lightVec3s[6]));
	if(ltofa > lightFloats[3]){
		attenuation = 0.0;
	}
	
	return (diffuse + specular) * attenuation;
}

vec3 applyLight(){
	return applyDL() + applyPL() + applyS();
}

vec3 applyGC(vec3 before){
	return pow(before, vec3(1.0 / 2.2));
}

void main(){
	preColor = applyGC(applyLight());
	float luma = preColor.x * 0.2126 + preColor.y * 0.7152 + preColor.z * 0.0722;
	if(luma < 0.25){
		color = vec3(0.0, 0.125, 0.0);
	}
	else if(luma < 0.5){
		color = vec3(0.0, 0.375, 0.0);
	}
	else if(luma < 0.75){
		color = vec3(0.0, 0.625, 0.0);
	}
	else{
		color = vec3(0.0, 0.875, 0.0);
	}
}
