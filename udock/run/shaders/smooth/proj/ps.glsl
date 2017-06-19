
#version 130

in vec4 gs_color_out;
out vec4 ps_color_out;

void main()
{
	ps_color_out = gs_color_out;
}