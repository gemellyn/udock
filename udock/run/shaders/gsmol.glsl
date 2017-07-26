#version 400

layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;
 
in VertexData {
    float nrgy;
    vec3 normal;
    vec4 color;
    vec3 vertex_to_light_vector;
    vec3 vertex_in_modelview_space;
} VertexIn[3];
 
out VertexData {
    float nrgy;
    vec3 normal;
    vec4 color;
    vec3 vertex_to_light_vector;
    vec3 vertex_in_modelview_space;
} VertexOut;
 
 void main()
{
  for(int i = 0; i < gl_in.length(); i++)
  {
    gl_Position = gl_in[i].gl_Position;
    VertexOut.nrgy = VertexIn[i].nrgy;
    VertexOut.normal = VertexIn[i].normal;
    VertexOut.color = VertexIn[i].color;
    VertexOut.vertex_to_light_vector = VertexIn[i].vertex_to_light_vector;
    VertexOut.vertex_in_modelview_space = VertexIn[i].vertex_in_modelview_space;
 
    EmitVertex();
  }
}