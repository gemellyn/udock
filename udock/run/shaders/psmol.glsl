#version 400

uniform float ambientLevel;
uniform float tailleCerne;

in VertexData {
	float nrgy;
    vec3 normal;
    vec4 color;
    vec3 vertex_to_light_vector;
    vec3 vertex_in_modelview_space;
} VertexIn;

out vec4 final_color;

void main()
{
	// Scaling The Input Vector To Length 1
	vec3 normalized_normal = normalize(VertexIn.normal);
	vec3 normalized_vertex_to_light_vector = normalize(VertexIn.vertex_to_light_vector);

	//Calculating The Diffuse Term And Clamping It To [0;1]
	float DiffuseTerm = clamp(dot(normalized_normal, normalized_vertex_to_light_vector), 0.0, 1.0);

	//Pour les bords
	vec3 normalized_cam_dir = normalize(VertexIn.vertex_in_modelview_space);

	float Bord = clamp(dot((-1*VertexIn.normal), normalized_cam_dir), 0.0, 1.0);

	//Debug
	//final_color = vec4(vs_normal_out,1.0);//AmbientColor + color * DiffuseTerm;
	//final_color = vec4(normalized_vertex_to_light_vector,1.0);//AmbientColor + color * DiffuseTerm;

	// Calculating The Final Color
	final_color = VertexIn.color * (DiffuseTerm*(1-ambientLevel) + ambientLevel);
	

	if(Bord < tailleCerne && VertexIn.nrgy > 0)
		final_color *= 0.4;

	final_color.a = VertexIn.color.a;
}