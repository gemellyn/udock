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

uniform int projType;

layout (location=0) in vec4 vs_color_in;
layout (location=1) in vec3 vs_normal_in;
layout (location=2) in vec3 vs_position_in;
out vec4 vs_color_out;

const float pi = 3.1415926;

//float dist_max = 30.0;

vec4 projSphere(vec3 position)
{

	vec3 vertex = position;

	float dist = sqrt(vertex.x*vertex.x+vertex.y*vertex.y+vertex.z*vertex.z);
	float longitude = atan(vertex.y,vertex.x); // 0 face à l'axe plus x, ensuite + pi et -pi selon le coté ou l'on part
	float latitude = atan( sqrt(vertex.x*vertex.x+vertex.y*vertex.y),vertex.z);  //0 au pole nord, pi au pole sud 

	float rayon = molMaxSize;

	//Direct polar par defaut
	float x = longitude;
	float y = latitude - pi/2.0;
	float z = dist/rayon; 

	x /= pi; 
	y /= pi; 

	//Mercator (conformal)
	if(projType == 1)
	{
		latitude = log(tan(latitude/2.0));
		x = longitude;
		y = latitude;
		z = dist/rayon; 

		x /= pi; //6 si on veut tout voir en mercator (domaine de ln(tan(lat/2)) est [-6,6])
		y /= pi; //6 si on veut tout voir en mercator (domaine de ln(tan(lat/2)) est [-6,6])
	}

	//Peters (equal area)
	if(projType == 2)
	{
		latitude = 2*sin(latitude-pi/2.0);
		x = longitude;
		y = latitude;
		z = dist/rayon; //On inverse z pour le zbuffer, garder la plus loin !

		x /= pi;
		y /= pi; 
	}

	//On a x qui varie entre -1 et 1 et y qui varie entre -0.5 et 0.5 (si on veut pas croper)

	float xRatio = 1; //On fixe x au bord de l'ecran101
	float yRatio = ratio; //On varie y en fonction du ratio

	if(ratio > 2)
	{
		yRatio = 2.0; //On fixe y au bord de l'écran
		xRatio = (1 / ratio) * 2.0; //On diminue x en fonction
	}


	return vec4(x * xRatio,y * yRatio,1-z,1); 
}

void main()
{
	vec4 vertex =  m * vec4(vs_position_in,1) ;
	vertex -= pos;

	gl_Position = projSphere(vertex.xyz);
	float depth = (1.0-gl_Position.z) ;


	//depth = 1-depth;

	depth *= 2; //boostm

	float attenue = 1.25;
	depth /= attenue;
	depth += 1-(1.0/attenue);

	//depth = 0.85;

	vs_color_out = vec4(vs_color_in.xyz * depth,vs_color_in.a);


	
}
