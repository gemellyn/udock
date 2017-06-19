#version 130

uniform sampler2D Texture0;
uniform sampler2D Texture1;
uniform float screen_width;
uniform float screen_height;
uniform vec4 colorFond;
uniform vec4 colorOverlay;

uniform mat4x4 mvp;
uniform mat4x4 p;
uniform mat4x4 ip;
uniform mat4x4 iv;

uniform float elapsed;

uniform vec3 kernel[64];
uniform vec3 sunShaftSource;


float LinearizeDepth(float z)
{
	float zNear = 1.0; // camera z near
	float zFar = 10000.0; // camera z far

	float z_n = z;//2.0 * z - 1.0;
	float z_e = (2.0 * zNear * zFar) / (zFar + zNear - z_n * (zFar - zNear));



	return z_e;
}


float zNdcToView(float zndc, float p43, float p33)
{
	return - p43 /((2*zndc-1) - p33);
}


float my_lerp(float a, float b, float alpha){
	return alpha * b + (1-alpha)*a;
}

vec3 saturate_color(vec3 color)
{
	
	vec3 finalc = color;
	for(int i=0;i<3;i++)
		if(color[i] > 1)
			for(int j=0;j<2;j++)
				finalc[(i+j)%3] += color[i] - 1.0;

	for(int i=0;i<3;i++)
		if(finalc[i] > 1)
			finalc[i] = 1;

		return finalc;
}

void main (void)
{
	float xstep = 1.0/screen_width;
	float ystep = 1.0/screen_height;
	float ratio = screen_width / screen_height;

	vec4 color = texture2D( Texture0 , vec2( gl_TexCoord[0] ) );
	float depthBase = texture2D( Texture1 , vec2( gl_TexCoord[0] ) ).r;	
	float depth = LinearizeDepth(depthBase);	
	
	//////////////////////////////////////
	/// CONTOUR
	//////////////////////////////////////
	float x,y,z;
	float sommedepth=0;
	float nb = 0;

	for(x=-1;x<1;x++)                               
	{
		for(y=-1;y<1;y++)
		{
			vec2 coord =  gl_TexCoord[0].xy;
			coord.x += x*xstep;
			coord.y += y*ystep;
			float depthLocal = texture2D( Texture1 , coord ).r;	
			depthLocal = LinearizeDepth(depthLocal);
			sommedepth += abs(depth - depthLocal) / depth; //plus on est loing, plus les differences de depth sont grandes cause perspective
			nb++;
		}
	}
	
	sommedepth /= nb;
	
	//////////////////////////////////////
	/// SSAO
	//////////////////////////////////////
	
	nb=0;
	float sommeDepthAmbOccl=0;
	float minz = 0.05;

	//on recup le coin en haut a gauche de l'image en coordonnées view
	vec4 hautGauche = vec4(-1,1,1,1);
	vec4 hautGaucheView = ip * hautGauche;
	
	vec4 basDroite = vec4(1,-1,1,1);
	vec4 basDroiteView = ip * basDroite;
	
	
	//Position du pixel courant en coords view
	vec4 posPixView;
	posPixView.x = my_lerp(hautGaucheView.x,basDroiteView.x,gl_TexCoord[0].x);
	posPixView.y = my_lerp(hautGaucheView.y,basDroiteView.y,1.0 - gl_TexCoord[0].y);
	posPixView.z = -1;
	float rapport = depth / 1.0; //near
	posPixView *= rapport;// / 10000.0;
	posPixView.w = 1.0;

	/*
	float i = gl_TexCoord[0].x * screen_width + gl_TexCoord[0].y * screen_height * screen_width;// + cos(elapsed*100);
	float cosi = cos(i);
	float sini = sin(i);
	mat3x3 rot = mat3x3(cosi,-sini,0,
						sini,cosi,0,
						0,0,1);

	int nbSamplesSSao = 16;

	int offset = int(abs(cosi)*(64-nbSamplesSSao-1));
	for(int i=0;i<nbSamplesSSao;i++)
	{		
		vec4 pos = vec4(rot*kernel[i+offset],0) * 8.0f;
		pos += posPixView;
		vec4 posimage = p * pos;
		posimage /= posimage.w;
		vec2 posTex = (posimage.xy + vec2(1,1))/2;

		if(posTex.x > 0 && posTex.x < 1 &&
           posTex.y > 0 && posTex.y < 1)
		{
			//On mate la depth a ce point la
			float depthLocal = texture2D( Texture1 , posTex ).r;	
			
			//on compare z et depth
			if(posimage.z > depthLocal)
				sommeDepthAmbOccl += 1;
			nb++;
		}
	}

	sommeDepthAmbOccl /= nb;
	sommeDepthAmbOccl = max(0.0,sommeDepthAmbOccl-0.5)*2.0;
	*/


	
	//////////////////////////////////////
	/// SUN SHAFT
	//////////////////////////////////////
	int NUM_SAMPLES = 40;
	float Density = 0.4;
	float Exposure = 0.5;
	float ExposureFactor = 0.6;
	float Decay = 0.95f;
	float Weight = 0.05f;
	float sizeSShaft = 3.0f;
	vec2 texCoord = gl_TexCoord[0].xy;
	//Image pos of sunshaft source
	vec4 srcImgPosW = mvp * vec4(sunShaftSource,1);
	vec4 srcImgPos = srcImgPosW/srcImgPosW.w;
	vec2 srcTexPos = (srcImgPos.xy + vec2(1,1))/2;
	// Calculate vector from pixel to light source in screen space.  
   	vec2 deltaTexCoord = (gl_TexCoord[0].xy - srcTexPos.xy);  
   	Exposure = min(0.7,max(0.0,1.0 - length(deltaTexCoord)/sizeSShaft));
   	Exposure *= ExposureFactor;
   	
   	if(srcImgPosW.z <= 0)
   		Exposure = 0;

	// Divide by number of samples and scale by control factor.  
	deltaTexCoord *= 1.0f / NUM_SAMPLES * Density;  
	// Store initial sample.  
	vec4 colorShaft = vec4(0,0,0,1);
	if(depth > 4000)
		colorShaft = color;  

	// Set up illumination decay factor.  
	float illuminationDecay = 1.0f;  
	// Evaluate summation from Equation 3 NUM_SAMPLES iterations.  
	for (int i = 0; i < NUM_SAMPLES; i++)  
	{  
		// Step sample location along ray.  
		texCoord -= deltaTexCoord;  
		// Retrieve Depth at new location.  
		float depth = LinearizeDepth(texture2D( Texture1 , texCoord ).r);	

		if(depth > 4000){
			// Retrieve sample at new location.  
			vec4 sample = texture2D( Texture0 , texCoord);
		
			// Apply sample attenuation scale/decay factors.  
			sample *= illuminationDecay * Weight; 
			 
			// Accumulate combined color.  
			colorShaft += sample;  

		}
		// Update exponential decay factor.  
		illuminationDecay *= Decay; 
	}  
  
  	colorShaft.xyz = saturate_color(colorShaft.xyz);
    colorShaft = vec4( colorShaft.xyz * Exposure, 1);  
	
	//////////////////////////////////////
	/// BROUILLARD
	//////////////////////////////////////
	float distEnd = 6000;
	float distMax = 3500;
	float start = 3000;
	if(depth > start && depth < distEnd){
		float coeff = clamp(((depth)-start) / (distMax-start),0,1);
		color = (1.0-coeff)*color + coeff * colorFond;
	}

	

	//////////////////////////////////////
	/// VIGNETTAGE
	//////////////////////////////////////
	vec2 vign = vec2( gl_TexCoord[0] );
	vign -= vec2(0.5,0.5);
	float facVign = max(0,(vign.x*vign.x + vign.y*vign.y)-0.35)*1.5;
	
	
	//////////////////////////////////////
	/// HORIZON
	//////////////////////////////////////
	vec4 posPixWorld = iv * posPixView;
	/*float barre = 300;
	if(posPixWorld.z > -barre && posPixWorld.z < barre && depthBase == 1)
		color *= 0.75 + abs(posPixWorld.z) / (barre*4);*/
	if(posPixWorld.z < 0 && depthBase == 1)
		color *= 0.92;
	
/*float seuilssao = 0.4;
	if(sommeDepthAmbOccl > seuilssao){
		//color *= (1-(sommeDepthAmbOccl-seuilssao)*(1-seuilssao));
		color *= (1-(sommeDepthAmbOccl-seuilssao));
		//color *= (1-(sommeDepthAmbOccl-seuilssao));
	}*/

		
	//Cerné
	if(sommedepth > 0.1)
	{
		color *= 0.5;
	} 
	
	color *= (1-facVign) ;

	color += colorOverlay;

	color += colorShaft;

	/*color = vec4(0,0,0,1);
	if(sommeDepthAmbOccl > seuilssao)
		color = vec4(sommeDepthAmbOccl,0,0,1);*/

	color.a = 1.0;
	gl_FragColor = color;
}

/*uniform sampler2D Texture0;
uniform sampler2D Texture1;
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

	vec4 color = texture2D( Texture0 , vec2( gl_TexCoord[0] ) );
	float depth = texture2D( Texture1 , vec2( gl_TexCoord[0] ) ).r;	
	
	//Permet de scaler la profondeur
	depth = LinearizeDepth(depth);	

	//Pour les contours		
	float x,y;
	float sommedepth=0;
	float nb = 0;

	for(x=-1;x<1;x++)                               
	{
		for(y=-1;y<1;y++)
		{
			vec2 coord =  gl_TexCoord[0].xy;
			coord.x += x*xstep;
			coord.y += y*ystep;
			float depthLocal = texture2D( Texture1 , coord ).r;	
			depthLocal = LinearizeDepth(depthLocal);
			sommedepth += abs(depth - depthLocal);
			nb++;
		}
	}
	
	sommedepth /= nb;
	
	


	if(sommedepth > 0.001)
	{
		color *= 0.5;
	} 
		

	//Vignettage	
	vec2 vign = vec2( gl_TexCoord[0] );
	vign -= vec2(0.5,0.5);
	
	
	float facVign = max(0,(vign.x*vign.x + vign.y*vign.y)-0.35)*1.5;


	color *= (1-facVign) ;

		
	color.a = 1.0;
	gl_FragColor = color;
}*/
