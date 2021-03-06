#version 330 core

in vec3 fragmentPosition;
in vec3 fragmentNormal;
in vec2 UV;

// Output data
out vec3 color;

uniform float lightFloats[6]; // dlIntensity, plIntensity, plAttenuationRatio, sIntensity, sAttenuationRatio, sConeAngle
uniform vec3 lightVec3s[7]; // dlColor, dlDirection, plColor, plLocation, sColor, sLocation, sDirection

uniform mat4 Eye;

uniform sampler2D myTextureSampler;

vec3 shininess = vec3(0.5), ambient = vec3(0.0012), textureColor = texture(myTextureSampler, UV).rgb;

vec3 applyDL(){
	if(lightFloats[0] == 0.0){
		return vec3(0.0);
	}
	
	vec3 tolight = -lightVec3s[1], 
	toV = normalize(Eye[3].xyz - fragmentPosition), 
	normal = normalize(fragmentNormal), 
	rv = reflect(-tolight, normal);
	float diffuseCo = max(0.0, dot(normal, tolight)), specularCo = 0.0;
	vec3 diffuse = diffuseCo * textureColor * lightVec3s[0] * lightFloats[0], specular = vec3(0.0);
	specularCo = pow(max(0.0, dot(rv, toV)), 64.0);
	specular = specularCo * shininess * lightVec3s[0] * lightFloats[0];
	
	return ambient + diffuse + specular;
}

vec3 applyPL(){
	if(lightFloats[1] == 0.0){
		return vec3(0.0);
	}
	
	vec3 tolight = normalize(lightVec3s[3] - fragmentPosition), 
	toV = normalize(Eye[3].xyz - fragmentPosition), 
	normal = normalize(fragmentNormal), 
	rv = reflect(-tolight, normal);
	float diffuseCo = max(0.0, dot(normal, tolight)), specularCo = 0.0;
	vec3 diffuse = diffuseCo * textureColor * lightVec3s[2] * lightFloats[1], specular = vec3(0.0);
	specularCo = pow(max(0.0, dot(rv, toV)), 64.0);
	specular = specularCo * shininess * lightVec3s[2] * lightFloats[1];
	float dist = distance(lightVec3s[3], fragmentPosition), 
	attenuation = 1.0 / (1.0 + lightFloats[2] * pow(dist, 2));
	
	return ambient + (diffuse + specular) * attenuation;
}

vec3 applyS(){
	if(lightFloats[3] == 0.0){
		return vec3(0.0);
	}
	
	vec3 tolight = normalize(lightVec3s[5] - fragmentPosition), 
	toV = normalize(Eye[3].xyz - fragmentPosition), 
	normal = normalize(fragmentNormal), 
	rv = reflect(-tolight, normal);
	float diffuseCo = max(0.0, dot(normal, tolight)), specularCo = 0.0;
	vec3 diffuse = diffuseCo * textureColor * lightVec3s[4] * lightFloats[3], specular = vec3(0.0);
	specularCo = pow(max(0.0, dot(rv, toV)), 64.0);
	specular = specularCo * shininess * lightVec3s[4] * lightFloats[3];
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
