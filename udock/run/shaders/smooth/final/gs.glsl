#version 150
 
layout(triangles) in;
layout (triangle_strip, max_vertices=3) out;

//layout(points) in;
//layout (points, max_vertices=3) out;
 
//in vec3 vs_normal_out[3];
in vec4 vs_color_out[3];
in vec3 vertex_to_light_vector[3];
in vec3 vertex_in_modelview_space[3];

out vec3 gs_normal_out;
out vec4 gs_color_out;
out vec3 gs_vertex_to_light_vector;
out vec3 gs_vertex_in_modelview_space;
 
void main()
{
  vec3 normal = normalize(cross((gl_in[2].gl_Position-gl_in[0].gl_Position).xyz,(gl_in[1].gl_Position-gl_in[0].gl_Position).xyz));

  normal.z = - normal.z; //Don't ask me why, my generated normals have and inverted z...

  for(int i = 0; i < gl_in.length(); i++)
  {

    // copy attributes
    gl_Position = gl_in[i].gl_Position;
    gs_color_out = vs_color_out[i];
    gs_normal_out = normal;
    gs_color_out = vs_color_out[i];
    gs_vertex_to_light_vector = vertex_to_light_vector[i];
    gs_vertex_in_modelview_space = vertex_in_modelview_space[i];
 
    // done with the vertex
    EmitVertex();
  }

  EndPrimitive();
  
  
}