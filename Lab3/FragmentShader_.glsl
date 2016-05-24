#version 330 core

in vec3 fragmentPosition;
in vec3 fragmentColor;
in vec3 fragmentNormal;
in vec3 fragmentNormalAlt;

// Output data
out vec3 color;

uniform float lightFloats[6]; // dlIntensity, plIntensity, plAttenuationRatio, sIntensity, sAttenuationRatio, sConeAngle
uniform vec3 lightVec3s[7]; // dlColor, dlDirection, plColor, plLocation, sColor, sLocation, sDirection

uniform mat4 Eye;

vec3 worldFP = vec3(Eye * vec4(fragmentPosition, 1.0)), shininess = vec3(0.5), ambient = vec3(0.0012);

vec3 applyDL(){
	if(lightFloats[0] == 0.0){
		return vec3(0.0);
	}
	
	vec3 tolight = normalize(-lightVec3s[1] - worldFP), 
	toV = -normalize(fragmentPosition), 
	hv = normalize(toV + tolight), 
	normal = normalize(fragmentNormal), 
	normalAlt = normalize(fragmentNormalAlt);
	float diffuseCo = max(0.0, dot(normal, tolight));
	vec3 diffuse = diffuseCo * fragmentColor * lightVec3s[0] * lightFloats[0];
	float specularCo = pow(max(0.0, dot(hv, normalAlt)), 64.0);
	vec3 specular = specularCo * shininess * lightVec3s[0] * lightFloats[0];
	
	return ambient + diffuse + specular;
}

vec3 applyPL(){
	if(lightFloats[1] == 0.0){
		return vec3(0.0);
	}
	
	vec3 tolight = normalize(lightVec3s[3] - worldFP), 
	toV = -normalize(fragmentPosition), 
	hv = normalize(toV + tolight), 
	normal = normalize(fragmentNormal), 
	normalAlt = normalize(fragmentNormalAlt);
	float diffuseCo = max(0.0, dot(normal, tolight));
	vec3 diffuse = diffuseCo * fragmentColor * lightVec3s[2] * lightFloats[1];
	float specularCo = pow(max(0.0, dot(hv, normalAlt)), 64.0);
	vec3 specular = specularCo * shininess * lightVec3s[2] * lightFloats[1];
	float dist = distance(lightVec3s[3], fragmentPosition), 
	attenuation = 1.0 / (1.0 + lightFloats[2] * pow(dist, 2));
	
	return ambient + (diffuse + specular) * attenuation;
}

vec3 applyS(){
	if(lightFloats[3] == 0.0){
		return vec3(0.0);
	}
	
	vec3 tolight = normalize(lightVec3s[5] - fragmentPosition), 
	toV = -normalize(fragmentPosition), 
	hv = normalize(toV + tolight), 
	normal = normalize(fragmentNormal), 
	normalAlt = normalize(fragmentNormalAlt);
	float diffuseCo = max(0.0, dot(normal, tolight));
	vec3 diffuse = diffuseCo * fragmentColor * lightVec3s[4] * lightFloats[3];
	float specularCo = pow(max(0.0, dot(hv, normalAlt)), 64.0);
	vec3 specular = specularCo * shininess * lightVec3s[4] * lightFloats[3];
	float dist = distance(lightVec3s[5], fragmentPosition), 
	attenuation = 1.0 / (1.0 + lightFloats[4] * pow(dist, 2)), 
	ltofa = acos(dot(-tolight, lightVec3s[6]));
	if(ltofa > lightFloats[5]){
		attenuation = 0.0;
	}
	
	return ambient + (diffuse + specular) * attenuation;
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
