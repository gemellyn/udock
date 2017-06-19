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

  float y1 = abs(gl_in[0].gl_Position.y - gl_in[1].gl_Position.y);
  float y2 = abs(gl_in[1].gl_Position.y - gl_in[2].gl_Position.y);
  float y3 = abs(gl_in[0].gl_Position.y - gl_in[2].gl_Position.y);

  float ymax = max(max(y1,y2),y3);

  float seuil = 0.5f;

  if(xmax < seuil && ymax < seuil)
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
    /*if(ymax >= seuil)
    {
        for(int i = 0; i < gl_in.length(); i++)
        {

          //tout le monde a droite
          gl_Position = gl_in[i].gl_Position;
          if(gl_Position.y < 0)
            gl_Position.y += 2.0;
          gs_color_out = vs_color_out[i];
       
          // done with the vertex
          EmitVertex();
        }
         EndPrimitive();

         for(int i = 0; i < gl_in.length(); i++)
        {

          //tout le monde a droite
          gl_Position = gl_in[i].gl_Position;
          if(gl_Position.y > 0)
            gl_Position.y -= 2.0;
          gs_color_out = vs_color_out[i];
       
          // done with the vertex
          EmitVertex();
        }
         EndPrimitive();
      
    }

    //on corrige le point
    else */

      if(xmax >= seuil)
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

     /*gl_Position = gl_in[0].gl_Position;
     gs_color_out = vs_color_out[0];
    EmitVertex();
    EndPrimitive();*/
}