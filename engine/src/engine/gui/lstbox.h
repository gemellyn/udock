#pragma once

#include "engine/gui/panel.h"

#include <vector>

class GUILstBox : public GUIPanel
{
	public:
		std::vector<std::string> Texts;
		NYColor ColorSel;
		uint32 LastClicked;
		uint32 LastChanged;
		bool AlphaEffect;
		bool ShowScrollBar;
		bool CenterSelected;


	private:
		sint16 _IdSelection;
		uint16 _ElementHeight;
		sint16 _OffsetAffichage;
		GUIPanel * _Ascenceur;
		uint16 _NbMaxElements;
		int _FontNum;


	public:
		GUILstBox() : GUIPanel()
		{
			setFontNum(0);

			_Ascenceur = new GUIPanel();
			_Ascenceur->Visible = false;
			_Ascenceur->FondPlein = true;

			setMaxElements(5);
			this->X = 10;
			this->Y = 10;
			this->Width = 100;
			ColorSel.R = 0.0;
			ColorSel.V = 1.0;
			ColorSel.B = 0.0;
			_IdSelection = -1;
			_OffsetAffichage = 0;
			LastClicked = 0;
			LastChanged = 0;
			ShowScrollBar = true;
			CenterSelected = false;

			AlphaEffect = false;

		}

		void setFontNum(int num)
		{
			_FontNum = num;
			uint16 up,down;
			_TextEngine->SelectFont(_FontNum);
			_TextEngine->fontHeight(up,down);
			_ElementHeight = (up+down+2);
		}

		uint16 getElementHeight(void)
		{
			return _ElementHeight;
		}

		uint16 getNbElements(void)
		{
			return Texts.size();
		}

		uint16 getNbMaxElements(void)
		{
			return _NbMaxElements;
		}

		sint16 getSelIndex(void)
		{
			return _IdSelection;
		}

		void setSelIndex(sint16 index)
		{
			if (index < (sint16)Texts.size())
				_IdSelection = index;
			if(CenterSelected)
				centerSelected();
		}

		void setZOrder(uint16 zorder)
		{
			ZOrder = zorder;
			_Ascenceur->ZOrder = zorder;
		}

		sint8 setMaxElements(uint16 nbMaxElements)
		{
			uint16 up,down;
			_TextEngine->SelectFont(_FontNum);
			_TextEngine->fontHeight(up,down);
			_NbMaxElements = nbMaxElements;
			Height = _NbMaxElements * _ElementHeight + down+2;
			return 0;
		}

		//détermine le nombre d'éléments max a partir d'une hauteur
		sint8 setMaxElementsFromHeight(sint32 height)
		{
			uint16 up,down;
			_TextEngine->SelectFont(_FontNum);
			_TextEngine->fontHeight(up,down);
			_NbMaxElements = (height - down - 2) / _ElementHeight;
			Height = _NbMaxElements * _ElementHeight + down+2;
			return 0;
		}

		sint8 addElement(const std::string & element)
		{
			Texts.push_back(element);
			if(Texts.size() > _NbMaxElements && ShowScrollBar)
				showAscenceur(true);
			return 0;
		}

		sint8 insertElement(const std::string & element, int pos)
		{
			Texts.insert(Texts.begin()+pos,element);
			if (Texts.size() > _NbMaxElements && ShowScrollBar)
				showAscenceur(true);
			return 0;
		}

		void clear(void)
		{
			Texts.clear();
			showAscenceur(false);
		}

		void render(uint16 zorder)
		{
			if(Visible && zorder == ZOrder)
			{
				GUIPanel::render(zorder);
				_Ascenceur->render(zorder);

				uint16 up,down;
				_TextEngine->SelectFont(_FontNum);
				_TextEngine->fontHeight(up,down);
				uint16 lenAff = (Width-5) / _TextEngine->fontWidth();

				glColor3f(ColorBorder.R,ColorBorder.V,ColorBorder.B);

				//Draw Strings
				for(unsigned int i=0; i < _NbMaxElements;i++)
				{
					float alpha = 1.0f;

					if(AlphaEffect)
						alpha = 1.0f-(float)abs((int)(i-_NbMaxElements/2.0f))/(( _NbMaxElements+1)/2.0f);

					if (i+_OffsetAffichage == _IdSelection)
						glColor4f(ColorSel.R,ColorSel.V,ColorSel.B,alpha);
					else
						glColor4f(ColorBorder.R,ColorBorder.V,ColorBorder.B,alpha);

					if(_OffsetAffichage + i >= 0 && _OffsetAffichage + i < Texts.size())
					{
						_TextEngine->glSetTextPos(X+5,Y + (_ElementHeight * (i+1)));
						_TextEngine->glPrint(lenAff-1,Texts[i+_OffsetAffichage].c_str());
					}

					if (i+_OffsetAffichage == _IdSelection)
						glColor3f(ColorBorder.R,ColorBorder.V,ColorBorder.B);
				}
			}
		}

		sint8 mouseCallback(int x, int y, uint16 click, sint16 wheel, uint16 zorder, bool focusAvailable, uint32 elapsed)
		{

			sint8 mouseForMe = 0;
			if(Visible && zorder == ZOrder)
			{
				GUIPanel::mouseCallback(x,y,click,wheel,zorder,focusAvailable,elapsed);

				if(MouseOn && focusAvailable)
				{
					mouseForMe = 1;

					if(click & SDL_BUTTON_LEFT)
					{
						uint16 up,down;
						_TextEngine->SelectFont(_FontNum);
						_TextEngine->fontHeight(up,down);

						_HasFocus = true;
						//_IdSelection = (((y-Y) - (_ElementHeight/2)+down) / _ElementHeight) + _OffsetAffichage;
						_IdSelection = (sint16) ( ((y-Y) / _ElementHeight) + _OffsetAffichage );

						if (_IdSelection >= (sint16) Texts.size())
							_IdSelection = (sint16) Texts.size() - 1;
						/*if (_IdSelection < 0)
							_IdSelection = 0;*/

						if(CenterSelected)
							centerSelected();

						LastClicked = SDL_GetTicks();
					}

					if(wheel>0)
						moveSelection(true);
					if(wheel<0)
						moveSelection(false);
				}
				else
				{
					if(click & SDL_BUTTON_LEFT)
					{
						_HasFocus = false;
					}
				}
			}

			return mouseForMe;
		}

		sint8 specialKeyCallback(char car,bool * keys, uint32 elapsed)
		{
			if(Visible)
			{
				if(_HasFocus)
				{
					if(keys[SDL_SCANCODE_DOWN])
					{
						moveSelection(false);
						keys[SDL_SCANCODE_DOWN] = false;
					}

					if(keys[SDL_SCANCODE_UP])
					{
						moveSelection(true);
						keys[SDL_SCANCODE_UP] = false;
					}
				}
			}
			return 0;
		}

		void centerSelected(void)
		{
			_OffsetAffichage = _IdSelection -  (_NbMaxElements/2);
		}

		void setPos(int x, int y, int width, int height)
		{
            this->X = x;
			this->Y = y;
			this->Width = width;
			this->Height = height;
			_Ascenceur->X = X + Width + 5;
			_Ascenceur->Y = Y + _OffsetAffichage;
		}

	private:
		void showAscenceur(bool show)
		{
			_Ascenceur->Width = 8;
			_Ascenceur->Height = 8;

			if(show && !_Ascenceur->Visible)
				Width -= _Ascenceur->Width + 5;
			if(!show && _Ascenceur->Visible)
				Width += _Ascenceur->Width + 5;

			_Ascenceur->X = X + Width + 5;
			_Ascenceur->Y = Y + _OffsetAffichage;

			_Ascenceur->Visible = show;
		}

		void moveSelection(bool up)
		{
			LastChanged = SDL_GetTicks();

			if(up)
			{
				if (_IdSelection > 0)
				{	_IdSelection--;
					if (_IdSelection < _OffsetAffichage)
					{
						_OffsetAffichage--;
						_Ascenceur->Y = (uint32) ( Y+(Height - _Ascenceur->Height) * (float)((float)_OffsetAffichage/(float)(Texts.size()-_NbMaxElements)) );
					}
				}
			}
			else
			{
				if (_IdSelection >= 0 && _IdSelection + 1  < (sint16) Texts.size())
				{
					_IdSelection++;
					if (_IdSelection - _OffsetAffichage >= _NbMaxElements)
					{
						_OffsetAffichage++;
						_Ascenceur->Y = (uint32) ( Y+(Height - _Ascenceur->Height) * (float)((float)_OffsetAffichage/(float)(Texts.size()-_NbMaxElements)) );
					}
				}
			}

			if(CenterSelected)
				centerSelected();
		}

};
