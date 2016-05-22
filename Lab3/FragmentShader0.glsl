#version 330 core

in vec3 fragmentPosition;
in vec3 fragmentColor;
in vec3 fragmentNormal;

// Ouput data
out vec3 color;

uniform float dlIntensity, plIntensity, sIntensity, sConeAngle;
uniform vec3 dlColor, dlDirection, plColor, plLocation, sColor, sLocation, sDirection;

vec3 shininess = vec3(0.5, 0.5, 0.5);

vec3 applyDL(){
	if(dlIntensity == 0.0){
		return vec3();
	}
	
	vec3 tolight = -dlDirection, 
	toV = -normalize(fragmentPosition), 
	half = normalize(toV + tolight), 
	normal = normalize(fragmentNormal);
	float diffuseCo = max(0.0, dot(normal, tolight));
	vec3 diffuse = diffuseCo * fragmentColor * dlColor * dlIntensity;
	float specularCo = pow(max(0.0, dot(half, normal)), 64.0);
	vec3 specular = specularCo * shininess * dlColor * dlIntensity;
	
	return diffuse + specular;
}

vec3 applyPL(){
	if(plIntensity == 0.0){
		return vec3();
	}
	
	vec3 tolight = normalize(plLocation - fragmentPosition), 
	toV = -normalize(fragmentPosition), 
	half = normalize(toV + tolight), 
	normal = normalize(fragmentNormal);
	float diffuseCo = max(0.0, dot(normal, tolight));
	vec3 diffuse = diffuseCo * fragmentColor * plColor * plIntensity;
	float specularCo = pow(max(0.0, dot(half, normal)), 64.0);
	vec3 specular = specularCo * shininess * plColor * plIntensity;
	float dist = distance(plLocation, fragmentPosition), 
	attenuation = 1.0 / (1.0 + pow(dist, 2));
	
	return (diffuse + specular) * attenuation;
}

vec3 applyS(){
	if(sIntensity == 0.0){
		return vec3();
	}
	
	vec3 tolight = normalize(sLocation - fragmentPosition), 
	toV = -normalize(fragmentPosition), 
	half = normalize(toV + tolight), 
	normal = normalize(fragmentNormal);
	float diffuseCo = max(0.0, dot(normal, tolight));
	vec3 diffuse = diffuseCo * fragmentColor * sColor * sIntensity;
	float specularCo = pow(max(0.0, dot(half, normal)), 64.0);
	vec3 specular = specularCo * shininess * sColor * sIntensity;
	float dist = distance(sLocation, fragmentPosition), 
	attenuation = 1.0 / (1.0 + pow(dist, 2)), 
	ltofa = acos(dot(-tolight, sDirection));
	if(ltofa > sConeAngle){
		attenuation = 0.0;
	}
	
	return (diffuse + specular) * attenuation;
}

void main(){
	color = pow(applyDL() + applyPL() + applyS(), vec3(1.0 / 2.2));
}
