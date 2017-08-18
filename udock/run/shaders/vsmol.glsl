#version 400

uniform vec4 colorPositif;
uniform vec4 colorNegatif;
uniform vec4 colorNeutre;
uniform int invertPolarity;

uniform mat4x4 mvp;
uniform mat4x4 mv;
uniform mat4x4 nmat;
uniform mat4x4 m;
uniform mat4x4 iv;

uniform vec3 lightpos;

layout (location=0) in vec4 vs_color_in;
layout (location=1) in vec3 vs_normal_in;
layout (location=2) in vec3 vs_position_in;
layout (location=3) in float vs_nrgy_in;

out VertexData {
    float nrgy;
    vec3 normal;
    vec4 color;
    vec3 vertex_to_light_vector;
    vec3 vertex_in_modelview_space;
} VertexOut;

void main()
{
	// Transforming The Vertex
	gl_Position = mvp * vec4(vs_position_in,1);

	// Transforming The Normal To ModelView-Space
	VertexOut.normal = (nmat * vec4(vs_normal_in,1)).xyz;  

	// Transforming The Vertex Position To ModelView-Space
	VertexOut.vertex_in_modelview_space = (mv * vec4(vs_position_in,1)).xyz;

	// Calculating The Vector From The Vertex Position To The Light Position
	//vertex_to_light_vector = vec3(gl_LightSource[0].position) - vertex_in_modelview_space;

	VertexOut.vertex_to_light_vector = (mvp * vec4(lightpos,1)).xyz;

	VertexOut.color = vs_color_in;
		
	//Positif
	if(vs_color_in.r < 1.0)
	{
		if(invertPolarity == 0)
			VertexOut.color = colorPositif * (1-vs_color_in.r) + colorNeutre * vs_color_in.r;
		else
			VertexOut.color = colorNegatif * (1-vs_color_in.r) + colorNeutre * vs_color_in.r;
	}

	//Negatif
	if(vs_color_in.b < 1.0)
	{
		if(invertPolarity == 0)
			VertexOut.color = colorNegatif * (1-vs_color_in.b) + colorNeutre * vs_color_in.b;
		else
			VertexOut.color = colorPositif * (1-vs_color_in.b) + colorNeutre * vs_color_in.b;
	}

	//energie debug
	/*
	if(vs_nrgy_in > 0)
	{
		VertexOut.color = mix(VertexOut.color, vec4(0,vs_nrgy_in,0,1), vs_nrgy_in);
	}*/

	VertexOut.nrgy = vs_nrgy_in;

	VertexOut.color.a = vs_color_in.a;
}
