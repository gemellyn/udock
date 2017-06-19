#version 130

uniform float ambientLevel;
uniform float tailleCerne;

in vec3 gs_normal_out;
in vec4 gs_color_out;
in vec3 gs_vertex_to_light_vector;
in vec3 gs_vertex_in_modelview_space;

out vec4 final_color;

void main()
{
	// Scaling The Input Vector To Length 1
	vec3 normalized_normal = normalize(gs_normal_out); 
	vec3 normalized_vertex_to_light_vector = normalize(gs_vertex_to_light_vector);

	//Calculating The Diffuse Term And Clamping It To [0;1]
	float DiffuseTerm = clamp(dot(normalized_normal, normalized_vertex_to_light_vector), 0.0, 1.0);

	//Pour les bords
	vec3 normalized_cam_dir = normalize(gs_vertex_in_modelview_space);

	float Bord = clamp(dot((-1*gs_normal_out), normalized_cam_dir), 0.0, 1.0);

	//Debug
	//final_color = vec4(gs_normal_out,1.0);//AmbientColor + color * DiffuseTerm;
	//final_color = vec4(normalized_vertex_to_light_vector,1.0);//AmbientColor + color * DiffuseTerm;

	// Calculating The Final Color
	final_color = gs_color_out * (DiffuseTerm*(1-ambientLevel) + ambientLevel);

	if(Bord < tailleCerne)
		final_color *= 0.4;

	final_color.a = gs_color_out.a;
}