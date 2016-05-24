#version 330 core

in vec3 fragmentPosition;
in vec3 fragmentColor;
in vec3 fragmentNormal;

// Output data
out vec3 color;

uniform float lightFloats[6]; // dlIntensity, plIntensity, plAttenuationRatio, sIntensity, sAttenuationRatio, sConeAngle
uniform vec3 lightVec3s[7]; // dlColor, dlDirection, plColor, plLocation, sColor, sLocation, sDirection

vec3 shininess = vec3(0.5, 0.5, 0.5);

vec3 applyDL(){
	if(lightFloats[0] == 0.0){
		return vec3(0.0);
	}
	
	vec3 tolight = normalize(-lightVec3s[1] - fragmentPosition), 
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
	attenuation = 1.0 / (1.0 + lightFloats[2] * pow(dist, 2));
	
	return (diffuse + specular) * attenuation;
}

vec3 applyS(){
	if(lightFloats[3] == 0.0){
		return vec3(0.0);
	}
	
	vec3 tolight = normalize(lightVec3s[5] - fragmentPosition), 
	toV = -normalize(fragmentPosition), 
	hv = normalize(toV + tolight), 
	normal = normalize(fragmentNormal);
	float diffuseCo = max(0.0, dot(normal, tolight));
	vec3 diffuse = diffuseCo * fragmentColor * lightVec3s[4] * lightFloats[3];
	float specularCo = pow(max(0.0, dot(hv, normal)), 64.0);
	vec3 specular = specularCo * shininess * lightVec3s[4] * lightFloats[3];
	float dist = distance(lightVec3s[5], fragmentPosition), 
	attenuation = 1.0 / (1.0 + lightFloats[4] * pow(dist, 2)), 
	ltofa = acos(dot(-tolight, lightVec3s[6]));
	if(ltofa > lightFloats[5]){
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
	color = applyGC(applyLight());
}
