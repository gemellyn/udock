#version 150
 
layout(triangles) in;
layout (triangle_strip, max_vertices=12) out;

//layout(points) in;
//layout (points, max_vertices=3) out;
 
in vec4 vs_color_out[3];
out vec4 gs_color_out;
 
void main()
{

  float x1 = abs(gl_in[0].gl_Position.x - gl_in[1].gl_Position.x);
  float x2 = abs(gl_in[1].gl_Position.x - gl_in[2].gl_Position.x);
  float x3 = abs(gl_in[0].gl_Position.x - gl_in[2].gl_Position.x);

  float xmax = max(max(x1,x2),x3);

  float seuil = 1.5f;

  if(xmax < seuil)
  {
  
    for(int i = 0; i < gl_in.length(); i++)
    {
      // copy attributes
      gl_Position = gl_in[i].gl_Position;
      gs_color_out = vs_color_out[i];
      // done with the vertex
      EmitVertex();
    }
    EndPrimitive();
  }
  else
  {
    
    for(int i = 0; i < gl_in.length(); i++)
    {

      //tout le monde a droite
       gl_Position = gl_in[i].gl_Position;
      if(gl_Position.x < 0)
        gl_Position.x += 2.0;
      gs_color_out = vs_color_out[i];
   
      // done with the vertex
      EmitVertex();
    }
     EndPrimitive();

     for(int i = 0; i < gl_in.length(); i++)
    {

      //tout le monde a droite
      gl_Position = gl_in[i].gl_Position;
      if(gl_Position.x > 0)
        gl_Position.x -= 2.0;
      gs_color_out = vs_color_out[i];
   
      // done with the vertex
      EmitVertex();
    }
     EndPrimitive();
      
    
  }
}