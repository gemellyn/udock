#version 400

layout (triangles) in;
layout (triangle_strip, max_vertices=6) out;
 
in VertexData {
    float nrgy;
    vec3 normal;
    vec4 color;
    vec3 vertex_to_light_vector;
    vec3 vertex_in_modelview_space;
} VertexIn[];
 
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
  EndPrimitive();

  for(int i = 0; i < gl_in.length(); i++)
  {
    gl_Position = gl_in[i].gl_Position;
    if(VertexIn[i].nrgy > 0)
      gl_Position += vec4(VertexIn[i].normal, 1);
    VertexOut.nrgy = VertexIn[i].nrgy;
    VertexOut.normal = VertexIn[i].normal;
    VertexOut.color = vec4(255,255,0,1);
    VertexOut.vertex_to_light_vector = VertexIn[i].vertex_to_light_vector;
    VertexOut.vertex_in_modelview_space = VertexIn[i].vertex_in_modelview_space;
    if(VertexIn[i].nrgy > 0)
    EmitVertex();
  }
  EndPrimitive();
}