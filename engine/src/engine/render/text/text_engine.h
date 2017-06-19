#ifndef __NY_TEXT_ENGINE__
#define __NY_TEXT_ENGINE__

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include <string>
#include <algorithm>
#include "engine/utils/types.h"
#include "engine/log/log.h"

#include "engine/render/text/FreeType.h"

class NYTextEngine
{
	private:


		//Private GDI
/*		HDC	_HDC;

		//Fonts
		uint32 _FontList[NB_FONTS];
		TEXTMETRIC _Metrics[NB_FONTS];
		HFONT _Fonts[NB_FONTS];*/

		typedef freetype::font_data NY_FONT;
		NY_FONT * _SelectedFont;
		std::vector<NY_FONT * > _Fonts;

		int _TextPosX;
		int _TextPosY;

	public:
		NYTextEngine ()
		{
			_SelectedFont = NULL;
			_TextPosX = 0;
			_TextPosY = 0;
		}

		NY_FONT * buildFont(const char * name, int size)							// Build Our Bitmap Font
		{
			FILE * fe = fopen(name, "r");
			if (!fe)
			{
				Log::log(Log::ENGINE_ERROR, ("Le fichier de font n'existe pas !!! " + toString(name)).c_str());
				return NULL;
			}
			fclose(fe);
				
            NY_FONT  * font = new freetype::font_data();
            font->init(name, size);
            _Fonts.push_back(font);
            return font;
		}

		GLvoid KillFont(NY_FONT * font)									// Delete The Font List
		{
			std::vector<NY_FONT*>::iterator it = std::find(_Fonts.begin(), _Fonts.end(), font);
			if(it != _Fonts.end())
                _Fonts.erase(it);

		}

		void SelectFont(NY_FONT * font)
		{
			std::vector<NY_FONT*>::iterator it = std::find(_Fonts.begin(), _Fonts.end(), font);
			if(it != _Fonts.end())
                _SelectedFont = font;
		}

		void SelectFont(int num)
		{
			if(num < _Fonts.size())
                _SelectedFont = _Fonts[num];
		}

        GLvoid glSetTextPos(int x, int y)
        {
            _TextPosX = x;
            _TextPosY = y;
        }

		GLvoid glPrint(int lenAff,const char *fmt, ...)					// Custom GL "Print" Routine
		{
			char text[1000];								// Holds Our String
			va_list		ap;										// Pointer To List Of Arguments

			if (fmt == NULL)									// If There's No Text
				return;											// Do Nothing

			va_start(ap, fmt);									// Parses The String For Variables
			    vsprintf(text, fmt, ap);						// And Converts Symbols To Actual Numbers
			va_end(ap);											// Results Are Stored In Text

            if(lenAff < 0 || lenAff > strlen(text))
                lenAff = strlen(text);

            uint16 up = 0;
            uint16 down = 0;
            fontHeight(up,down);

            glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT  | GL_ENABLE_BIT | GL_TRANSFORM_BIT);
            glDisable(GL_LIGHTING);
            glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glListBase(_SelectedFont->list_base);
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            glTranslatef(_TextPosX,_TextPosY,0);
            glCallLists(lenAff, GL_UNSIGNED_BYTE, text);
			glBindTexture(GL_TEXTURE_2D, 0);
            glPopMatrix();
            glPopAttrib();
		}

		uint16 stringSize(std::string & chaine)
		{
            uint16 size_chaine = 0;
            for(int i=0;i<chaine.size();i++)
                size_chaine += _SelectedFont->carac_sizes[chaine[i]].width;
			return size_chaine;
		}

		uint16 fontHeight(uint16 & up, uint16 & down)
		{
            int base = _SelectedFont->carac_sizes['a'].height;
            int car_dessous = _SelectedFont->carac_sizes['p'].height;
            int car_dessus = _SelectedFont->carac_sizes['h'].height;
            down = car_dessous - base;
            up = car_dessus;
            return up + down;
		}

		uint16 fontWidth(void)
		{
			return (uint16)_SelectedFont->carac_sizes['A'].width;
		}

};

#endif
