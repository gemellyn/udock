#version 130

in vec4 gs_color_out;
out vec4 ps_color_out;

uniform sampler2D colorTex1;
uniform sampler2D depthTex1;
uniform float screen_width;
uniform float screen_height;

uniform float smoothing;

uniform int precise;

void main (void)
{
	float xstep = 1.0/screen_width;
	float ystep = 1.0/screen_height;
	float ratio = screen_width / screen_height;


	float profBase = (texture2D( depthTex1 , gl_TexCoord[0].xy )).x;
	vec3 color = texture2D( colorTex1 , gl_TexCoord[0].xy ).rgb;

	float profmoy = profBase;
	vec3 colorMoy = color;
	float nb = 1;
	int size = int(20*profBase * smoothing); 
	int step = max((size/6),1);

	if(precise == 1)
		step = 1;

	for(int i=-size;i<size;i+=step)
	{
		for(int j=-size;j<size;j+=step)
		{
			float prof = (texture2D( depthTex1 , vec2(gl_TexCoord[0].x+i*xstep,gl_TexCoord[0].y+j*ystep) )).x;
			if(abs(prof) < 1)
			{
			 	profmoy += prof;
				colorMoy += texture2D( colorTex1 , vec2(gl_TexCoord[0].x+i*xstep,gl_TexCoord[0].y+j*ystep) ).rgb;
				nb ++;
			}
		}
	}

	profmoy /= nb;
	colorMoy /= nb;

	//color = vec3(profmoy,profmoy,profmoy);

	//color = texture2D( depthTex1 , gl_TexCoord[0] ).rgb;

	gl_FragColor = vec4(colorMoy.r,colorMoy.g,colorMoy.b,1);
	gl_FragDepth = profmoy;
}
