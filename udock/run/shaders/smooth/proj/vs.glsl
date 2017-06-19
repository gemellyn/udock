#version 400

uniform vec4 colorPositif;
uniform vec4 colorNegatif;
uniform vec4 colorNeutre;
uniform int invertPolarity;
uniform vec4 pos;
uniform vec4 barycenter;
uniform float ratio; //width / height
uniform mat4x4 rotation; //width / height

uniform mat4x4 mvp;
uniform mat4x4 mv;
uniform mat4x4 nmat;
uniform mat4x4 m;
uniform mat4x4 iv;

uniform float xoffset;
uniform float yoffset;

uniform int invert;

uniform float molMaxSize;

layout (location=0) in vec4 vs_color_in;
layout (location=1) in vec3 vs_normal_in;
layout (location=2) in vec3 vs_position_in;

out vec4 vs_color_out;

const float pi = 3.1415926;

//float dist_max = 28.11; 

vec4 projSphere(vec3 position)
{

	vec3 vertex = position;

	float dist = sqrt(vertex.x*vertex.x+vertex.y*vertex.y+vertex.z*vertex.z);
	float longitude = atan(vertex.y,vertex.x); // 0 face à l'axe plus x, ensuite + pi et -pi selon le coté ou l'on part
	float latitude = atan( sqrt(vertex.x*vertex.x+vertex.y*vertex.y),vertex.z);  //0 au pole nord, pi au pole sud 


	//Spherique
	float x = (longitude / pi);
	float y = ((latitude  / pi) - 0.5);
	float z = dist/molMaxSize; //On inverse z pour le zbuffer, garder la plus loin !

	//x *= 0.6;
	//y *= 0.6;

	return vec4(x,y*ratio,1-z,1); 
}

void main()
{
	vec4 position = projSphere(vs_position_in);
	position.z = 2*position.z - 1; //On la veut entre -1 et 1, mais ensuite elle est clampée entre 0 et 1 ar le pipeline (gldepthclamp); 
	gl_Position = position;
	vs_color_out = vs_color_in;
}
