#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include <cstdio>
#include <stdlib.h>
#include <math.h>
#include <iostream>


#include "engine/utils/types_3d.h"
#include "engine/render/camera.h"
#include "engine/render/text/text_engine.h"
#include "engine/render/graph/tex_manager.h"
#include "engine/log/log.h"

//BACKGROUND COLOR FOND
#define ROUGE_FOND 176.0f/255.0f
#define VERT_FOND 192.0f/255.0f
#define BLEU_FOND 198.0f/255.0f

#define DEFAULT_SCREEN_WIDTH 800	
#define DEFAULT_SCREEN_HEIGHT 600

#define NB_PASSES_MAX 10

class NYFbo
{
	public :
		GLuint * _ColorTex;
		int _NbColorTex;
		GLuint _DepthTex;
		GLuint _FBO;
		int _Width;
		int _Height;
		

		NYFbo(int nbOutTex)
		{
			_ColorTex = new GLuint[nbOutTex];
			memset(_ColorTex, 0x00, sizeof(GLuint) *nbOutTex);
			_NbColorTex = nbOutTex;
			_DepthTex = 0;
			_FBO = 0;
		}

		~NYFbo()
		{
			SAFEDELETE_TAB(_ColorTex);
		}

		void init(int width, int height)
		{
			_Width = width;
			_Height = height;
			createColorTexs(width, height);
			createFBO();
		}

		void setColorAsShaderInput(int numCol = 0, int location = GL_TEXTURE0, char * texSamplerName = "colorTex1")
		{
			GLint prog;
			glGetIntegerv(GL_CURRENT_PROGRAM, &prog);

			GLuint texLoc = glGetUniformLocation(prog, texSamplerName);
			checkGlError("glGetUniformLocation(prog, texSamplerName);");
			glUniform1i(texLoc, location - GL_TEXTURE0);
			checkGlError("glUniform1i(texLoc, location- GL_TEXTURE0);");

			glActiveTexture(location);
			glBindTexture(GL_TEXTURE_2D, _ColorTex[numCol]);

			//reset
			glActiveTexture(GL_TEXTURE0);
		}

		void setDepthAsShaderInput(int location, char * texSamplerName)
		{
			GLint prog;
			glGetIntegerv(GL_CURRENT_PROGRAM, &prog);

			GLuint texLoc = glGetUniformLocation(prog, texSamplerName);
			glUniform1i(texLoc, location- GL_TEXTURE0);
			checkGlError("glUniform1i(texLoc, location);");

			glActiveTexture(location);
			glBindTexture(GL_TEXTURE_2D, _DepthTex);
		}

		static void loadFileToBuffer(const char * name, uint8 * bufferOut, int buffSize)
		{
			NYTexManager * texMan = NYTexManager::getInstance();

			png_image image;
			png_bytep buffer;
			uint32 i;
			memset(&image, 0x00, sizeof(png_image));

			if (texMan->loadImageFile_PNG(std::string(name), &image, &buffer))
				return;

			uint32 nbPixels = image.width * image.height;

			if (nbPixels * 3 != buffSize)
				Log::log(Log::USER_ERROR, "Cannot load png to buffer, bad buffer size");

			//remove la composante alpha
			for (int x = 0; x < image.width; x++)
			{
				for (int y = 0; y < image.height; y++)
				{
					int pix1 = (x + y*image.width);
					int pix2 = (x + (image.height - y - 1)*image.width);
					bufferOut[(3 * pix1) + 0] = buffer[(4 * pix2) + 0];
					bufferOut[(3 * pix1) + 1] = buffer[(4 * pix2) + 1];
					bufferOut[(3 * pix1) + 2] = buffer[(4 * pix2) + 2];
				}
			}
		}

		void loadFromFile(const char * name, int numCol)
		{
			NYTexManager * texMan = NYTexManager::getInstance();

			png_image image;
			png_bytep buffer;
			uint32 i;
			memset(&image, 0x00, sizeof(png_image));

			if (texMan->loadImageFile_PNG(std::string(name), &image, &buffer))
				return;

			if (_Width != image.width || _Height != image.height)
				Log::log(Log::USER_ERROR, "Loading tex in FBO with bad height or width");

			uint32 nbPixels = _Width * _Height;
			uint8 * pixelsRgb = new uint8[nbPixels * 3];

			if (!pixelsRgb)
				return;

			//remove la composante alpha
			for (int x = 0; x < min((int)image.width,_Width); x++)
			{
				for (int y = 0; y < min((int)image.height,_Height); y++)
				{
					int pix1 = (x + y*_Width);
					int pix2 = (x + (image.height-y-1)*image.width);
					pixelsRgb[(3 * pix1) + 0] = buffer[(4 * pix2) + 0];
					pixelsRgb[(3 * pix1) + 1] = buffer[(4 * pix2) + 1];
					pixelsRgb[(3 * pix1) + 2] = buffer[(4 * pix2) + 2];
				}
			}

			if (buffer != NULL)
				free(buffer);
				

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, _ColorTex[numCol]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _Width, _Height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelsRgb);
			
			glBindTexture(GL_TEXTURE_2D, 0);

			SAFEDELETE_TAB(pixelsRgb);
		}

		void readFb(int numCol, uint8 * buff, uint32 bufSize)
		{
			glReadPixels((GLint)0, (GLint)0,
				(GLint)_Width, (GLint)_Height,
				GL_RGB, GL_UNSIGNED_BYTE, buff);
		}

		void readFbTex(int numCol, uint8 * buff, uint32 bufSize)
		{
			int width, height;

			if (!buff)
			{
				Log::log(Log::ENGINE_ERROR, "Fbo read fail, buf is null");
				return;
			}
			
			uint32 nbPixels = _Width * _Height;
			if (nbPixels * 3 != bufSize)
			{
				Log::log(Log::ENGINE_ERROR, "Fbo read fail buffer has bad size");
				return;
			}

			glActiveTexture(GL_TEXTURE0);

			glBindTexture(GL_TEXTURE_2D, _ColorTex[numCol]);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

			if (width != _Width || height != _Height)
			{
				Log::log(Log::ENGINE_ERROR, "Fbo tex has not buff size, cannot read it");
				return;
			}

			glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
			glPixelStorei(GL_PACK_ALIGNMENT, 1); //par defaut word aligned et padding deborde ?	

			/*int val;
			glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_INTERNAL_FORMAT, &val);
			glGetIntegerv(GL_PACK_ALIGNMENT, &val);*/

			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, buff);
			checkGlError("glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, buff);");

			glBindTexture(GL_TEXTURE_2D, 0);

		}

		void saveToFile(const char * name, int numCol, bool useCurrentFb = false)
		{
			uint32 nbPixels = _Width * _Height;
			uint8 * pixelsRgb = (uint8*)malloc(nbPixels * 3);

			if (!pixelsRgb)
			{
				Log::log(Log::ENGINE_ERROR, "Fbo read buf alloc failed");
				return;
			}

			if(useCurrentFb)
				readFb(numCol, pixelsRgb, nbPixels * 3);
			else
				readFbTex(numCol, pixelsRgb, nbPixels * 3);

			NYTexManager::writeImage(name, _Width, _Height, pixelsRgb, "fbo save", true);	

			free(pixelsRgb);
		}

private:
		void createColorTexs(int width, int height)
		{
			if (_ColorTex[0] != 0)
				glDeleteTextures(_NbColorTex,_ColorTex);
			glGenTextures(_NbColorTex, _ColorTex);

			for (int i = 0; i < _NbColorTex;i++)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, _ColorTex[i]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f); //no aniso filtering
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			}
			
		}

		void createDepthTex(int width, int height)
		{
			if (_DepthTex != 0)
				glDeleteTextures(1, &_DepthTex);
			glGenTextures(1, &_DepthTex);
			glBindTexture(GL_TEXTURE_2D, _DepthTex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		}

		//// !! Creer les tex avnt de creer les FBO
		void createFBO()
		{
			if (_FBO != 0)
				glDeleteFramebuffers(1, &_FBO);

			glGenFramebuffers(1, &_FBO);
			
			//On bind le FBO pour tester si tout est ok
			glBindFramebuffer(GL_FRAMEBUFFER, _FBO);

			//Attach 2D texture to this FBO
			for (int i = 0; i < _NbColorTex;i++)
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, _ColorTex[i], 0);
			
			//Attach depth texture to FBO
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _DepthTex, 0);
				
			//Does the GPU support current FBO configuration?
			GLenum status;
			status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			switch (status)
			{
				case GL_FRAMEBUFFER_COMPLETE:
					//_cprintf("GPU ok for VBO with depth and color\n");
					break;
				default:
					Log::log(Log::ENGINE_ERROR, "GPU does not support VBO");
					Log::log(Log::USER_ERROR, "You graphic card is not able to run this software (no VBO support)");
					return;
			}
			
			//On debind
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		static void checkGlError(const char * call)
		{
			GLenum error = glGetError();

			if (error != 0)
			{
				switch (error)
				{
				case GL_INVALID_ENUM: Log::log(Log::ENGINE_ERROR, ("Opengl error (GL_INVALID_ENUM) for call " + toString(call)).c_str()); break;
				case GL_INVALID_OPERATION: Log::log(Log::ENGINE_ERROR, ("Opengl error (GL_INVALID_OPERATION) for call " + toString(call)).c_str()); break;
				case GL_STACK_OVERFLOW: Log::log(Log::ENGINE_ERROR, ("Opengl error (GL_STACK_OVERFLOW) for call " + toString(call)).c_str()); break;
				case GL_STACK_UNDERFLOW: Log::log(Log::ENGINE_ERROR, ("Opengl error (GL_STACK_UNDERFLOW) for call " + toString(call)).c_str()); break;
				case GL_OUT_OF_MEMORY: Log::log(Log::ENGINE_ERROR, ("Opengl error (GL_OUT_OF_MEMORY) for call " + toString(call)).c_str()); break;
				case GL_TABLE_TOO_LARGE: Log::log(Log::ENGINE_ERROR, ("Opengl error (GL_TABLE_TOO_LARGE) for call " + toString(call)).c_str()); break;
				default: Log::log(Log::ENGINE_ERROR, ("Unknown Opengl error for call " + toString(call)).c_str()); break;
				}
			}
		}
};

class NYRenderer
{
	public :
        SDL_Window * _SDLWindow; ///Fenetre sdl, necessaire pour le swap buffer

		NYCamera * _Camera; ///< Gestion du point de vue
		int _ScreenWidth; ///< Largeur ecran en pixels
		int _ScreenHeight; ///< Hauteur ecran en pixels

		void (*_RenderObjectsFun)(int num_pass); ///< Fonction de rendu des objets uniquement (parfois fait n fois dans certains rendus)
		void (*_RenderObjectsDepthOnlyFun)(); ///< Fonction de rendu d'objets uniquement dans le z buffer (pour certains rendus uniquement)
		void (*_Render2DFun)(void); ///< Rendu en 2d (en passe en mode camera ortho, etc...)
		void (*_SetLights)(int num_pass); ///< Choisit la position des lumieres (besoin de le faire dans le bon referentiel, après matrice view défine)

		//Post Process
		bool _DoPostProcess;
		NYVert3Df _SunShaftSource; ///< Source de lumière pour le sunshaft
		bool _DrawRepere;
		GLuint _ColorTexPP[NB_PASSES_MAX]; ///< Rendu ecran to texture
		GLuint _DataTexPP[NB_PASSES_MAX]; ///< Textures de data (GL_RED et des floats)
		bool _DataPass[NB_PASSES_MAX]; ///< Si c'est une passe data ou pas
		GLuint _ColorWasteTex; ///< Buffer couleur a binder mais inutilisé par la suite (poubelle)
		GLuint _DepthWasteTex; ///< Buffer prof a binder mais inutilisé par la suite (poubelle)
		GLuint _DepthTexPP[NB_PASSES_MAX]; ///< Rendu du buffer de profondeur to texture
		GLuint _FBO; ///< Front buffer Object : pour effectuer un render to texture
		GLuint _ProgramPP; ///< Le programme de shader pour le post process
		int _NbPasses; ///< Combien on fait de passes de rendus
		int _NumPass; ///<Numero de la passe actuelle

		NYColor _BackGroundColor; ///< Couleur de fond. La modifier avec setBackgroundColor()
		NYColor _OverlayColor; ///< Couleur de teinte passée au script de PP

		NYTextEngine * _TextEngine; ///< Rendu de texte

		//HWND _WHnd; ///< Handle de la fenetre principale

		static float _DeltaTime; ///< Temps écoulé depuis la dernière frame (passe a la fonction render)
		static float _DeltaTimeCumul; ///< Temps écoulé depuis le lancement de l'appli

		#define KERNEL_SIZE 64
		float _Kernel[KERNEL_SIZE * 3]; ///32 random vec3 de taille < 1, utilisé par ex pour le ssao

		bool _ClearBuffs;
		bool _IsFullScreen;

		GLuint _VBOCube;

	private :
		static NYRenderer * _Me; ///< Singleton

		NYRenderer()
		{
            _SDLWindow = NULL;
			_Camera = new NYCamera();
			_Camera->setPosition(NYVert3Df(-100,-190,100));
			_Camera->setLookAt(NYVert3Df(0,0,0));
			_ScreenWidth = DEFAULT_SCREEN_WIDTH;
			_ScreenHeight = DEFAULT_SCREEN_HEIGHT;
			_RenderObjectsFun = NULL;
			_RenderObjectsDepthOnlyFun = NULL;
			_Render2DFun = NULL;
			_SetLights = NULL;
			_TextEngine = new NYTextEngine();
			_TextEngine->buildFont("fonts/consolas.ttf",11);
			_TextEngine->buildFont("fonts/consolas.ttf",14);
			_TextEngine->SelectFont(0);
			_BackGroundColor.R = ROUGE_FOND;
			_BackGroundColor.V = VERT_FOND;
			_BackGroundColor.B = BLEU_FOND;
			_DoPostProcess = false;
			_DrawRepere = false;
			_NbPasses = 1;
			_NumPass = 0;
			_ClearBuffs = true;
			_IsFullScreen = false;
			memset(_DataPass,0x00,NB_PASSES_MAX * sizeof(bool));
			_VBOCube = 0;
			_ProgramPP = 0;

			for (int i = 0; i < KERNEL_SIZE; i++)
			{
				NYVert3Df vert;
				vert.X = randf() * 2 - 1.0;
				vert.Y = randf() * 2 - 1.0;
				vert.Z = randf() * 2 - 1.0;
				vert.normalize();
				vert *= randf();
				_Kernel[i * 3 + 0] = vert.X;
				_Kernel[i * 3 + 1] = vert.Y;
				_Kernel[i * 3 + 2] = vert.Z;
			}
				
		}

	public :

		static NYRenderer * getInstance()
		{
			if(_Me == NULL)
				_Me = new NYRenderer();
			return _Me;
		}

		/**
		  * Changement de camera (pour une sous classe par exemple)
		  */
		void setCam(NYCamera * cam)
		{
			_Camera = cam;
		}

		float * addTobuffer(float * ptBuffer, const NYVert3Df & vert, const NYVert3Df & norm, const NYVert3Df & uv)
		{
			*ptBuffer = vert.X; ptBuffer++; //Vert
			*ptBuffer = vert.Y; ptBuffer++; //Vert
			*ptBuffer = vert.Z; ptBuffer++; //Vert

			*ptBuffer = norm.X; ptBuffer++; //Norm
			*ptBuffer = norm.Y; ptBuffer++; //Norm
			*ptBuffer = norm.Z; ptBuffer++; //Norm

			*ptBuffer = uv.X; ptBuffer++; //u
			*ptBuffer = uv.Y; ptBuffer++; //v

			return ptBuffer;
		}

		/**
		* Initialisation du moteur
		*/
		void initialise(SDL_Window * window, bool postProcess = false)
		{
			_SDLWindow = window;
			_DoPostProcess = postProcess;

			if (postProcess)
			{
				this->initFBO();
				this->initShadersPostProcess();
			}

			glClearColor(_BackGroundColor.R, _BackGroundColor.V, _BackGroundColor.B, _BackGroundColor.A);

			glDepthFunc(GL_LEQUAL);
			glEnable(GL_DEPTH_TEST);

			glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
			glEnable(GL_BLEND);

			glEnable(GL_LIGHTING);

			glDisable(GL_COLOR_MATERIAL);
			GLfloat gris[] = { 0.7F, 0.7F, 0.7F, 1.0F };
			glMaterialfv(GL_FRONT, GL_DIFFUSE, gris);

			//On cree le VBO pour un cube
			float * buffer = new float[24 * (3 + 3 + 2)];
			float * ptBuffer = buffer;

			//Face1
			ptBuffer = addTobuffer(ptBuffer, NYVert3Df(-1, -1, -1), NYVert3Df(0, -1, 0), NYVert3Df(0, 1, 0));
			ptBuffer = addTobuffer(ptBuffer, NYVert3Df(1, -1, -1), NYVert3Df(0, -1, 0), NYVert3Df(1, 1, 0));
			ptBuffer = addTobuffer(ptBuffer, NYVert3Df(1, -1, 1), NYVert3Df(0, -1, 0), NYVert3Df(1, 0, 0));
			ptBuffer = addTobuffer(ptBuffer, NYVert3Df(-1, -1, 1), NYVert3Df(0, -1, 0), NYVert3Df(0, 0, 0));

			//Face2
			ptBuffer = addTobuffer(ptBuffer, NYVert3Df(-1, 1, -1), NYVert3Df(0, 1, 0), NYVert3Df(1, 1, 0));
			ptBuffer = addTobuffer(ptBuffer, NYVert3Df(-1, 1, 1), NYVert3Df(0, 1, 0), NYVert3Df(1, 0, 0));
			ptBuffer = addTobuffer(ptBuffer, NYVert3Df(1, 1, 1), NYVert3Df(0, 1, 0), NYVert3Df(0, 0, 0));
			ptBuffer = addTobuffer(ptBuffer, NYVert3Df(1, 1, -1), NYVert3Df(0, 1, 0), NYVert3Df(0, 1, 0));

			//Face3
			ptBuffer = addTobuffer(ptBuffer, NYVert3Df(1, -1, -1), NYVert3Df(1, 0, 0), NYVert3Df(0, 1, 0));
			ptBuffer = addTobuffer(ptBuffer, NYVert3Df(1, 1, -1), NYVert3Df(1, 0, 0), NYVert3Df(1, 1, 0));
			ptBuffer = addTobuffer(ptBuffer, NYVert3Df(1, 1, 1), NYVert3Df(1, 0, 0), NYVert3Df(1, 0, 0));
			ptBuffer = addTobuffer(ptBuffer, NYVert3Df(1, -1, 1), NYVert3Df(1, 0, 0), NYVert3Df(0, 0, 0));

			//Face4
			ptBuffer = addTobuffer(ptBuffer, NYVert3Df(-1, -1, -1), NYVert3Df(-1, 0, 0), NYVert3Df(1, 1, 0));
			ptBuffer = addTobuffer(ptBuffer, NYVert3Df(-1, -1, 1), NYVert3Df(-1, 0, 0), NYVert3Df(1, 0, 0));
			ptBuffer = addTobuffer(ptBuffer, NYVert3Df(-1, 1, 1), NYVert3Df(-1, 0, 0), NYVert3Df(0, 0, 0));
			ptBuffer = addTobuffer(ptBuffer, NYVert3Df(-1, 1, -1), NYVert3Df(-1, 0, 0), NYVert3Df(0, 1, 0));

			//Face5
			ptBuffer = addTobuffer(ptBuffer, NYVert3Df(-1, -1, 1), NYVert3Df(0, 0, 1), NYVert3Df(1, 1, 0));
			ptBuffer = addTobuffer(ptBuffer, NYVert3Df(1, -1, 1), NYVert3Df(0, 0, 1), NYVert3Df(0, 1, 0));
			ptBuffer = addTobuffer(ptBuffer, NYVert3Df(1, 1, 1), NYVert3Df(0, 0, 1), NYVert3Df(0, 0, 0));
			ptBuffer = addTobuffer(ptBuffer, NYVert3Df(-1, 1, 1), NYVert3Df(0, 0, 1), NYVert3Df(1, 0, 0));

			//Face6
			ptBuffer = addTobuffer(ptBuffer, NYVert3Df(-1, -1, -1), NYVert3Df(0, 0, -1), NYVert3Df(1, 1, 0));
			ptBuffer = addTobuffer(ptBuffer, NYVert3Df(-1, 1, -1), NYVert3Df(0, 0, -1), NYVert3Df(1, 0, 0));
			ptBuffer = addTobuffer(ptBuffer, NYVert3Df(1, 1, -1), NYVert3Df(0, 0, -1), NYVert3Df(0, 0, 0));
			ptBuffer = addTobuffer(ptBuffer, NYVert3Df(1, -1, -1), NYVert3Df(0, 0, -1), NYVert3Df(0, 1, 0));
	
			if (_VBOCube != 0)
				glDeleteBuffers(1, &_VBOCube);

			glGenBuffers(1, &_VBOCube);

			glBindBuffer(GL_ARRAY_BUFFER, _VBOCube);

			glBufferData(GL_ARRAY_BUFFER,
				24 * (3 + 3 + 2) * sizeof(float),
				buffer,
				GL_STREAM_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		const char * getCurrentDriverName()
		{
			return SDL_GetCurrentVideoDriver();
		}

		const char * getGPUName()
		{
			return (const char*) glGetString(GL_RENDERER);
		}

		void getScreenFullRes(int * wmax, int * hmax)
		{
			SDL_DisplayMode mode = { SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0 };
			SDL_GetDisplayMode(0, 0, &mode);
			*wmax = mode.w;
			*hmax = mode.h;			
		}
		
		void setFullScreen(bool full,bool fullResolution = true)
		{
            if(full)
            {
                if(fullResolution)
                {
                    int wmax = DEFAULT_SCREEN_WIDTH;
                    int hmax = DEFAULT_SCREEN_HEIGHT;
					getScreenFullRes(&wmax, &hmax);
                    SDL_SetWindowSize(_SDLWindow,wmax,hmax);
                }
            }
            else
            {
                SDL_SetWindowSize(_SDLWindow, DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT);
            }

            _IsFullScreen = full;
            if(full)
                SDL_SetWindowFullscreen(_SDLWindow,SDL_WINDOW_FULLSCREEN);
            else
                SDL_SetWindowFullscreen(_SDLWindow,0);
		}

		void toggleFullScreen(bool fullresolution = true)
		{
            setFullScreen(!_IsFullScreen, fullresolution);
		}

		void setScreenSize(int width, int height)
		{
			SDL_SetWindowSize(_SDLWindow, width, height);
			_ScreenHeight = height;
			_ScreenWidth = width;
		}

		void setPostProcess(bool doit)
		{

			if (doit  && !_DoPostProcess)
			{
				_DoPostProcess = true;
				this->initFBO();
				this->initShadersPostProcess();
			}
			_DoPostProcess = doit;
		}

		void setDrawRepere(bool doit)
		{
			_DrawRepere = doit;
		}

		void setNbPasses(int nbPasses)
		{
			_NbPasses = nbPasses;
			for(int i=0;i<_NbPasses;i++)
				_DataPass[i] = false;

			if(_NbPasses > NB_PASSES_MAX)
			{
				Log::log(Log::ENGINE_ERROR,"Asking for too many passes");
			}
		}

		void setDataPass(int numPass, bool data)
		{
			_DataPass[numPass] = data;
		}

		void resize(int screen_width, int screen_height)
		{
			_ScreenWidth = screen_width;
			_ScreenHeight = screen_height;

			if(_DoPostProcess)
			{
				this->killFBO();
				this->initFBO();
			}
		}

		void setBackgroundColor(NYColor color)
		{
			_BackGroundColor = color;
		}

		void setOverlayColor(NYColor color)
		{
			_OverlayColor = color;
		}

		void setRenderObjectFun(void (*fun)(int num_pass))
		{
			_RenderObjectsFun = fun;
		}

		void setRenderObjectDepthOnlyFun(void (*fun)(void))
		{
			_RenderObjectsDepthOnlyFun = fun;
		}

		void setRender2DFun(void (*fun)(void))
		{
			_Render2DFun = fun;
		}

		void setLightsFun(void (*fun)(int num_pass))
		{
			this->_SetLights = fun;
		}

		void setSunShaftSource(NYVert3Df source)
		{
			_SunShaftSource = source;
		}

		void drawBillBoard(float size)
		{
            NYVert3Df upLeft = (_Camera->_UpVec - _Camera->_NormVec)*size;
            NYVert3Df upRight = (_Camera->_UpVec + _Camera->_NormVec)*size;
            NYVert3Df downLeft = (_Camera->_UpVec*-1 - _Camera->_NormVec)*size;
            NYVert3Df downRight = (_Camera->_UpVec*-1 + _Camera->_NormVec)*size;

            glDisable(GL_CULL_FACE);
            glBegin(GL_QUADS);
            glVertex3f(upLeft.X,upLeft.Y,upLeft.Z);
            glVertex3f(upRight.X,upRight.Y,upRight.Z);
            glVertex3f(downRight.X,downRight.Y,downRight.Z);
            glVertex3f(downLeft.X,downLeft.Y,downLeft.Z);
            glEnd();

            /*glBegin(GL_LINE_LOOP);
            glVertex3f(upLeft.X,upLeft.Y,upLeft.Z);
            glVertex3f(upRight.X,upRight.Y,upRight.Z);
            glVertex3f(downRight.X,downRight.Y,downRight.Z);
            glVertex3f(downLeft.X,downLeft.Y,downLeft.Z);
            glEnd();*/
		}

		void drawSolidSphere(double radius, int stacks, int slices)
		{
            //TODONY : Obligé de l'allouer à chaque fois ?
            GLUquadricObj* quadric = gluNewQuadric();
            gluQuadricDrawStyle(quadric, GLU_FILL);
            gluSphere(quadric, radius, stacks, slices);
            gluDeleteQuadric(quadric);
		}

		void drawSolidCube(float size)
		{

			//On bind le buuffer
			/*glBindBuffer(GL_ARRAY_BUFFER, _VBOCube);
			NYRenderer::checkGlError("glBindBuffer");

			//On active les datas que contiennent le VBO
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_NORMAL_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			
			glPushMatrix();
			glScalef(size, size, size);

			//On place les pointeurs sur les datas, aux bons offsets
			glVertexPointer(3, GL_FLOAT, (3 + 3 + 2)*sizeof(float), (void*)(0));
			glNormalPointer( GL_FLOAT, (3 + 3 + 2)*sizeof(float), (void*)(3 * sizeof(float)));
			glTexCoordPointer(2,GL_FLOAT, (3 + 3 + 2)*sizeof(float), (void*)((3+3) * sizeof(float)));

			//On demande le dessin
			glDrawArrays(GL_QUADS, 0, 24);

			glPopMatrix();

			//On cleane
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);*/

            glBegin(GL_QUADS);

            //Face1
            //glColor3d(0,0.5f,0);
            glNormal3f(0,-1,0);
			glTexCoord2f(0, 1);
            glVertex3f(-size,-size,-size);
			glTexCoord2f(1, 1);
            glVertex3f(size,-size,-size);
			glTexCoord2f(1, 0);
            glVertex3f(size,-size,size);
			glTexCoord2f(0, 0);
            glVertex3f(-size,-size,size);

            //Face2
            //glColor3d(0,0.5f,0);
            glNormal3f(0,1,0);
			glTexCoord2f(1, 1);
            glVertex3f(-size,size,-size);
			glTexCoord2f(1, 0);
            glVertex3f(-size,size,size);
			glTexCoord2f(0, 0);
            glVertex3f(size,size,size);
			glTexCoord2f(0, 1);
            glVertex3f(size,size,-size);

            //Face3
            //glColor3d(0.5f,0,0);
            glNormal3f(1,0,0);
			glTexCoord2f(0, 1);
            glVertex3f(size,-size,-size);
			glTexCoord2f(1, 1);
            glVertex3f(size,size,-size);
			glTexCoord2f(1, 0);
            glVertex3f(size,size,size);
			glTexCoord2f(0, 0);
            glVertex3f(size,-size,size);

            //Face4
            //glColor3d(0.5f,0,0);
            glNormal3f(-1,0,0);
			glTexCoord2f(1, 1);
            glVertex3f(-size,-size,-size);
			glTexCoord2f(1, 0);
            glVertex3f(-size,-size,size);
			glTexCoord2f(0, 0);
            glVertex3f(-size,size,size);
			glTexCoord2f(0, 1);
            glVertex3f(-size,size,-size);

            //Face5
            //glColor3d(0,0,0.5f);

            glNormal3f(0,0,1);
			glTexCoord2f(1, 1);
            glVertex3f(-size,-size,size);
			glTexCoord2f(0, 1);
            glVertex3f(size,-size,size);
			glTexCoord2f(0, 0);
            glVertex3f(size,size,size);
			glTexCoord2f(1, 0);
            glVertex3f(-size,size,size);

            //Face6
            //glColor3d(0,0,0.5f);
            glNormal3f(0,0,-1);
			glTexCoord2f(1, 1);
            glVertex3f(-size,-size,-size);
			glTexCoord2f(1, 0);
            glVertex3f(-size,size,-size);
			glTexCoord2f(0, 0);
            glVertex3f(size,size,-size);
			glTexCoord2f(0, 1);
            glVertex3f(size,-size,-size);

            glEnd();
		}


		void preRender(float elapsed)
		{
			//Le temps
			_DeltaTime = elapsed;
			_DeltaTimeCumul += elapsed;

			//Updates
			_Camera->update(elapsed);
		}

		void setOutFBO(NYFbo * fbo = NULL)
		{
			
			if (fbo)
			{
				//On passe en FBO pour pouvoir faire nos effets
				glBindFramebuffer(GL_FRAMEBUFFER, fbo->_FBO);
				checkGlError("glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_FBO);");

				//Attach 2D texture to this FBO
				for (int i = 0; i < fbo->_NbColorTex; i++)
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, fbo->_ColorTex[i], 0);

				//Attach depth texture to FBO
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fbo->_DepthTex, 0);

			}
			else
			{
				//On passe en mode rendu normal
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}

			//On efface
			glClearColor(_BackGroundColor.R, _BackGroundColor.V, _BackGroundColor.B, _BackGroundColor.A);
			if (_ClearBuffs)
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			//On efface les matrices
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			//On définit la caméra
			this->viewFromCam();

			if (_DrawRepere)
			{
				glDisable(GL_LIGHTING);
				glDisable(GL_TEXTURE_2D);
				glUseProgram(0);
				drawRepere(10.0f);
			}

			//On place les lumieres
			if (this->_SetLights)
				this->_SetLights(_NumPass);
			else
			{
				//On active la light 0
				glEnable(GL_LIGHT0);

				//On définit une lumière directionelle (un soleil)
				float direction[4] = { 0.5, 0.5, 1, 0 }; ///w = 0 donc elle est a l'infini
				glLightfv(GL_LIGHT0, GL_POSITION, direction);
				float color[4] = { 0.5f, 0.5f, 0.5f };
				glLightfv(GL_LIGHT0, GL_DIFFUSE, color);
				float color2[4] = { 0.3f, 0.3f, 0.3f };
				glLightfv(GL_LIGHT0, GL_AMBIENT, color2);
			}

			glColor3f(1.f, 1.f, 1.f);
			glEnable(GL_LIGHTING);
			glEnable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);

		}

		void endRender()
		{
			//Restore (sinon freetype par en sucette)
			glActiveTexture(GL_TEXTURE0);

			//Fin des shaders
			glUseProgram(0);

			//Mode 2D
			//glBindTexture(GL_TEXTURE_2D, 0); //reinit tex
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glDisable(GL_LIGHTING);
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glEnable(GL_COLOR_MATERIAL);

			glMatrixMode(GL_PROJECTION); // passe en mode matrice de projection
			glLoadIdentity(); // Réinitialisation
			gluOrtho2D(0, _ScreenWidth, _ScreenHeight, 0);

			glMatrixMode(GL_MODELVIEW); // on repasse en mode matrice modele
			glLoadIdentity(); // Réinitialisation

			//On fait le rendu 2D sur l'écran
			if (_Render2DFun != NULL)
				(*_Render2DFun)();
			
			//Fini
			SDL_GL_SwapWindow(_SDLWindow);
		}
		
		void render(float elapsed)
		{
			//Le temps
			_DeltaTime = elapsed;
			_DeltaTimeCumul += elapsed;

			//Updates
			_Camera->update(elapsed);

			//Premier effacage de tout
			glClearColor(_BackGroundColor.R, _BackGroundColor.V, _BackGroundColor.B, _BackGroundColor.A);

			if (_ClearBuffs)
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			/*else
				glClear(GL_DEPTH_BUFFER_BIT);*/

			//On fait toutes les passes
			for (_NumPass = 0; _NumPass < _NbPasses; _NumPass++)
			{
				//Si on a active le post process ou qu'on est pas a la dernière passe
				//Mais qu'on est pas a la derniere passe sans post process
				if ((_DoPostProcess || _NumPass < _NbPasses - 1) && !(!_DoPostProcess && _NumPass == _NbPasses - 1))
				{
					//On passe en FBO pour pouvoir faire nos effets
					glBindFramebuffer(GL_FRAMEBUFFER, _FBO);
					checkGlError("glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_FBO);");

					//Attach 2D texture to this FBO
					GLuint buffer = _ColorTexPP[_NumPass];
					if (_DataPass[_NumPass])
						buffer = _DataTexPP[_NumPass];

					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer, 0);
					checkGlError("glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer, 0);");

					//Attach depth texture to FBO
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _DepthTexPP[_NumPass], 0);
					checkGlError("glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _DepthTexPP[_NumPass], 0);");

					//On active le FBO
					glBindFramebuffer(GL_FRAMEBUFFER, _FBO);
					checkGlError("glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_FBO);");
				}
				else
				{
					//On passe en mode rendu normal
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
				}

				//On efface
				glClearColor(_BackGroundColor.R, _BackGroundColor.V, _BackGroundColor.B, _BackGroundColor.A);
				if (_ClearBuffs)
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				/*else
					glClear(GL_DEPTH_BUFFER_BIT);*/

				//On efface les matrices
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();

				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();

				//On définit la caméra
				this->viewFromCam();

				if (_DrawRepere)
				{
					glDisable(GL_LIGHTING);
					glDisable(GL_TEXTURE_2D);
					glUseProgram(0);
					drawRepere(10.0f);
				}

				//On place les lumieres
				if (this->_SetLights)
					this->_SetLights(_NumPass);
				else
				{
					//On active la light 0
					glEnable(GL_LIGHT0);

					//On définit une lumière directionelle (un soleil)
					float direction[4] = { 0.5, 0.5, 1, 0 }; ///w = 0 donc elle est a l'infini
					glLightfv(GL_LIGHT0, GL_POSITION, direction);
					float color[4] = { 0.5f, 0.5f, 0.5f };
					glLightfv(GL_LIGHT0, GL_DIFFUSE, color);
					float color2[4] = { 0.3f, 0.3f, 0.3f };
					glLightfv(GL_LIGHT0, GL_AMBIENT, color2);
				}

				glColor3f(1.f, 1.f, 1.f);
				glEnable(GL_LIGHTING);
				glEnable(GL_BLEND);
				glEnable(GL_DEPTH_TEST);
				glEnable(GL_CULL_FACE);

				//Rendu de la scène
				if (_RenderObjectsFun != NULL)
					(*_RenderObjectsFun)(_NumPass);

			}

			//Dernière passe de post process
			if (_DoPostProcess)
			{
				//On detache le color buffer mais on laisse le depth
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _ColorWasteTex, 0);
				checkGlError("glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _ColorWasteTex, 0);");

				//Rendu des objets de profondeur seulement (fantoooomes) QUE SI POST PROCESS !
				if (_RenderObjectsDepthOnlyFun != NULL)
					(*_RenderObjectsDepthOnlyFun)();

				//On nettoie le FBO (important, sinon depth reste branché)
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _DepthWasteTex, 0);
				checkGlError("glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _DepthWasteTex, 0);");


				//On passe en mode rendu normal
				glBindFramebuffer(GL_FRAMEBUFFER, 0);

				glDisable(GL_CULL_FACE);

				//On met le bon programme de shader pour le post process
				glUseProgram(_ProgramPP);

				//On lui balance les matrices (celles du rendu normal, pas du post process)
				sendOglMatToShader();

				//On rend un quad de la taille de l'ecran
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();

				//on set les variables du shader
				GLuint texLoc = glGetUniformLocation(_ProgramPP, "Texture0");
				glUniform1i(texLoc, 0);

				texLoc = glGetUniformLocation(_ProgramPP, "Texture1");
				glUniform1i(texLoc, 1);

				GLuint var = glGetUniformLocation(_ProgramPP, "screen_width");
				glUniform1f(var, (float)_ScreenWidth);

				var = glGetUniformLocation(_ProgramPP, "screen_height");
				glUniform1f(var, (float)_ScreenHeight);

				var = glGetUniformLocation(_ProgramPP, "colorFond");
				glUniform4f(var, (float)_BackGroundColor.R, (float)_BackGroundColor.V, (float)_BackGroundColor.B, 1);

				var = glGetUniformLocation(_ProgramPP, "colorOverlay");
				glUniform4f(var, (float)_OverlayColor.R, (float)_OverlayColor.V, (float)_OverlayColor.B, 1);

				var = glGetUniformLocation(_ProgramPP, "sunShaftSource");
				glUniform3f(var, (float)_SunShaftSource.X, (float)_SunShaftSource.Y, (float)_SunShaftSource.Z);				

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, _ColorTexPP[_NbPasses - 1]);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, _DepthTexPP[_NbPasses - 1]);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

				//Restore
				glActiveTexture(GL_TEXTURE0);

				renderFullScreenQuad();

				//Fin des shaders
				glUseProgram(0);
			}

			//Rendu 2D (UI et autres)

			//Mode 2D
			glBindTexture(GL_TEXTURE_2D, 0); //reinit tex
			glColor4f(1.0f, 1.0f, 1.0f,1.0f);
			glDisable(GL_LIGHTING);
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glEnable(GL_COLOR_MATERIAL);

			glMatrixMode(GL_PROJECTION); // passe en mode matrice de projection
			glLoadIdentity(); // Réinitialisation
			gluOrtho2D(0, _ScreenWidth, _ScreenHeight, 0);

			glMatrixMode(GL_MODELVIEW); // on repasse en mode matrice modele
			glLoadIdentity(); // Réinitialisation
			
			//On fait le rendu 2D sur l'écran
			if (_Render2DFun != NULL)
				(*_Render2DFun)();


			//Fini
            SDL_GL_SwapWindow(_SDLWindow);
		}

		//GESTION DES SHADERS

		/**
		  * Permet de créer un programme de shaders, a activer quand on veut
		  */
		GLuint createProgram(const char * filePixelShader, const char * fileVertexShader = NULL, const char * fileGeomShader = NULL)
		{
			GLuint fs = 0;
			if(filePixelShader != NULL  && strlen(filePixelShader) > 0)
				fs = loadShader(GL_FRAGMENT_SHADER,filePixelShader);

			GLuint vs = 0;
			if(fileVertexShader != NULL && strlen(fileVertexShader) > 0)
				vs = loadShader(GL_VERTEX_SHADER,fileVertexShader);

			GLuint gs = 0;
			if(fileGeomShader != NULL  && strlen(fileGeomShader) > 0)
				gs = loadShader(GL_GEOMETRY_SHADER,fileGeomShader);

			if(fs > 0 || vs > 0 || gs > 0)
			{
				GLuint prog = glCreateProgram();
				if(fs > 0)
				{
					glAttachShader(prog, fs);
					checkGlError("glAttachShader(prog, fs);");
				}

				if(vs > 0)
				{
					glAttachShader(prog, vs);
					checkGlError("glAttachShader(prog, vs);");
				}

				if(gs > 0)
				{
					glAttachShader(prog, gs);
					checkGlError("glAttachShader(prog, gs);");
				}

				/*glBindAttribLocation(prog, 0, "vs_color_in");
				glBindAttribLocation(prog, 1, "vs_normal_in");
				glBindAttribLocation(prog, 2, "vs_position_in");*/

				glLinkProgram(prog);
				checkGlError("glLinkProgram(prog);");

				
				GLint isLinked = 0;
				glGetProgramiv(prog, GL_LINK_STATUS, &isLinked);
				if (isLinked == GL_FALSE)
				{
					GLint maxLength = 0;
					glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &maxLength);

					//The maxLength includes the NULL character
					char * infoLog = new char[maxLength];
					memset(infoLog, 0, maxLength * sizeof(char));


					glGetProgramInfoLog(prog, maxLength, &maxLength, infoLog);

					//The program is useless now. So delete it.
					glDeleteProgram(prog);

					//Provide the infolog in whatever manner you deem best.
					std::string error = "Unable to link";
					if (filePixelShader != NULL)
						error += " ps[" + toString(filePixelShader) + "]";
					if (fileVertexShader != NULL)
						error += " vs[" + toString(filePixelShader) + "]";
					if (fileGeomShader != NULL)
						error += " gs[" + toString(filePixelShader) + "]";

					error += " because " + toString(infoLog);
					Log::log(Log::ENGINE_ERROR, error.c_str());
					
					SAFEDELETE(infoLog);

					return 0;
				}

				return prog;
			}

			return 0;
		}

		static void checkGlError(const char * call)
		{
			GLenum error = glGetError();

			if(error != 0)
			{
				switch(error)
				{
				case GL_INVALID_ENUM: Log::log(Log::ENGINE_ERROR,("Opengl error (GL_INVALID_ENUM) for call " + toString(call)).c_str()); break;
				case GL_INVALID_OPERATION: Log::log(Log::ENGINE_ERROR,("Opengl error (GL_INVALID_OPERATION) for call " + toString(call)).c_str()); break;
				case GL_STACK_OVERFLOW: Log::log(Log::ENGINE_ERROR,("Opengl error (GL_STACK_OVERFLOW) for call " + toString(call)).c_str()); break;
				case GL_STACK_UNDERFLOW: Log::log(Log::ENGINE_ERROR,("Opengl error (GL_STACK_UNDERFLOW) for call " + toString(call)).c_str()); break;
				case GL_OUT_OF_MEMORY: Log::log(Log::ENGINE_ERROR,("Opengl error (GL_OUT_OF_MEMORY) for call " + toString(call)).c_str()); break;
				case GL_TABLE_TOO_LARGE: Log::log(Log::ENGINE_ERROR,("Opengl error (GL_TABLE_TOO_LARGE) for call " + toString(call)).c_str()); break;
				default : Log::log(Log::ENGINE_ERROR,("Unknown Opengl error for call " + toString(call)).c_str()); break;
				}
			}
		}

		void viewFromCam(void)
		{
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(45.0,((float)_ScreenWidth) / (float)_ScreenHeight, 1, 10000.0);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			_Camera->look();
		}

		//Donner z = 1 pour etre au fond du buffer de profondeur
		void unProjectMousePos(int mouseX, int mouseY, float mouseZ, double * posX, double * posY, double * posZ)
		{
			GLint viewport[4];
			GLdouble modelview[16];
			GLdouble projection[16];
			GLfloat winX, winY, winZ;

			//Mode rendu du monde car sinon elle sont en mode rendu du quad de post process
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			this->viewFromCam();

			glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
			glGetDoublev( GL_PROJECTION_MATRIX, projection );
			glGetIntegerv( GL_VIEWPORT, viewport );

			winX = (float)mouseX;
			winY = (float)viewport[3] - (float)mouseY;
			winZ = mouseZ;

			gluUnProject( winX, winY, winZ, modelview, projection, viewport, posX, posY, posZ);
		}

		void project(double posX, double posY, double posZ, double * mouseX, double * mouseY, double * mouseZ)
		{
			GLint viewport[4];
			GLdouble modelview[16];
			GLdouble projection[16];
			double winX, winY, winZ;

			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();

			//Mode rendu du monde car sinon elle sont en mode rendu du quad de post process
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			this->viewFromCam();

			glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
			glGetDoublev(GL_PROJECTION_MATRIX, projection);
			glGetIntegerv(GL_VIEWPORT, viewport);

			gluProject(posX, posY, posZ, modelview, projection, viewport, &winX, &winY, &winZ);

			*mouseX = winX;
			*mouseY = (float)viewport[3] - (float)winY;
			*mouseZ = winZ;

			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();

			
		}
		        
		void snapshot(const char* filename)
		{
			uint8 * bmpBuffer = (uint8*)malloc(_ScreenWidth*_ScreenHeight*3);
			if (!bmpBuffer)
			{
				Log::log(Log::ENGINE_ERROR,"Malloc failed for snapshot");
				return;
			}

			glReadPixels((GLint)0, (GLint)0,
				(GLint)_ScreenWidth, (GLint)_ScreenHeight,
				GL_RGB, GL_UNSIGNED_BYTE, bmpBuffer);

            NYTexManager::writeImage(filename,_ScreenWidth,_ScreenHeight,bmpBuffer,"udock sshot",true);

			free(bmpBuffer);
		}

		void renderFullScreenQuad()
		{
			//On rend un quad de la taille de l'ecran
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			glDisable(GL_CULL_FACE);
			glDisable(GL_LIGHTING);

			glBegin(GL_QUADS);
			glTexCoord2f( 0, 1); glVertex3f( -1.0f,  1.0f, 0);
			glTexCoord2f( 1, 1); glVertex3f(  1.0f,  1.0f, 0);
			glTexCoord2f( 1, 0); glVertex3f(  1.0f, -1.0f, 0);
			glTexCoord2f( 0, 0); glVertex3f( -1.0f, -1.0f, 0);
			glEnd();
		}

		void sendPassTexsToShader(int num_pass, bool data, const char* colorTexName, int colorTexSlot, const char* depthTexName, int depthTexSlot)
		{
			GLint prog;
			glGetIntegerv(GL_CURRENT_PROGRAM,&prog);

			GLuint texLoc = glGetUniformLocation(prog,colorTexName);
			glUniform1i(texLoc, colorTexSlot);
			checkGlError("glUniform1i(texLoc, colorTexSlot);");

			texLoc = glGetUniformLocation(prog, depthTexName);
			glUniform1i(texLoc, depthTexSlot);
			checkGlError("glUniform1i(texLoc, depthTexSlot);");

			GLuint var = glGetUniformLocation(prog, "screen_width");
			glUniform1f(var, (float)_ScreenWidth);

			var = glGetUniformLocation(prog, "screen_height");
			glUniform1f(var, (float)_ScreenHeight);


			glActiveTexture(GL_TEXTURE0 + colorTexSlot);
			if(data)
				glBindTexture(GL_TEXTURE_2D, _DataTexPP[num_pass]);
			else
				glBindTexture(GL_TEXTURE_2D, _ColorTexPP[num_pass]);

			glActiveTexture(GL_TEXTURE0 + depthTexSlot);
			glBindTexture(GL_TEXTURE_2D, _DepthTexPP[num_pass]);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

			//Restore
			glActiveTexture(GL_TEXTURE0);

		}

		void sendOglMatToShader(void)
		{
			float matMvTab[16];
			glGetFloatv(GL_MODELVIEW_MATRIX, matMvTab);
			NYFloatMatrix matMv;
			memcpy(matMv.Mat.t,matMvTab,16*sizeof(float));
			matMv.transpose();

			float matProjTab[16];
			glGetFloatv(GL_PROJECTION_MATRIX, matProjTab);
			NYFloatMatrix matProj;
			memcpy(matProj.Mat.t,matProjTab,16*sizeof(float));
			matProj.transpose();

			NYFloatMatrix matMVP;
			matMVP = matProj;
			matMVP *= matMv;

			NYFloatMatrix matNorm;
			matNorm = matMv;
			matNorm.invert();
			matNorm.transpose();

			NYFloatMatrix viewMat;
			viewMat.createViewMatrix(_Camera->_Position,_Camera->_LookAt,_Camera->_UpVec);

			NYFloatMatrix invertViewMat;
			invertViewMat = viewMat;
			invertViewMat.invert();

			NYFloatMatrix model;
			model = invertViewMat;
			model *= matMv;

			NYFloatMatrix invertProjMat;
			invertProjMat = matProj;
			invertProjMat.invert();

			GLint prog;
			glGetIntegerv(GL_CURRENT_PROGRAM,&prog);

			GLuint mvp = glGetUniformLocation(prog, "mvp");
			glUniformMatrix4fv(mvp,1,true,matMVP.Mat.t);

			GLuint mv = glGetUniformLocation(prog, "mv");
			glUniformMatrix4fv(mv,1,false,matMvTab);

			GLuint m = glGetUniformLocation(prog, "m");
			glUniformMatrix4fv(m,1,true,model.Mat.t);

			GLuint v = glGetUniformLocation(prog, "v");
			glUniformMatrix4fv(v,1,true,viewMat.Mat.t);

			GLuint p = glGetUniformLocation(prog, "p");
			glUniformMatrix4fv(p,1,true,matProj.Mat.t);

			GLuint iv = glGetUniformLocation(prog, "iv");
			glUniformMatrix4fv(iv,1,true,invertViewMat.Mat.t);

			GLuint ip = glGetUniformLocation(prog, "ip");
			glUniformMatrix4fv(ip, 1, true, invertProjMat.Mat.t);

			GLuint nmat = glGetUniformLocation(prog, "nmat");
			glUniformMatrix4fv(nmat,1,true,matNorm.Mat.t);

			GLuint elaps = glGetUniformLocation(prog, "elapsed");
			glUniform1f(elaps, _DeltaTimeCumul);

			GLuint elapsFrm = glGetUniformLocation(prog, "elapsed_frame");
			glUniform1f(elapsFrm, _DeltaTime);

			GLuint kern = glGetUniformLocation(prog, "kernel");
			glUniform3fv(kern, KERNEL_SIZE, _Kernel);

			GLuint var = glGetUniformLocation(prog, "screen_width");
			glUniform1f(var, (float)_ScreenWidth);

			var = glGetUniformLocation(prog, "screen_height");
			glUniform1f(var, (float)_ScreenHeight);
			
		}

		void reloadInternShader()
		{
			initShadersPostProcess();
		}


	private:

		void initShadersPostProcess(void)
		{
			if (_ProgramPP != 0)
				glDeleteProgram(_ProgramPP);
			_ProgramPP = glCreateProgram();
			GLuint shader = loadShader(GL_FRAGMENT_SHADER,"shaders/postprocess/pshader.glsl");
			glAttachShader(_ProgramPP, shader);
			checkGlError("glAttachShader(_ProgramPP, shader);");
			glLinkProgram(_ProgramPP);
			checkGlError("glLinkProgram(_ProgramPP);");
		}

		void killFBO(void)
		{
			glDeleteTextures(NB_PASSES_MAX,_ColorTexPP);
			glDeleteTextures(NB_PASSES_MAX,_DataTexPP);
			glDeleteTextures(1,&_ColorWasteTex);
			glDeleteTextures(1,&_DepthWasteTex);
			glDeleteTextures(NB_PASSES_MAX,_DepthTexPP);
			glDeleteFramebuffers(1,&_FBO);
		}

		//Get Depth et Frame buffer to textures
		int initFBO(void)
		{
			Log::log(Log::ENGINE_INFO,"Reset FBO");
			//RGBA8 2D texture, 24 bit depth texture, 256x256
			glGenTextures(NB_PASSES_MAX, _ColorTexPP);
			for(int i=0;i<NB_PASSES_MAX;i++)
			{
				glBindTexture(GL_TEXTURE_2D, _ColorTexPP[i]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				//NULL means reserve texture memory, but texels are undefined
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _ScreenWidth, _ScreenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			}

			glGenTextures(NB_PASSES_MAX, _DataTexPP);
			for(int i=0;i<NB_PASSES_MAX;i++)
			{
				glBindTexture(GL_TEXTURE_2D, _DataTexPP[i]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				//NULL means reserve texture memory, but texels are undefined
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _ScreenWidth, _ScreenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			}

			glGenTextures(1, &_ColorWasteTex);
			glBindTexture(GL_TEXTURE_2D, _ColorWasteTex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			//NULL means reserve texture memory, but texels are undefined
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _ScreenWidth, _ScreenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

			glGenTextures(NB_PASSES_MAX, _DepthTexPP);
			for(int i=0;i<NB_PASSES_MAX;i++)
			{
				glBindTexture(GL_TEXTURE_2D, _DepthTexPP[i]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
				//NULL means reserve texture memory, but texels are undefined
				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _ScreenWidth, _ScreenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			}

			glGenTextures(1, &_DepthWasteTex);
			glBindTexture(GL_TEXTURE_2D, _DepthWasteTex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			//NULL means reserve texture memory, but texels are undefined
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _ScreenWidth, _ScreenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);



			//-------------------------
			glGenFramebuffers(1, &_FBO);
			//On bind le FBO pour tester si tout est ok
			glBindFramebufferEXT(GL_FRAMEBUFFER, _FBO);

			for(int i=0;i<NB_PASSES_MAX;i++)
			{
				//Attach 2D texture to this FBO
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _ColorTexPP[i], 0);
				//-------------------------
				//Attach depth texture to FBO
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _DepthTexPP[i], 0);
				//-------------------------
				//Does the GPU support current FBO configuration?
				GLenum status;
				status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
				switch(status)
				{
					case GL_FRAMEBUFFER_COMPLETE:
						//_cprintf("GPU ok for VBO with depth and color\n");
						break;
					default:
						Log::log(Log::ENGINE_ERROR,"GPU does not support VBO");
						Log::log(Log::USER_ERROR,"You graphic card is not able to run this software (no VBO support)");
						return -1;
				}
			}

			//On debind
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			return 0;
		}

		//X est en bleu
		//Y est en rouge
		void drawRepere(float slotSize)
		{
			static int sizeRepere = 20;
			glPushMatrix();
			glBegin(GL_LINES);

			glColor3f(1.0f,0.0f,0.0f);
			glVertex3f(0.0f,0.0f,0.0f);
			glVertex3f(1000.0f,0.0f,0.0f);
			glColor3f(0.0f,1.0f,0.0f);
			glVertex3f(0.0f,0.0f,0.0f);
			glVertex3f(0.0f,1000.0f,0.0f);
			glColor3f(0.0f,0.0f,1.0f);
			glVertex3f(0.0f,0.0f,0.0f);
			glVertex3f(0.0f,0.0f,1000.0f);


			for(int i = -sizeRepere; i<=sizeRepere;i++)
			{
				glColor3f(0.3f,0.3f,0.3f);

				if(i == 0)
					glColor3f(1.0f,1.0f,1.0f);

				glVertex3f(i*slotSize,-sizeRepere*slotSize,0);

				if(i == 0)
				{
					glColor3f(1.0f,1.0f,1.0f);
					glVertex3f(0.0f,0.0f,0.0f);
					glColor3f(0.0f,1.0f,0.0f);
					glVertex3f(0.0f,0.0f,0.0f);
				}

				glVertex3f(i*slotSize,sizeRepere*slotSize,0);

				if(i == 0)
					glColor3f(1.0,1.0,1.0);

				glVertex3f(-sizeRepere*slotSize,i*slotSize,0);

				if(i == 0)
				{
					glColor3f(1.0,1.0,1.0);
					glVertex3f(0,0,0);
					glColor3f(1.0,0.0,0.0);
					glVertex3f(0,0,0);
				}

				glVertex3f(sizeRepere*slotSize,i*slotSize,0);
			}

			glEnd();
			glPopMatrix();
		}

		char* loadSource(const char *filename)
		{
			char *src = NULL;   /* code source de notre shader */
			FILE *fp = NULL;    /* fichier */
			long size;          /* taille du fichier */
			long i;             /* compteur */


			/* on ouvre le fichier */
			fp = fopen(filename, "r");
			/* on verifie si l'ouverture a echoue */
			if(fp == NULL)
			{
				Log::log(Log::ENGINE_ERROR,(std::string("Unable to load shader file ")+ std::string(filename)).c_str());
				return NULL;
			}

			/* on recupere la longueur du fichier */
			fseek(fp, 0, SEEK_END);
			size = ftell(fp);

			/* on se replace au debut du fichier */
			rewind(fp);

			/* on alloue de la memoire pour y placer notre code source */
			src = (char*)malloc(size+1); /* +1 pour le caractere de fin de chaine '\0' */
			if(src == NULL)
			{
				fclose(fp);
				Log::log(Log::ENGINE_ERROR,"Unable to allocate memory for shader file before compilation");
				return NULL;
			}

			/* lecture du fichier */
			for(i=0; i<size && !feof(fp); i++)
				src[i] = fgetc(fp);

			/* on place le dernier caractere a '\0' */
			src[i] = '\0';

			//sous windows, EOF a virer
			if (src[i - 1] == EOF)
				src[i - 1] = '\0';

			fclose(fp);

			return src;
		}

		GLuint loadShader(GLenum type, const char *filename)
		{
			GLuint shader = 0;
			GLsizei logsize = 0;
			GLint compile_status = GL_TRUE;
			char *log = NULL;
			char *src = NULL;

			// creation d'un shader de sommet
			shader = glCreateShader(type);
			if(shader == 0)
			{
				Log::log(Log::ENGINE_ERROR,"Unable to create shader");
				return 0;
			}

			// chargement du code source
			src = loadSource(filename);
			if(src == NULL)
			{
				// theoriquement, la fonction LoadSource a deja affiche un message
				// d'erreur, nous nous contenterons de supprimer notre shader
				// et de retourner 0

				glDeleteShader(shader);
				return 0;
			}

			// assignation du code source
			glShaderSource(shader, 1, (const GLchar**)&src, NULL);

			// compilation du shader
			glCompileShader(shader);

			// liberation de la memoire du code source
			free(src);
			src = NULL;

			//verification du succes de la compilation
			glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logsize);

			log = (char*)malloc(logsize + 1);
			if (log == NULL)
			{
				Log::log(Log::ENGINE_ERROR, "Unable to allocate memory for shader compilation log");
				return 0;
			}
			memset(log, '\0', logsize + 1);

			glGetShaderInfoLog(shader, logsize, &logsize, log);

			if(compile_status != GL_TRUE)
			{
				
				Log::log(Log::ENGINE_ERROR,("Unable to compile shader " + toString(filename) + " : " + toString(log)).c_str());
				free(log);
				glDeleteShader(shader);

				return 0;
			}
			else
			{
				Log::log(Log::ENGINE_INFO,("Shader " + toString(filename) + " compilation ok").c_str());
				Log::log(Log::ENGINE_INFO, ("Compile res " + toString(filename) + " : " + toString(log)).c_str());
				free(log);
			}

			return shader;
		}





};

#endif
