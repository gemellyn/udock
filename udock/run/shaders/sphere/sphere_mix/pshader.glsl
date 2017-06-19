uniform sampler2D colorTex1;
uniform sampler2D depthTex1;
uniform sampler2D colorTex2;
uniform sampler2D depthTex2;
uniform float screen_width;
uniform float screen_height;

float LinearizeDepth(float z)
{
	float n = 1.0; // camera z near
  	float f = 2000.0; // camera z far
  	return (2.0 * n) / (f + n - z * (f - n));
}

void main (void)
{
	float xstep = 1.0/screen_width;
	float ystep = 1.0/screen_height;
	float ratio = screen_width / screen_height;

	vec4 color = texture2D( colorTex1 , vec2( gl_TexCoord[0] ) );
	float depth = texture2D( depthTex1 , vec2( gl_TexCoord[0] ) ).r;	

	vec4 color2 = texture2D( colorTex2 , vec2( gl_TexCoord[0] ) );
	float depth2 = texture2D( depthTex2 , vec2( gl_TexCoord[0] ) ).r;	

	//Permet de scaler la profondeur
	//depth = LinearizeDepth(depth);

	//float distance = max(abs(color.r - color2.b),abs(color.b - color2.r));

	float charge1pos = color.r - color.b;
	if(charge1pos < 0)
		charge1pos = 0;
	float charge1neg = color.b - color.r;
	if(charge1neg < 0)
		charge1neg = 0;
	float charge1 = (charge1pos > charge1neg) ? charge1pos : -charge1neg;

	float charge2pos = color2.r - color2.b;
	if(charge2pos < 0)
		charge2pos = 0;
	float charge2neg = color2.b - color2.r;
	if(charge2neg < 0)
		charge2neg = 0;
	float charge2 = (charge2pos > charge2neg) ? charge2pos : -charge2neg;


	//float distance = (color.r * color2.b);// - color.b * color2.b - color.r * color2.r;
	//distance = abs(depth - depth2) *10;

	float distance = ((- (charge2 * charge1))+1)/2;// - color.b * color2.b - color.r * color2.r;

	gl_FragColor = vec4(distance,distance,distance,1);

	//gl_FragColor = 0.5*color + 0.5*color2;



}