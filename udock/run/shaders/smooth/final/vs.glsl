#version 400

uniform sampler2D colorTex1;
uniform sampler2D depthTex1;
uniform float screen_width;
uniform float screen_height;

uniform vec4 colorPositif;
uniform vec4 colorNegatif;
uniform vec4 colorNeutre;
uniform int invertPolarity;

uniform vec4 pos;
uniform vec4 barycenter;
uniform float ratio; //width / height

uniform float morphing; //width / height

uniform mat4x4 mvp;
uniform mat4x4 mv;
uniform mat4x4 nmat;
uniform mat4x4 m;
uniform mat4x4 v;
uniform mat4x4 p;
uniform mat4x4 iv;

uniform vec3 lightpos;

uniform float molMaxSize;

layout (location=0) in vec4 vs_color_in;
layout (location=1) in vec3 vs_normal_in;
layout (location=2) in vec3 vs_position_in;

out vec3 vs_normal_out;
out vec4 vs_color_out;
out vec3 vertex_to_light_vector;
out vec3 vertex_in_modelview_space;

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
	float pix_size_x = 1.0/screen_width;
	float pix_size_y = 1.0/screen_height;

	//On projette sur une sphere
	vec4 position = projSphere(vs_position_in);	

	//Les deux profs
	float prof_locale = position.z;

	float x = (position.x+1.0)/2.0;
	float y = (position.y+1.0)/2.0;
	float profmoy = texture2D( depthTex1 , vec2(x,y) ).r;

	//On recup la couleur dans la tex ce coup ci
	vs_color_out = texture2D( colorTex1 , vec2(x,y) );

	//Delta entre les profondeurs
	float delta = (prof_locale - profmoy) * molMaxSize;

	vec4 vert_mod_space = m * vec4(vs_position_in,1) ;
	vec4 centre = pos;
	vec3 dir = (vert_mod_space - centre).xyz;
	dir = normalize(dir);

	float alpha = clamp(morphing,0,1);

	vert_mod_space += vec4(dir,0) * alpha * delta;


	// Transforming The Vertex
	gl_Position = p * v * vert_mod_space;
	
	// Transforming The Normal To ModelView-Space
	vs_normal_out = (nmat * vec4(vs_normal_in,1)).xyz;  

	// Transforming The Vertex Position To ModelView-Space
	vertex_in_modelview_space = (mv * vec4(vs_position_in,1)).xyz;

	// Calculating The Vector From The Vertex Position To The Light Position
	vertex_to_light_vector = (mvp * vec4(lightpos,1)).xyz;

	//vs_color_out.r = profmoy+0.1;
		
	//Positif
	/*if(vs_color_in.r < 1.0)
	{
		if(invertPolarity == 0)
			vs_color_out = colorPositif * (1-vs_color_in.r) + colorNeutre * vs_color_in.r;
		else
			vs_color_out = colorNegatif * (1-vs_color_in.r) + colorNeutre * vs_color_in.r;
	}

	//Negatif
	if(vs_color_in.b < 1.0)
	{
		if(invertPolarity == 0)
			vs_color_out = colorNegatif * (1-vs_color_in.b) + colorNeutre * vs_color_in.b;
		else
			vs_color_out = colorPositif * (1-vs_color_in.b) + colorNeutre * vs_color_in.b;
	}*/

	vs_color_out.a = vs_color_in.a;

	//vs_color_out = vec4(delta,delta,delta,1); 

	
}
