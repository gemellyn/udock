
#include "engine/render/renderer.h"
#include "engine/log/log_console.h"
#include "engine/log/log_file.h"
#include "engine/timer.h"

#include "engine/gui/screen_manager.h"
#include "engine/gui/label.h"
#include "engine/gui/bouton.h"
#include "engine/gui/combox.h"
#include "engine/gui/edtbox.h"

#include "engine/sound/sound.h"
#include "engine/particles/particles_emitter.h"
#include "engine/physics/physic_engine.h"
#include "engine/render/graph/tex_manager.h"


//GUI
GUIScreenManager * g_screen_manager = NULL;
GUIBouton * g_btn_params = NULL;
GUILabel * g_lbl_fps = NULL;
GUIScreen * g_screen_jeu = NULL;
GUIEdtBox * g_edtbox_test = NULL;
GUIComboBox * g_cbox_test = NULL;

NYRenderer * g_renderer = NULL;
NYTimer * g_timer = NULL;
bool g_termine = false;
uint8 g_mouse_btn_gui_state = 0;
NYVert3Df g_mouse_position;

SoundEngine * g_sound_engine = NULL;
SoundBasic * g_sound_ambiance = NULL;

NYBody * g_box_body = NULL;

NYTexFile * g_texture =  NULL;



ParticlesEmitter * partTest = NULL;

bool update(void)
{
    static float g_elapsed_fps = 0;
    static int g_nb_frames = 0;

    float elapsed = g_timer->getElapsedSeconds(true);

	//Calcul des fps
	g_elapsed_fps += elapsed;
	g_nb_frames++;
	if(g_elapsed_fps > 0.25)
	{
		g_lbl_fps->Text = std::string("FPS : ") + toString((int)(g_nb_frames / g_elapsed_fps + 0.5f));
		g_elapsed_fps = 0.0f;
		g_nb_frames = 0;
	}

	//Update son
	g_sound_engine->update(elapsed);

	//Update phy
	NYPhysicEngine::getInstance()->step(elapsed);

	partTest->update(elapsed);

    g_renderer->render(elapsed);
    return false;
}

void render2D(void)
{
    g_screen_manager->render();

    NYTexManager::getInstance()->drawTex2D(10,200,1,1,0,0,*g_texture);
}

void renderObjects(int num_pass)
{
    partTest->render();

    glEnable(GL_LIGHTING);
    glColor3f(1.0,0.3,0.3);
    float matrix[16];
	g_box_body->getOpenGlMatrix(matrix);
	glPushMatrix();
	glMultMatrixf(matrix);
	g_renderer->drawSolidCube(10);
	glPopMatrix();
}

//Interaction Utilisateur
void keyboard_key(int key, bool down)
{
    /*if(down)
        Log::log(Log::ENGINE_INFO,"key down");
    else
        Log::log(Log::ENGINE_INFO,"key up");*/

    if(g_screen_manager->specialKeyCallback(key,down,0))
		return;

    if(key == SDL_SCANCODE_ESCAPE)
        g_termine = true;
}

void keyboard_txt(char car)
{
    //Log::log(Log::ENGINE_INFO,"texte");
    g_screen_manager->keyCallback(car,true,0);
    if(g_screen_manager->keyCallback(car,false,0))
        return;
}

void mouse_move(SDL_Event & evt)
{
    //Log::log(Log::ENGINE_INFO,("mouse move " + toString(evt.motion.x)).c_str());
    bool mouseTraite = false;
    g_mouse_position.X = evt.motion.x;
    g_mouse_position.Y = evt.motion.y;

    if(g_screen_manager->hasActiveScreen())
	{
		mouseTraite = g_screen_manager->mouseCallback(evt.motion.x,evt.motion.y,g_mouse_btn_gui_state,0,0);

	}

	if(mouseTraite)
		return;


}

void mouse_click(SDL_Event & evt, bool down)
{
    bool mouseTraite = false;

    //Log::log(Log::ENGINE_INFO,"mouse click");

    g_mouse_btn_gui_state = 0;
	if(evt.button.button == SDL_BUTTON_LEFT && down)
		g_mouse_btn_gui_state |= GUI_MLBUTTON;

    if(g_screen_manager->hasActiveScreen())
	{
		mouseTraite = g_screen_manager->mouseCallback(evt.button.x,evt.button.y,g_mouse_btn_gui_state,0,0);
	}

	if(mouseTraite)
		return;

}

void mouse_wheel(SDL_Event & evt)
{
    //Log::log(Log::ENGINE_INFO,"mouse wheel");
    if(g_screen_manager->mouseCallback(g_mouse_position.X,g_mouse_position.Y,0,evt.wheel.y,0))
		return;
}


int main(int argc, char **argv)
{
    int screen_width = 800;
    int screen_height = 600;

    bool logConsole = false;
	bool fullLog = false;

	for(int i=0;i<argc;i++)
	{
		if(argv[i][0] == 'w')
		{
			Log::log(Log::ENGINE_INFO,"arg w asking window mode");
			logConsole = true;
		}

		if(argv[i][0] == 'l')
		{
			Log::log(Log::ENGINE_INFO,"arg l asking to show full log");
			fullLog = true;
		}
	}

	//Creation du log
	Log::addLog(new LogFile());

	if(logConsole)
		Log::addLog(new LogConsole());

	if(fullLog)
		Log::showEngineLog(true);

    //INIT SDL

    SDL_Window * fenetre = NULL;
    SDL_GLContext contexteOpenGL = NULL;

    // Initialisation de la SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        Log::log(Log::ENGINE_ERROR,("SDL Init Failed : " + toString(SDL_GetError())).c_str());
        SDL_Quit();
        return -1;
    }

    Log::log(Log::ENGINE_INFO,"SDL Init Ok");

    // Double Buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Création de la fenêtre
    fenetre = SDL_CreateWindow("engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if(fenetre == 0)
    {
        Log::log(Log::ENGINE_ERROR,("SDL Error while creating window : " + toString(SDL_GetError())).c_str());
        SDL_Quit();
        return -1;
    }

    Log::log(Log::ENGINE_INFO,"SDL Window Ok");

    // Création du contexte OpenGL
    contexteOpenGL = SDL_GL_CreateContext(fenetre);
    if(contexteOpenGL == 0)
    {
        Log::log(Log::ENGINE_ERROR,("SDL could not create gl context : " + toString(SDL_GetError())).c_str());
        SDL_DestroyWindow(fenetre);
        SDL_Quit();

        return -1;
    }

    Log::log(Log::ENGINE_INFO,"SDL GL context Ok");

    // On initialise GLEW
    GLenum initialisationGLEW( glewInit() );

    // Si l'initialisation a échouée :
    if(initialisationGLEW != GLEW_OK)
    {
        // On affiche l'erreur grâce à la fonction : glewGetErrorString(GLenum code)

        Log::log(Log::ENGINE_ERROR,("GLEW could not be initialized : " + std::string((char*)glewGetErrorString(initialisationGLEW))).c_str());
        SDL_GL_DeleteContext(contexteOpenGL);
        SDL_DestroyWindow(fenetre);
        SDL_Quit();
        return -1;
    }

    Log::log(Log::ENGINE_INFO,"GLEW init Ok");

    //Init Renderer
    g_renderer = NYRenderer::getInstance();
	//g_renderer->setCam(new NYCameraAnimated());
	g_renderer->setRenderObjectFun(renderObjects);
	//g_renderer->setRenderObjectDepthOnlyFun(renderObjectsDepthOnly);
	g_renderer->setRender2DFun(render2D);
	g_renderer->setRenderObjectFun(renderObjects);
	g_renderer->initialise(fenetre);
	g_renderer->resize(screen_width,screen_height);
	g_renderer->setDrawRepere(true);

	//Moteur Son
	g_sound_engine = SoundEngine::getInstance();

	g_sound_ambiance = new SoundBasic();
	g_sound_ambiance->load("sound/snd_background_1.wav");
	g_sound_ambiance->setVolume(0.5f);
	g_sound_ambiance->_Loop = true;
	g_sound_ambiance->play();
	g_sound_engine->addSound(g_sound_ambiance);

	//Particules
	partTest = new ParticlesEmitter();
    partTest->_Speed = 200.0f;
    partTest->_SpeedLossPerSec = partTest->_Speed * 2.0f;
    partTest->_StartSize = 4.0f;
    partTest->_SizeLossPerSec = partTest->_StartSize * 2.0f;
    partTest->_Transform._Pos = NYVert3Df(0,0,0);
    partTest->_RandomColor = true;
    partTest->_Frequency = 5000.0f;
    partTest->_Dir = NYVert3Df(0,0,1);
    partTest->_RndDir = 0.3f;
    partTest->emit(true);

	//ECRANS GUI
	//Ecran de jeu
	uint16 x = 10;
	uint16 y = 10;
	g_screen_jeu = new GUIScreen();

	g_screen_manager = new GUIScreenManager();

	//Bouton pour afficher les params
	g_btn_params = new GUIBouton();
	g_btn_params->Titre = std::string("Params");
	g_btn_params->X = x;
	g_btn_params->Y = y;
	//BtnParams->setOnClick(clickBtnParams);
	g_screen_jeu->addElement(g_btn_params);

	y += g_btn_params->Height + 1;

	g_lbl_fps = new GUILabel();
	g_lbl_fps->Text = "FPS";
	g_lbl_fps->X = x;
	g_lbl_fps->Y = y;
	g_lbl_fps->Visible = true;
	g_screen_jeu->addElement(g_lbl_fps);

	y += g_lbl_fps->Height + 1;

    g_edtbox_test = new GUIEdtBox();
	g_edtbox_test->X = x;
	g_edtbox_test->Y = y;
	g_edtbox_test->Width = 100;
	g_screen_jeu->addElement(g_edtbox_test);

	y += g_edtbox_test->Height + 6;

	g_cbox_test = new GUIComboBox();
	g_cbox_test->Visible = true;

	g_cbox_test->setPos(x,y,100,5);
	g_cbox_test->LstBox->addElement(std::string("Pouet1"));
	g_cbox_test->LstBox->addElement(std::string("Pouet2"));
	g_cbox_test->LstBox->addElement(std::string("Pouet3"));
	g_cbox_test->LstBox->addElement(std::string("Pouet4"));
	g_cbox_test->LstBox->addElement(std::string("Pouet5"));
	g_cbox_test->LstBox->addElement(std::string("Pouet6"));
	g_cbox_test->LstBox->addElement(std::string("Pouet7"));
	g_screen_jeu->addElement(g_cbox_test);

    y += g_cbox_test->Height + 1;

    g_screen_manager->setActiveScreen(g_screen_jeu);

    //Init physique
    NYPhysicEngine::getInstance()->initialisation(NYVert3Df(0,0,-9.81));
    NYPhysicEngine::getInstance()->addPlaneObject(false,NYVert3Df(0,0,1),NYVert3Df(0,0,0),NULL,0);
    g_box_body = NYPhysicEngine::getInstance()->addBoxObject(true,NYVert3Df(10,10,10),NYVert3Df(0,0,100),NULL,0);


    //Texture
    g_texture = NYTexManager::getInstance()->loadTexture(std::string("textures/grass.png"));

    //timer
    g_timer = new NYTimer();
    g_timer->start();


    // Boucle principale
    bool termine = false;

    SDL_Event evt;
    SDL_StartTextInput();
    while(!termine && !g_termine)
    {
        //Update de l'applications
        termine = update();

        //Gestion des messages fenetre
        while (SDL_PollEvent(&evt))
        {
            switch (evt.type)
            {
                case SDL_QUIT: termine = true; break;
                case SDL_KEYDOWN: keyboard_key(evt.key.keysym.scancode,true); break;
                case SDL_KEYUP: keyboard_key(evt.key.keysym.scancode,false); break;
                case SDL_MOUSEMOTION: mouse_move(evt); break;
                case SDL_MOUSEBUTTONDOWN: mouse_click(evt,true); break;
                case SDL_MOUSEBUTTONUP: mouse_click(evt,false); break;
                case SDL_MOUSEWHEEL: mouse_wheel(evt); break;
                case SDL_TEXTINPUT: keyboard_txt(evt.text.text[0]); break;
            }
        }
    }

    // On quitte la SDL
    SDL_GL_DeleteContext(contexteOpenGL);
    SDL_DestroyWindow(fenetre);
    SDL_Quit();

    return 0;
}
