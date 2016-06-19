#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 3) in vec2 vertexUV;
layout(location = 4) in vec3 tangents;

// Output data ; will be interpolated for each fragment.
out vec3 fragmentPosition;
out vec3 fragmentNormal;
out vec2 UV;

out vec3 dltolight;
out vec3 dltov;
out vec3 dlrv;
out vec3 pltolight;
out vec3 pltov;
out vec3 plrv;
out vec3 stolight;
out vec3 stov;
out vec3 srv;

uniform float lightFloats[6]; // dlIntensity, plIntensity, plAttenuationRatio, sIntensity, sAttenuationRatio, sConeAngle
uniform vec3 lightVec3s[7]; // dlColor, dlDirection, plColor, plLocation, sColor, sLocation, sDirection

uniform mat4 ModelTransform;
uniform mat4 Eye;
uniform mat4 Projection;

void main(){
	// Output position of the vertex, in clip space : MVP * position
	vec4 wPosition = ModelTransform * vec4(vertexPosition_modelspace, 1.0);
	fragmentPosition = wPosition.xyz;
	gl_Position = Projection * inverse(Eye) * wPosition;
	
	//transpose of inversed model matrix
	mat4 NM = transpose(inverse(ModelTransform));
	vec4 tnormal = vec4(vertexNormal_modelspace, 0.0);
	fragmentNormal = vec3(NM * tnormal);
	
	UV = vertexUV;
	
	vec3 n = vec3(NM * tnormal), t = normalize(NM * vec4(tangents, 0.0)).xyz, b = cross(n, t), pos = wPosition.xyz;
	
	if(lightFloats[0] > 0.0){
		vec3 tolight = -lightVec3s[1], tov = normalize(Eye[3].xyz - pos), rv = reflect(-tolight, n);
		
		dltolight = normalize(vec3(dot(tolight, t), dot(tolight, b), dot(tolight, n)));
		dltov = normalize(vec3(dot(tov, t), dot(tov, b), dot(tov, n)));
		dlrv = normalize(vec3(dot(rv, t), dot(rv, b), dot(rv, n)));
	}
	if(lightFloats[1] > 0.0){
		vec3 tolight = normalize(lightVec3s[3] - pos), tov = normalize(Eye[3].xyz - pos), 
			rv = reflect(-tolight, n);
		
		pltolight = normalize(vec3(dot(tolight, t), dot(tolight, b), dot(tolight, n)));
		pltov = normalize(vec3(dot(tov, t), dot(tov, b), dot(tov, n)));
		plrv = normalize(vec3(dot(rv, t), dot(rv, b), dot(rv, n)));
	}
	if(lightFloats[3] > 0.0){
		vec3 tolight = normalize(lightVec3s[5] - pos), tov = normalize(Eye[3].xyz - pos), 
			rv = reflect(-tolight, n);
		
		stolight = normalize(vec3(dot(tolight, t), dot(tolight, b), dot(tolight, n)));
		stov = normalize(vec3(dot(tov, t), dot(tov, b), dot(tov, n)));
		srv = normalize(vec3(dot(rv, t), dot(rv, b), dot(rv, n)));
	}
}
