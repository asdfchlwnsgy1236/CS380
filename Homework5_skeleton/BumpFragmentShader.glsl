#version 330 core

in vec3 fragmentPosition;
in vec3 fragmentNormal;
in vec2 UV;

in vec3 dltolight;
in vec3 dltov;
in vec3 dlrv;
in vec3 pltolight;
in vec3 pltov;
in vec3 plrv;
in vec3 stolight;
in vec3 stov;
in vec3 srv;

// Output data
out vec3 color;

uniform float lightFloats[6]; // dlIntensity, plIntensity, plAttenuationRatio, sIntensity, sAttenuationRatio, sConeAngle
uniform vec3 lightVec3s[7]; // dlColor, dlDirection, plColor, plLocation, sColor, sLocation, sDirection

uniform mat4 Eye;

uniform sampler2D myTextureSampler;
uniform sampler2D myBumpSampler;

vec3 shininess = vec3(0.25), ambient = vec3(0.0012), normal = texture(myBumpSampler, UV).rgb * 2.0 - 1.0, 
	textureColor = texture(myTextureSampler, UV).rgb;

vec3 applyDL(){
	if(lightFloats[0] == 0.0){
		return vec3(0.0);
	}
	
	float diffuseCo = max(0.0, dot(normal, dltolight)), specularCo = 0.0;
	vec3 diffuse = diffuseCo * textureColor * lightVec3s[0] * lightFloats[0], specular = vec3(0.0);
	specularCo = pow(max(0.0, dot(dlrv, dltov)), 64.0);
	specular = specularCo * shininess * lightVec3s[0] * lightFloats[0];
	
	return ambient + diffuse + specular;
}

vec3 applyPL(){
	if(lightFloats[1] == 0.0){
		return vec3(0.0);
	}
	
	float diffuseCo = max(0.0, dot(normal, pltolight)), specularCo = 0.0;
	vec3 diffuse = diffuseCo * textureColor * lightVec3s[2] * lightFloats[1], specular = vec3(0.0);
	specularCo = pow(max(0.0, dot(plrv, pltov)), 64.0);
	specular = specularCo * shininess * lightVec3s[2] * lightFloats[1];
	float dist = distance(lightVec3s[3], fragmentPosition), 
	attenuation = 1.0 / (1.0 + lightFloats[2] * pow(dist, 2));
	
	return ambient + (diffuse + specular) * attenuation;
}

vec3 applyS(){
	if(lightFloats[3] == 0.0){
		return vec3(0.0);
	}
	
	float diffuseCo = max(0.0, dot(normal, stolight)), specularCo = 0.0;
	vec3 diffuse = diffuseCo * textureColor * lightVec3s[4] * lightFloats[3], specular = vec3(0.0);
	specularCo = pow(max(0.0, dot(srv, stov)), 64.0);
	specular = specularCo * shininess * lightVec3s[4] * lightFloats[3];
	float dist = distance(lightVec3s[5], fragmentPosition), 
	attenuation = 1.0 / (1.0 + lightFloats[4] * pow(dist, 2)), 
	ltofa = acos(dot(-stolight, lightVec3s[6]));
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
