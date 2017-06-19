#ifndef __TEX_MANAGER_H__
#define __TEX_MANAGER_H__

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include <vector>
#include <string>

//#include "math.h"
#include "engine/utils/types.h"
#include "engine/utils/ny_utils.h"
#include "engine/log/log.h"
#include "libpng16/png.h"

using namespace std;

//fichier de texture
class NYTexFile
{
	public:
		string File;    ///< Fichier ou trouver la texture
		bool Loaded;    ///< Si on a charge ce fchier image
		uint16 Texture; ///< Le handle de cette texture pour OGL
		uint32 LastUse; ///< Pour savoir laquelle effacer, heure de derniere utilisation
		uint8 Alpha;    ///< Alpha de la texture
		uint16 SizeX;
		uint16 SizeY;
		bool Visible;

		 NYTexFile & operator = (const NYTexFile & tex)
		{
			File = tex.File;
			Loaded = tex.Loaded;
			Texture = tex.Texture;
			LastUse = tex.LastUse;
			Alpha = tex.Alpha;
			SizeX = tex.SizeX;
			SizeY = tex.SizeY;
			Visible = tex.Visible;
			return *this;
		}

		 NYTexFile()
		 {
			 Loaded = false;
			 Texture = 0;
			 LastUse = 0;
			 Alpha = 0;
			 SizeX = 0;
			 SizeY = 0;
			 File = "";
			 Visible = true;
		 }
};

class NYTexManager
{
	private:
		vector<NYTexFile*> _Textures;
		float _ColorPaint[3];

		static NYTexManager * _Instance;
		NYTexManager()
		{
			_ColorPaint[0] = 1;
			_ColorPaint[1] = 1;
			_ColorPaint[2] = 1;
		}

	public:

		static NYTexManager * getInstance(void)
		{
			if(_Instance == NULL)
				_Instance = new NYTexManager();
			return _Instance;
		}

		NYTexFile * loadTexture(const string & file)
		{

			NYTexFile * texFile = new NYTexFile();
			texFile->File = file;
			_Textures.push_back(texFile);
			uint16 pos = _Textures.size() - 1;
			loadTexFile(*texFile);
			return texFile;
		}

		sint8 unloadTexFile(NYTexFile & texfile)
		{
			glDeleteTextures(1,(GLuint*)&(texfile.Texture));
			return 0;
		}

		void setColorPaint(float R, float V, float B)
		{
			_ColorPaint[0] = R;
			_ColorPaint[1] = V;
			_ColorPaint[2] = B;
		}

		void drawTex2D(float posX, float posY, uint16 nbX, uint16 nbY, uint16 numX, uint16 numY, NYTexFile & tex)
		{
			glColor3fv(_ColorPaint);
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_BLEND);
			//glDisable(GL_DEPTH_TEST);

			glPushMatrix();
			glTranslatef(posX,posY,0);

			int sizeX = (tex.SizeX / nbX);
			int sizeY = (tex.SizeY / nbY);

			float x = 1.0f/((float)nbX);
			float y = 1.0f/((float)nbY);

			float xd = numX * x;
			float xf = xd + x;
			float yd = numY * y;
			float yf = yd + y;

			GLfloat white[] = { 1.0F,1.0F,1.0F,1.0F };
			glMaterialfv(GL_FRONT,GL_DIFFUSE,white);
			glDisable(GL_COLOR_MATERIAL);
			glDisable(GL_LIGHTING);

			glBindTexture(GL_TEXTURE_2D, tex.Texture);
			glBegin(GL_QUADS);
				glTexCoord2f( xd, yf); glVertex3f( 0.0f,  (float)sizeY, 0);
				glTexCoord2f(xf, yf);  glVertex3f(  (float)sizeX,  (float)sizeY, 0);
				glTexCoord2f( xf, yd); glVertex3f(  (float)sizeX, 0.0f, 0);
				glTexCoord2f( xd, yd); glVertex3f( 0.0f, 0.0f, 0);
			glEnd();
			glPopMatrix();

			//glDisable(GL_BLEND);
			//glEnable(GL_DEPTH_TEST);
			glDisable(GL_TEXTURE_2D);
		}

		//Pour sauver une image (la donner en RGB)
		static void writeImage(const char* filename, int width, int height, float *data, const char* title, bool inverty = false)
		{
			png_image image;

			/* Only the image structure version number needs to be set. */
			memset(&image, 0, sizeof image);
			image.version = PNG_IMAGE_VERSION;
			image.format = PNG_FORMAT_RGBA;
			image.width = width;
			image.height = height;

			png_byte * buffer;
			buffer = new png_byte[4*width*height];

			if (!buffer)
				return;


			int i = 0;
			for(int x=0;x<width;x++)
			{
                if(!inverty)
                {
                    for(int y=0;y<height;y++)
                    {
                        buffer[(y+(x*height))*4+0] = data[i++]*255;
                        buffer[(y+(x*height))*4+1] = data[i++]*255;
                        buffer[(y+(x*height))*4+2] = data[i++]*255;
                        buffer[(y+(x*height))*4+3] = 255;
                    }
                }
                else
                {
                    for(int y=height-1;y>=0;y--)
                    {
                        buffer[(y+(x*height))*4+0] = data[i++]*255;
                        buffer[(y+(x*height))*4+1] = data[i++]*255;
                        buffer[(y+(x*height))*4+2] = data[i++]*255;
                        buffer[(y+(x*height))*4+3] = 255;
                    }
                }
            }

			int ret = png_image_write_to_file(&image, filename,
                  0/*convert_to_8bit*/, buffer, 0/*row_stride*/,
                  NULL/*colormap*/);

			SAFEDELETE_TAB(buffer);
		}

		static void writeImage(const char* filename, int width, int height, uint8 *data, const char* title, bool inverty = false)
		{
			png_image image;

			/* Only the image structure version number needs to be set. */
			memset(&image, 0, sizeof image);
			image.version = PNG_IMAGE_VERSION;
			image.format = PNG_FORMAT_RGBA;
			image.width = width;
			image.height = height;

			png_byte * buffer;
			buffer = new png_byte[4*width*height];

			if (!buffer)
				return;

			/*int i = 0;
			for(int x=0;x<width;x++)
			{
                if(inverty)
                {
                    for(int y=0;y<height;y++)
                    {
                        buffer[(y+(x*height))*4+0] = data[i++]*255;
                        buffer[(y+(x*height))*4+1] = data[i++]*255;
                        buffer[(y+(x*height))*4+2] = data[i++]*255;
                        buffer[(y+(x*height))*4+3] = 255;
                    }
                }
                else
                {
                    for(int y=height-1;y>=0;y--)
                    {
                        buffer[(y+(x*height))*4+0] = data[i++]*255;
                        buffer[(y+(x*height))*4+1] = data[i++]*255;
                        buffer[(y+(x*height))*4+2] = data[i++]*255;
                        buffer[(y+(x*height))*4+3] = 255;
                    }
                }
            }*/

            int i = 0;
            if(!inverty)
            {
                for(int y=0;y<height;y++)
                {
                    for(int x=0;x<width;x++)
                    {
                            buffer[((y*width)+x)*4 + 0] = data[i++];
                            buffer[((y*width)+x)*4 + 1] = data[i++];
                            buffer[((y*width)+x)*4 + 2] = data[i++];
                            buffer[((y*width)+x)*4 + 3] = 255;
                    }
                }
            }
            else
            {
                for(int y=height-1;y>=0;y--)
                {
                    for(int x=0;x<width;x++)
                    {
                            buffer[((y*width)+x)*4 + 0] = data[i++];
                            buffer[((y*width)+x)*4 + 1] = data[i++];
                            buffer[((y*width)+x)*4 + 2] = data[i++];
                            buffer[((y*width)+x)*4 + 3] = 255;
                    }
                }
            }



			int ret = png_image_write_to_file(&image, filename,
                  0/*convert_to_8bit*/, buffer, 0/*row_stride*/,
                  NULL/*colormap*/);

			SAFEDELETE_TAB(buffer);
		}

		sint8 loadImageFile_PNG(string & file, png_image * image, png_bytep * buffer)
		{
			if (!file.length())
				return 1;

			/* Open image file */
			FILE * fp = fopen(file.c_str(), "rb");
			if (!fp)
			{
				Log::log(Log::ENGINE_ERROR, ("Could not load texture file " + file).c_str());
				return 1;
			}
			fclose(fp);

			/* Initialize the 'png_image' structure. */
			memset(image, 0, (sizeof *image));
			image->version = PNG_IMAGE_VERSION;

			/* The first argument is the file to read: */
			if (png_image_begin_read_from_file(image, file.c_str()))
			{
				/* Set the format in which to read the PNG file; this code chooses a
				* simple sRGB format with a non-associated alpha channel, adequate to
				* store most images.
				*/
				image->format = PNG_FORMAT_RGBA;

				/* Now allocate enough memory to hold the image in this format; the
				* PNG_IMAGE_SIZE macro uses the information about the image (width,
				* height and format) stored in 'image'.
				*/
				*buffer = (png_byte*)malloc(PNG_IMAGE_SIZE(*image));

				/* If enough memory was available read the image in the desired format
				* then write the result out to the new file.  'background' is not
				* necessary when reading the image because the alpha channel is
				* preserved; if it were to be removed, for example if we requested
				* PNG_FORMAT_RGB, then either a solid background color would have to
				* be supplied or the output buffer would have to be initialized to the
				* actual background of the image.
				*
				* The fourth argument to png_image_finish_read is the 'row_stride' -
				* this is the number of components allocated for the image in each
				* row.  It has to be at least as big as the value returned by
				* PNG_IMAGE_ROW_STRIDE, but if you just allocate space for the
				* default, minimum, size using PNG_IMAGE_SIZE as above you can pass
				* zero.
				*
				* The final argument is a pointer to a buffer for the colormap;
				* colormaps have exactly the same format as a row of image pixels (so
				* you choose what format to make the colormap by setting
				* image.format).  A colormap is only returned if
				* PNG_FORMAT_FLAG_COLORMAP is also set in image.format, so in this
				* case NULL is passed as the final argument.  If you do want to force
				* all images into an index/color-mapped format then you can use:
				*
				*    PNG_IMAGE_COLORMAP_SIZE(image)
				*
				* to find the maximum size of the colormap in bytes.
				*/
				if (*buffer != NULL && png_image_finish_read(image, NULL/*background*/, *buffer,
					0/*row_stride*/, NULL/*colormap*/))
				{
					return 0;
				}
				else
				{
					if (*buffer != NULL)
						free(*buffer);
				}
			}
			return 1;
		}


	private:

		

		sint8 loadTexFile(NYTexFile & texFile)
		{
			png_image image;
			png_bytep buffer;
			uint32 i;
			memset(&image,0x00,sizeof(png_image));

			if(loadImageFile_PNG(texFile.File,&image,&buffer))
			{
				return 1;
			}

			texFile.SizeX = image.width;
			texFile.SizeY = image.height;

			uint32 nbPixels = texFile.SizeX*texFile.SizeY;

			uint8 * pixelsRgbAlpha = new uint8[nbPixels*4];

			//Ajoute la composante alpha
			for(i=0;i<nbPixels;i++)
			{
				pixelsRgbAlpha[(4*i)+0] = buffer[(4*i)+0];
				pixelsRgbAlpha[(4*i)+1] = buffer[(4*i)+1];
				pixelsRgbAlpha[(4*i)+2] = buffer[(4*i)+2];
				pixelsRgbAlpha[(4*i)+3] = buffer[(4*i)+3];
 			}

			//Cree une texture
			texFile.Texture = 0;
			glGenTextures(1, (GLuint*)&(texFile.Texture));


			//Create texture
			glBindTexture(GL_TEXTURE_2D, texFile.Texture);
			glTexImage2D(GL_TEXTURE_2D,
										0, //Niveau de detail
										GL_RGBA, //4 couleurs
										texFile.SizeX, //Largeur
										texFile.SizeY, //Hauteur
										0,//bord de 0 pixels
										GL_RGBA,
										GL_UNSIGNED_BYTE,
										pixelsRgbAlpha);

			//Set filters
			/*glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	// Linear Filtering
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	// Linear Filtering*/

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

			glBindTexture(GL_TEXTURE_2D,0);

			if(!isPowerOfTwo(texFile.SizeX))
				return 3;
			if(!isPowerOfTwo(texFile.SizeY))
				return 4;

			//Texture chargee
			texFile.Loaded = true;

			//Libere memeoire image
			SAFEDELETE(buffer);
			SAFEDELETE(pixelsRgbAlpha);
			return 0;
		}

};

#endif
