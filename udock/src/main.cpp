//Includes ENGINE

#include "engine/network/osc.h"

#include "engine/log/log_console.h"
#include "engine/log/log_file.h"
#include "engine/timer.h"

#include "engine/render/renderer.h"
#include "engine/render/camera_animated.h"
#include "engine/render/graph/tex_manager.h"
#include "engine/particles/particles_emitter.h"
#include "engine/physics/physic_engine.h"
#include "engine/sound/sound.h"
#include "engine/sound/granular_sound.h"
 
#include "engine/gui/screen.h"
#include "engine/gui/screen_manager.h"
#include "engine/gui/label.h"
#include "engine/gui/bouton.h"
#include "engine/gui/combox.h"
#include "engine/gui/edtbox.h"
#include "engine/gui/loading.h"



//Includes UDOCK
#include "log/log_screen.h"
#include "log/log_actions_reader.h"
#include "molecules/molecules_manager.h"
#include "molecules/marching_cubes/molecule_cubes.h"
#include "ships/ships_manager.h"
#include "ships/projectile.h"

#include "network/http_manager_total_deco.h"
#include "network/http_manager_public_deco.h"


#include "game_states/mol_state.h"
#include "game_states/ship_state.h"
#include "game_states/load_state.h"
#include "game_states/choose_mol_state.h"
#include "game_states/login_state.h"
#include "game_states/login_auto_state.h"

#include "tutorial/tutorial.h"

#include "boost/date_time/time.hpp"
#include "boost/static_assert.hpp"


//Variables globales

NYPhysicEngine * g_phyEngine = NULL;
NYRenderer * g_renderer = NULL;
NYTimer * g_timer = NULL;

int g_nb_frames = 0;
float g_elapsed_fps = 0;
int g_main_window_id;
int g_mouse_btn_gui_state = 0;
bool g_termine = false;
NYVert3Df g_mouse_position;

//States
MolState * g_mol_state = NULL;
ShipState * g_ship_state = NULL;
LoadState * g_load_state = NULL;
ChooseMolState * g_choose_mol_state = NULL;
LoginState * g_login_state = NULL;

//Molecules
MoleculesManager * g_moleculesManager = NULL;

//Vaisseaux
ShipManager * g_ship_manager = NULL;
ProjectilesManager * g_proj_manager = NULL;

//GUI
GUIScreenManager * g_screen_manager = NULL;
GUIScreen * g_screen_params = NULL;
GUIScreen * g_screen_jeu = NULL;
GUIScreen * g_screen_choose_mol = NULL;
GUIScreen * g_screen_load_mols = NULL;
GUIScreen * g_screen_login = NULL;
GUIScreen * g_screen_credits = NULL;
GUIScreen * g_screen_controls = NULL;
GUIScreen * g_screen_log = NULL; //Petit message en base toujours visible

GUIBouton * BtnParams = NULL;
GUIBouton * BtnSwitch = NULL;
GUIBouton * BtnFreezePhysics = NULL;
GUIBouton * BtnLoad = NULL;
GUIBouton * BtnLogin = NULL;
GUIBouton * BtnHelp = NULL;
GUIBouton * BtnCredits = NULL;
GUIBouton * BtnGenerate = NULL;
GUIBouton * g_bouton_dock = NULL;

GUISlider * g_slider_lower_fps = NULL;
GUISlider * g_slider_random;
GUISlider * g_slider_pos;
GUISlider * g_slider_size;
GUISlider * g_slider_overlap;
GUISlider * g_slider_volume_ambiance;
GUISlider * g_slider_ambient_light;
GUISlider * g_slider_color_pos_h;
GUISlider * g_slider_color_pos_s;
GUISlider * g_slider_color_pos_v;
GUISlider * g_slider_color_neg_h;
GUISlider * g_slider_color_neg_s;
GUISlider * g_slider_color_neg_v;
GUISlider * g_slider_color_fond_h;
GUISlider * g_slider_color_fond_s;
GUISlider * g_slider_color_fond_v;
GUISlider * g_slider_cerne;

GUILabel * LabelFps = NULL;
GUILabel * g_lbl_random;
GUILabel * g_lbl_pos;
GUILabel * g_lbl_size;
GUILabel * g_lbl_overlap;
GUILabel * g_lbl_best_login;
GUILabel * g_lbl_url_udock;
GUILabel * g_lbl_welcome;
GUILabel * g_lbl_log;
GUILabel * g_lbl_version;
GUILabel * g_lbl_best_player;
GUILabel * g_lbl_beated_names;
GUILabel * g_label_load;
GUILabel * g_label_time;

GUIPBar * g_pbar_acquire;
GUIPBar * g_pbar_loading;
GUIPBar * g_pbar_score;
GUIPBar * g_pbar_charge;
GUIPBar * g_pbar_best_score;
GUIPBar * g_pbar_optim;

GUILoading * g_loading;

GUIComboBox * g_cbox_mol1;
GUIComboBox * g_cbox_mol2;

GUIEdtBox * EdtLogin;
GUIEdtBox * EdtPass;

GUILstBox * g_lst_scores;

//Sons
SoundEngine * g_sound_engine = NULL;
#define NB_SOUND_ATTACH 6
SoundBasic * g_sounds_attach[NB_SOUND_ATTACH];
SoundBasic * g_sound_detach;
SoundBasic * g_sound_high_score;
SoundBasic * g_sound_reset;
SoundGrain * g_sound_grain;
SoundBasic * g_sound_ambiance;

//Shaders
GLuint g_program_mol;
NYColor g_color_positif(0.0,0.0,1.0,1.0);
NYColor g_color_negatif(1.0,0.0,0.0,1.0);
NYColor g_color_neutre(1.0,1.0,1.0,1.0);
NYColor g_color_fond(0.0,0.347,0.398,1.0);
float g_ambient_level = 0.75f;
float g_taille_cerne = 0.0f;
bool g_invert_polarity = false;

//Projection sphérique
GLuint g_program_spherique;
GLuint g_program_spherique_mix;
float g_xoffset_spherique = 0;
float g_yoffset_spherique = 0;
float g_rotatex_spherique = 0;
float g_rotatey_spherique = 0;

//Smoothing
GLuint g_program_smooth_proj;
GLuint g_program_smooth_tex;
GLuint g_program_smooth_smooth;

//Temps
float g_elapsed_time_dock = 0;

//Flags pour screen shots
bool g_hideShipScreenShot = false;
bool g_hideInterfaceScreenShot = false;

//Controller
SDL_Joystick  * g_gamepad = NULL;


//////////////////////////////////////////////////////////////////////////
// GESTION APPLICATION
//////////////////////////////////////////////////////////////////////////

void mousePollFunction (float elapsed);
void checkJoysticks();
void pollGamePad();

void runApplication(float elapsed)
{
	static float g_autofit_elapsed = 0;
	static float g_eval_elapsed = 0;
	static Ship * last_selected_ship = NULL;

	g_elapsed_time_dock+= elapsed;

	//Si on est sous 10fps, on bloque a 10 (peut etre long step de traitement)
	if(elapsed > 0.1f)
		elapsed = 0.1f;

	//Limitation fps
	#ifdef TOTAL_DECO
	if(g_slider_lower_fps->Value > 0.0)
		Sleep((int)g_slider_lower_fps->Value);
	#endif

	//Calcul des fps
	g_elapsed_fps += elapsed;
	g_nb_frames++;
	if(g_elapsed_fps > 1.0)
	{
		LabelFps->Text = std::string("FPS : ") + toString(g_nb_frames) + std::string(" smooth : ") + toString(g_mol_state->_SmoothLevel) ;
		if(g_mol_state->_Active)
			LabelFps->Text += std::string(" [0] : ") + g_moleculesManager->_Molecules[0]->_MolName;

		g_elapsed_fps -= 1.0f;
		g_nb_frames = 0;
	}

	//Updates variables globales sales
	g_xoffset_spherique += g_rotatex_spherique * NYRenderer::_DeltaTime;
	g_yoffset_spherique += g_rotatey_spherique * NYRenderer::_DeltaTime;

	if(g_mol_state->_Active)
	{
		g_mol_state->_SmoothLevel += g_rotatey_spherique * NYRenderer::_DeltaTime;
		if(g_rotatey_spherique != 0)
			g_mol_state->_FirstPass = true;
		if(g_mol_state->_Precise)
			g_mol_state->_FirstPass = true;
		g_mol_state->_MorphLevel += g_rotatex_spherique * NYRenderer::_DeltaTime;
	}


	if(g_mol_state->_SmoothLevel > 1.0f)
		g_mol_state->_SmoothLevel = 1.0f;
	if(g_mol_state->_SmoothLevel < 0.0f)
		g_mol_state->_SmoothLevel = 0.0f;

	if(g_mol_state->_MorphLevel > 1.0f)
		g_mol_state->_MorphLevel = 1.0f;
	if(g_mol_state->_MorphLevel < 0.0f)
		g_mol_state->_MorphLevel = 0.0f;

	if(abs(g_xoffset_spherique) > 2*M_PI)
		g_xoffset_spherique += 2*M_PI;

	if(abs(g_xoffset_spherique) > 2*M_PI)
		g_xoffset_spherique -= nysign(g_xoffset_spherique)*2*M_PI;
	if(abs(g_yoffset_spherique) > 2*M_PI)
		g_yoffset_spherique -= nysign(g_yoffset_spherique)*2*M_PI;

	//Update ecrans
	g_screen_manager->update(elapsed);
	g_screen_log->update(elapsed);

	//Update son
	g_sound_engine->update(elapsed);

	//Inputs
	mousePollFunction(elapsed);
	checkJoysticks();
	pollGamePad();

	//Paramétrage du rendu
	if(!g_mol_state->_Active)
	{
		g_renderer->setNbPasses(1);
	}
	else
	{
		switch(g_mol_state->_RenderMode)
		{
			case 4:
				g_renderer->setNbPasses(3);
				g_renderer->setDrawRepere(false);
				break;

			case 2:
			case 3:
				//glClearDepth(0.0f);
				//glDepthFunc(GL_GREATER);
				g_renderer->setNbPasses(1);
				g_renderer->setDrawRepere(false);
				break;

			case 12:
				g_renderer->setBackgroundColor(NYColor(0,0,0,0));
				g_renderer->setNbPasses(2);
				g_renderer->setPostProcess(false);
				g_renderer->setDrawRepere(false);
				break;

			case 1:
				g_renderer->setPostProcess(true);
				if(g_mol_state->_SmoothLevel > 0.0f && g_mol_state->_FirstPass)
				{
					g_renderer->setNbPasses(5);
					g_renderer->setDrawRepere(false);
					g_renderer->setBackgroundColor(NYColor(0,0,0,0));
				}
				else
				{
					g_renderer->setNbPasses(1);
					g_renderer->setDrawRepere(false);
					g_renderer->setBackgroundColor(g_color_fond);
				}
				break;
			default:
				g_renderer->setNbPasses(1);
				g_renderer->setDrawRepere(false);
				g_renderer->setPostProcess(true);
				break;
		}
	}

	//Debut des states
	if(g_mol_state->_Active)
		g_mol_state->preState(elapsed);

	if(g_ship_state->_Active)
		g_ship_state->preState(elapsed);

	if(g_load_state->_Active)
	{
		g_elapsed_time_dock = 0;
		g_load_state->preState(elapsed);
	}

	if(g_choose_mol_state->_Active)
	{
		g_elapsed_time_dock = 0;
		g_choose_mol_state->preState(elapsed);
	}

	if(g_login_state->_Active)
	{
		g_elapsed_time_dock = 0;
		g_login_state->preState(elapsed);
	}

	//Temps ecoule
	if(g_elapsed_time_dock > 0)
		g_lbl_log->Text = toString((int)(g_elapsed_time_dock/60)) + "mn " + toString(((int)(g_elapsed_time_dock))%60)+"s";
	else
		g_lbl_log->Text = "";

	//Update des objets
	float updatePhy = min(elapsed,1.0f/60.0f); //Bullet fait de la merde si sous 60hz...
	g_ship_manager->updateBeforePhysic(updatePhy);

	//update du moteur physique
	g_phyEngine->step(updatePhy);

	//Mise a jour des objets de jeu en fonction de la physique
	g_moleculesManager->updateAfterPhysic(updatePhy);
	g_ship_manager->updateAfterPhysic(updatePhy);
	g_proj_manager->updateAfterPhysic(updatePhy);

	//Fin des states
	if(g_mol_state->_Active)
		g_mol_state->postState(elapsed);

	if(g_ship_state->_Active)
		g_ship_state->postState(elapsed);

	if(g_load_state->_Active)
		g_load_state->postState(elapsed);

	if(g_choose_mol_state->_Active)
		g_choose_mol_state->postState(elapsed);

	if(g_login_state->_Active)
		g_login_state->postState(elapsed);
}

void render2D(void)
{
	if(!g_hideInterfaceScreenShot)
	{
		if(g_mol_state->_Active)
			g_mol_state->render2D();
		if(g_choose_mol_state->_Active)
			g_choose_mol_state->render2D();
		g_screen_manager->render();
		g_screen_log->render();
	}
}
void renderObjectsDepthOnly(void)
{
	g_moleculesManager->renderHiddenObjects();
}

void renderObjects(int num_pass)
{
	if(g_choose_mol_state->_Active)
		g_renderer->setDrawRepere(false);
	else
	{
		for(int i=0;i<g_moleculesManager->_Molecules.size();i++)
		{
			//Mode debug

			if(g_mol_state->_RenderMode == 12)
			{

				if(num_pass == 0 && i == 0)
				{
					glUseProgram(g_program_smooth_proj);

					GLuint rot = glGetUniformLocation(g_program_smooth_proj, "rotation");
					glUniformMatrix4fv(rot,1,false,g_moleculesManager->_Molecules[i]->_Transform._Rot.Mat.t);
					GLuint pos = glGetUniformLocation(g_program_smooth_proj, "pos");
					glUniform4f(pos,g_moleculesManager->_Molecules[i]->_Transform._Pos.X,
						g_moleculesManager->_Molecules[i]->_Transform._Pos.Y,
						g_moleculesManager->_Molecules[i]->_Transform._Pos.Z,0.0f);
					GLuint barycenter = glGetUniformLocation(g_program_smooth_proj, "barycenter");
					glUniform4f(barycenter, g_moleculesManager->_Molecules[i]->_Barycenter.X,
						g_moleculesManager->_Molecules[i]->_Barycenter.Y,
						g_moleculesManager->_Molecules[i]->_Barycenter.Z,
						0.0f);

					GLuint ratio = glGetUniformLocation(g_program_smooth_proj, "ratio");
					glUniform1f(ratio, (float)g_renderer->_ScreenWidth / (float)g_renderer->_ScreenHeight );

					glDisable(GL_CULL_FACE);

					//On en rend qu'une seule, en fonction du mode
					GLuint maxSize = glGetUniformLocation(g_program_smooth_proj, "molMaxSize");
					float maxDim;
					g_moleculesManager->_Molecules[0]->getMaxDim(maxDim);
					glUniform1f(maxSize,maxDim);
					g_moleculesManager->_Molecules[0]->render();
				}

				if(num_pass == 1 && i == 0)
				{
					glUseProgram(g_program_smooth_tex);

					GLuint smooth = glGetUniformLocation(g_program_smooth_tex, "smoothing");
					glUniform1f(smooth, g_mol_state->_SmoothLevel);

					g_renderer->sendPassTexsToShader(0,false,"colorTex1",0,"depthTex1",1);
					g_renderer->renderFullScreenQuad();
				}


			}



			//Mode normal, 3d
			if(g_mol_state->_RenderMode == 1)
			{
				//Si on smooth pas, mode normal
				if(g_mol_state->_SmoothLevel == 0.0f)
				{
					glUseProgram(g_program_mol);

					GLuint colPos = glGetUniformLocation(g_program_mol, "colorPositif");
					GLuint colNeg = glGetUniformLocation(g_program_mol, "colorNegatif");
					GLuint colNet = glGetUniformLocation(g_program_mol, "colorNeutre");

					glUniform4f(colNet, g_color_neutre.R,g_color_neutre.V,g_color_neutre.B,g_color_neutre.A);
					if(g_invert_polarity && i==1)
					{
						glUniform4f(colPos, g_color_negatif.R,g_color_negatif.V,g_color_negatif.B,g_color_negatif.A);
						glUniform4f(colNeg, g_color_positif.R,g_color_positif.V,g_color_positif.B,g_color_positif.A);
					}
					else
					{
						glUniform4f(colPos, g_color_positif.R,g_color_positif.V,g_color_positif.B,g_color_positif.A);
						glUniform4f(colNeg, g_color_negatif.R,g_color_negatif.V,g_color_negatif.B,g_color_negatif.A);
					}

					GLuint ambLvl = glGetUniformLocation(g_program_mol, "ambientLevel");
					glUniform1f(ambLvl, g_ambient_level);
					GLuint cerne = glGetUniformLocation(g_program_mol, "tailleCerne");
					glUniform1f(cerne, g_taille_cerne);

					GLuint lightpos = glGetUniformLocation(g_program_mol, "lightpos");
					glUniform3f(lightpos, -g_renderer->_Camera->_Direction.X,
						-g_renderer->_Camera->_Direction.Y,
						-g_renderer->_Camera->_Direction.Z);


					g_moleculesManager->_Molecules[i]->render();
				}
				else
				{
					//On est en mode smooth

					//On fait la projection spherique dans une texture de chaque molecule
					if((num_pass == 0 || num_pass == 1) && g_mol_state->_FirstPass)
					{
						glUseProgram(g_program_smooth_proj);

						GLuint rot = glGetUniformLocation(g_program_smooth_proj, "rotation");
						glUniformMatrix4fv(rot,1,false,g_moleculesManager->_Molecules[i]->_Transform._Rot.Mat.t);
						GLuint pos = glGetUniformLocation(g_program_smooth_proj, "pos");
						glUniform4f(pos,g_moleculesManager->_Molecules[i]->_Transform._Pos.X,
							g_moleculesManager->_Molecules[i]->_Transform._Pos.Y,
							g_moleculesManager->_Molecules[i]->_Transform._Pos.Z,0.0f);
						GLuint barycenter = glGetUniformLocation(g_program_smooth_proj, "barycenter");
						glUniform4f(barycenter, g_moleculesManager->_Molecules[i]->_Barycenter.X,
							g_moleculesManager->_Molecules[i]->_Barycenter.Y,
							g_moleculesManager->_Molecules[i]->_Barycenter.Z,
							0.0f);

						GLuint ratio = glGetUniformLocation(g_program_smooth_proj, "ratio");
						glUniform1f(ratio, (float)g_renderer->_ScreenWidth / (float)g_renderer->_ScreenHeight );

						GLuint maxSize = glGetUniformLocation(g_program_smooth_proj, "molMaxSize");

						glDisable(GL_CULL_FACE);

						//On en rend qu'une seule, en fonction du mode
						if(num_pass == 0 && i == 0)
						{
							float maxDim;
							g_moleculesManager->_Molecules[0]->getMaxDim(maxDim);
							glUniform1f(maxSize,maxDim);
							g_moleculesManager->_Molecules[0]->render();
						}
						if(num_pass == 1 && i == 0)
						{
							float maxDim;
							g_moleculesManager->_Molecules[1]->getMaxDim(maxDim);
							glUniform1f(maxSize,maxDim);
							g_moleculesManager->_Molecules[1]->render();
						}
					}

					//On smoothe la projection spherique de chaque mol dans une texture
					if((num_pass == 2 || num_pass == 3) && g_mol_state->_FirstPass)
					{
						if(i==0)
						{
							glUseProgram(g_program_smooth_tex);

							GLuint smooth = glGetUniformLocation(g_program_smooth_tex, "smoothing");
							glUniform1f(smooth, g_mol_state->_SmoothLevel);

							GLuint precise = glGetUniformLocation(g_program_smooth_tex, "precise");
							glUniform1i(precise, g_mol_state->_Precise ? 1 : 0);

							g_renderer->sendPassTexsToShader(num_pass-2,false,"colorTex1",0,"depthTex1",1);
							g_renderer->renderFullScreenQuad();
						}

						if(num_pass == 3 && i == 1)
						{
							g_renderer->setDrawRepere(false);
							g_renderer->setBackgroundColor(g_color_fond);
						}
					}

					//Pour que lors de la première phase de smooth, on ait quand meme un rendu des mols
					if((num_pass == 4 && g_mol_state->_FirstPass) ||
					 (num_pass == 0 && !g_mol_state->_FirstPass))
					{
						glUseProgram(g_program_smooth_smooth);
						g_renderer->sendPassTexsToShader(i+2,false,"colorTex1",0,"depthTex1",1);

						GLuint pos = glGetUniformLocation(g_program_smooth_smooth, "pos");
						glUniform4f(pos,g_moleculesManager->_Molecules[i]->_Transform._Pos.X,
							g_moleculesManager->_Molecules[i]->_Transform._Pos.Y,
							g_moleculesManager->_Molecules[i]->_Transform._Pos.Z,0.0f);
						GLuint barycenter = glGetUniformLocation(g_program_smooth_smooth, "barycenter");
						glUniform4f(barycenter, g_moleculesManager->_Molecules[i]->_Barycenter.X,
							g_moleculesManager->_Molecules[i]->_Barycenter.Y,
							g_moleculesManager->_Molecules[i]->_Barycenter.Z,
							0.0f);

						GLuint ratio = glGetUniformLocation(g_program_smooth_smooth, "ratio");
						glUniform1f(ratio, (float)g_renderer->_ScreenWidth / (float)g_renderer->_ScreenHeight );

						GLuint colPos = glGetUniformLocation(g_program_smooth_smooth, "colorPositif");
						GLuint colNeg = glGetUniformLocation(g_program_smooth_smooth, "colorNegatif");
						GLuint colNet = glGetUniformLocation(g_program_smooth_smooth, "colorNeutre");

						glUniform4f(colNet, g_color_neutre.R,g_color_neutre.V,g_color_neutre.B,g_color_neutre.A);
						if(g_invert_polarity && i==1)
						{
							glUniform4f(colPos, g_color_negatif.R,g_color_negatif.V,g_color_negatif.B,g_color_negatif.A);
							glUniform4f(colNeg, g_color_positif.R,g_color_positif.V,g_color_positif.B,g_color_positif.A);
						}
						else
						{
							glUniform4f(colPos, g_color_positif.R,g_color_positif.V,g_color_positif.B,g_color_positif.A);
							glUniform4f(colNeg, g_color_negatif.R,g_color_negatif.V,g_color_negatif.B,g_color_negatif.A);
						}

						GLuint ambLvl = glGetUniformLocation(g_program_smooth_smooth, "ambientLevel");
						glUniform1f(ambLvl, g_ambient_level);
						GLuint cerne = glGetUniformLocation(g_program_smooth_smooth, "tailleCerne");
						glUniform1f(cerne, g_taille_cerne);

						GLuint lightpos = glGetUniformLocation(g_program_smooth_smooth, "lightpos");
						glUniform3f(lightpos, -g_renderer->_Camera->_Direction.X,
							-g_renderer->_Camera->_Direction.Y,
							-g_renderer->_Camera->_Direction.Z);


						GLuint morph = glGetUniformLocation(g_program_smooth_smooth, "morphing");
						glUniform1f(morph, g_mol_state->_MorphLevel);

						GLuint maxSize = glGetUniformLocation(g_program_smooth_smooth, "molMaxSize");

						float maxDim;
						g_moleculesManager->_Molecules[i]->getMaxDim(maxDim);
						glUniform1f(maxSize,maxDim);
						g_moleculesManager->_Molecules[i]->render();

						if(i == 1)
						{
							g_mol_state->_FirstPass = false;
							g_mol_state->_Precise = false;
						}
					}
				}
			}

					//On fait la projection spherique dans une texture de chaque molecule
					/*if((num_pass == 0 || num_pass == 1) && g_mol_state->_FirstPass)
					{
						glUseProgram(g_program_smooth_proj);

						GLuint rot = glGetUniformLocation(g_program_smooth_proj, "rotation");
						glUniformMatrix4fv(rot,1,false,g_moleculesManager->_Molecules[i]->_Transform._Rot.Mat.t);
						GLuint pos = glGetUniformLocation(g_program_smooth_proj, "pos");
						glUniform4f(pos,g_moleculesManager->_Molecules[i]->_Transform._Pos.X,
							g_moleculesManager->_Molecules[i]->_Transform._Pos.Y,
							g_moleculesManager->_Molecules[i]->_Transform._Pos.Z,0.0f);
						GLuint barycenter = glGetUniformLocation(g_program_smooth_proj, "barycenter");
						glUniform4f(barycenter, g_moleculesManager->_Molecules[i]->_Barycenter.X,
							g_moleculesManager->_Molecules[i]->_Barycenter.Y,
							g_moleculesManager->_Molecules[i]->_Barycenter.Z,
							0.0f);

						GLuint ratio = glGetUniformLocation(g_program_smooth_proj, "ratio");
						glUniform1f(ratio, (float)g_renderer->_ScreenWidth / (float)g_renderer->_ScreenHeight );

						GLuint maxSize = glGetUniformLocation(g_program_smooth_proj, "molMaxSize");

						glDisable(GL_CULL_FACE);

						//On en rend qu'une seule, en fonction du mode
						if(num_pass == 0 && i == 0)
						{
							float maxDim;
							g_moleculesManager->_Molecules[0]->getMaxDim(maxDim);
							glUniform1f(maxSize,maxDim);
							g_moleculesManager->_Molecules[0]->render();
						}
						if(num_pass == 1 && i == 0)
						{
							float maxDim;
							g_moleculesManager->_Molecules[1]->getMaxDim(maxDim);
							glUniform1f(maxSize,maxDim);
							g_moleculesManager->_Molecules[1]->render();
						}
					}

					//On smoothe la projection spherique de chaque mol dans une texture
					if((num_pass == 2 || num_pass == 3) && g_mol_state->_FirstPass)
					{
						if(i==0)
						{
							glUseProgram(g_program_smooth_tex);

							GLuint smooth = glGetUniformLocation(g_program_smooth_tex, "smoothing");
							glUniform1f(smooth, g_mol_state->_SmoothLevel);

							g_renderer->sendPassTexsToShader(num_pass-2,false,"colorTex1",0,"depthTex1",1);
							g_renderer->renderFullScreenQuad();
						}

						if(num_pass == 3 && i == 1)
						{
							g_renderer->setDrawRepere(false);
							g_renderer->setBackgroundColor(g_color_fond);
						}
					}

					//Pour que lors de la première phase de smooth, on ait quand meme un rendu des mols
					if((num_pass == 4 && g_mol_state->_FirstPass) ||
					 (num_pass == 0 && !g_mol_state->_FirstPass))
					{
						glUseProgram(g_program_smooth_smooth);
						g_renderer->sendPassTexsToShader(i+2,false,"colorTex1",0,"depthTex1",1);

						GLuint pos = glGetUniformLocation(g_program_smooth_smooth, "pos");
						glUniform4f(pos,g_moleculesManager->_Molecules[i]->_Transform._Pos.X,
							g_moleculesManager->_Molecules[i]->_Transform._Pos.Y,
							g_moleculesManager->_Molecules[i]->_Transform._Pos.Z,0.0f);
						GLuint barycenter = glGetUniformLocation(g_program_smooth_smooth, "barycenter");
						glUniform4f(barycenter, g_moleculesManager->_Molecules[i]->_Barycenter.X,
							g_moleculesManager->_Molecules[i]->_Barycenter.Y,
							g_moleculesManager->_Molecules[i]->_Barycenter.Z,
							0.0f);

						GLuint ratio = glGetUniformLocation(g_program_smooth_smooth, "ratio");
						glUniform1f(ratio, (float)g_renderer->_ScreenWidth / (float)g_renderer->_ScreenHeight );

						GLuint colPos = glGetUniformLocation(g_program_smooth_smooth, "colorPositif");
						GLuint colNeg = glGetUniformLocation(g_program_smooth_smooth, "colorNegatif");
						GLuint colNet = glGetUniformLocation(g_program_smooth_smooth, "colorNeutre");

						glUniform4f(colNet, g_color_neutre.R,g_color_neutre.V,g_color_neutre.B,g_color_neutre.A);
						if(g_invert_polarity && i==1)
						{
							glUniform4f(colPos, g_color_negatif.R,g_color_negatif.V,g_color_negatif.B,g_color_negatif.A);
							glUniform4f(colNeg, g_color_positif.R,g_color_positif.V,g_color_positif.B,g_color_positif.A);
						}
						else
						{
							glUniform4f(colPos, g_color_positif.R,g_color_positif.V,g_color_positif.B,g_color_positif.A);
							glUniform4f(colNeg, g_color_negatif.R,g_color_negatif.V,g_color_negatif.B,g_color_negatif.A);
						}

						GLuint ambLvl = glGetUniformLocation(g_program_smooth_smooth, "ambientLevel");
						glUniform1f(ambLvl, g_ambient_level);
						GLuint cerne = glGetUniformLocation(g_program_smooth_smooth, "tailleCerne");
						glUniform1f(cerne, g_taille_cerne);

						GLuint lightpos = glGetUniformLocation(g_program_smooth_smooth, "lightpos");
						glUniform3f(lightpos, -g_renderer->_Camera->_Direction.X,
							-g_renderer->_Camera->_Direction.Y,
							-g_renderer->_Camera->_Direction.Z);


						GLuint morph = glGetUniformLocation(g_program_smooth_smooth, "morphing");
						glUniform1f(morph, g_mol_state->_MorphLevel);

						GLuint maxSize = glGetUniformLocation(g_program_smooth_smooth, "molMaxSize");

						float maxDim;
						g_moleculesManager->_Molecules[i]->getMaxDim(maxDim);
						glUniform1f(maxSize,maxDim);
						g_moleculesManager->_Molecules[i]->render();
						//g_renderer->drawSolidSphere(1.0f,20,20);
						//g_renderer->drawSolidCube(1.0f);


						if(i == 1)
						{
							g_mol_state->_FirstPass = false;
						}
					}
				}
			}*/

			//Mode carte, pour une molecule
			if(g_mol_state->_RenderMode == 2 || g_mol_state->_RenderMode == 3)
			{
				glUseProgram(g_program_spherique);

				GLuint rot = glGetUniformLocation(g_program_spherique, "rotation");
				glUniformMatrix4fv(rot,1,false,g_moleculesManager->_Molecules[i]->_Transform._Rot.Mat.t);
				GLuint pos = glGetUniformLocation(g_program_spherique, "pos");
				glUniform4f(pos,g_moleculesManager->_Molecules[i]->_Transform._Pos.X,
					g_moleculesManager->_Molecules[i]->_Transform._Pos.Y,
					g_moleculesManager->_Molecules[i]->_Transform._Pos.Z,0.0f);
				GLuint barycenter = glGetUniformLocation(g_program_spherique, "barycenter");
				glUniform4f(barycenter, g_moleculesManager->_Molecules[i]->_Barycenter.X,
										g_moleculesManager->_Molecules[i]->_Barycenter.Y,
										g_moleculesManager->_Molecules[i]->_Barycenter.Z,
										0.0f);

				GLuint colPos = glGetUniformLocation(g_program_spherique, "colorPositif");
				glUniform4f(colPos, g_color_positif.R,g_color_positif.V,g_color_positif.B,g_color_positif.A);
				GLuint colNeg = glGetUniformLocation(g_program_spherique, "colorNegatif");
				glUniform4f(colNeg, g_color_negatif.R,g_color_negatif.V,g_color_negatif.B,g_color_negatif.A);
				GLuint colNet = glGetUniformLocation(g_program_spherique, "colorNeutre");
				glUniform4f(colNet, g_color_neutre.R,g_color_neutre.V,g_color_neutre.B,g_color_neutre.A);

				GLuint ratio = glGetUniformLocation(g_program_spherique, "ratio");
				glUniform1f(ratio, (float)g_renderer->_ScreenWidth / (float)g_renderer->_ScreenHeight );

				GLuint xoffset = glGetUniformLocation(g_program_spherique, "xoffset");
				glUniform1f(xoffset, g_xoffset_spherique );
				GLuint yoffset = glGetUniformLocation(g_program_spherique, "yoffset");
				glUniform1f(yoffset, g_yoffset_spherique );

				GLuint invert = glGetUniformLocation(g_program_spherique, "invert");
				glUniform1i(invert,i);

				GLuint maxSize = glGetUniformLocation(g_program_spherique, "molMaxSize");

				GLuint projType = glGetUniformLocation(g_program_spherique, "projType");
				glUniform1i(projType,g_mol_state->_ProjType);

				g_renderer->setDrawRepere(false);
				glDisable(GL_CULL_FACE);

				//On en rend qu'une seule, en fonction du mode
				if(g_mol_state->_RenderMode == 2 && i == 0)
				{
					float maxDim;
					g_moleculesManager->_Molecules[0]->getMaxDim(maxDim);
					glUniform1f(maxSize,maxDim);
					g_moleculesManager->_Molecules[0]->render();
				}

				if(g_mol_state->_RenderMode == 3 && i == 1)
				{
					float maxDim;
					g_moleculesManager->_Molecules[1]->getMaxDim(maxDim);
					glUniform1f(maxSize,maxDim);
					g_moleculesManager->_Molecules[1]->render();
				}
			}

			//Mode carte, fusion des deux cartes
			if(g_mol_state->_RenderMode == 4)
			{
				if(num_pass == 0 || num_pass == 1)
				{
					glUseProgram(g_program_spherique);

					GLuint rot = glGetUniformLocation(g_program_spherique, "rotation");
					glUniformMatrix4fv(rot,1,false,g_moleculesManager->_Molecules[i]->_Transform._Rot.Mat.t);
					GLuint pos = glGetUniformLocation(g_program_spherique, "pos");
					glUniform4f(pos,g_moleculesManager->_Molecules[i]->_Transform._Pos.X,
						g_moleculesManager->_Molecules[i]->_Transform._Pos.Y,
						g_moleculesManager->_Molecules[i]->_Transform._Pos.Z,0.0f);
					GLuint barycenter = glGetUniformLocation(g_program_spherique, "barycenter");
					glUniform4f(barycenter, g_moleculesManager->_Molecules[i]->_Barycenter.X,
						g_moleculesManager->_Molecules[i]->_Barycenter.Y,
						g_moleculesManager->_Molecules[i]->_Barycenter.Z,
						0.0f);

					GLuint colPos = glGetUniformLocation(g_program_spherique, "colorPositif");
					glUniform4f(colPos, g_color_positif.R,g_color_positif.V,g_color_positif.B,g_color_positif.A);
					GLuint colNeg = glGetUniformLocation(g_program_spherique, "colorNegatif");
					glUniform4f(colNeg, g_color_negatif.R,g_color_negatif.V,g_color_negatif.B,g_color_negatif.A);
					GLuint colNet = glGetUniformLocation(g_program_spherique, "colorNeutre");
					glUniform4f(colNet, g_color_neutre.R,g_color_neutre.V,g_color_neutre.B,g_color_neutre.A);

					GLuint ratio = glGetUniformLocation(g_program_spherique, "ratio");
					glUniform1f(ratio, (float)g_renderer->_ScreenWidth / (float)g_renderer->_ScreenHeight );

					GLuint xoffset = glGetUniformLocation(g_program_spherique, "xoffset");
					glUniform1f(xoffset, g_xoffset_spherique );
					GLuint yoffset = glGetUniformLocation(g_program_spherique, "yoffset");
					glUniform1f(yoffset, g_yoffset_spherique );

					GLuint invert = glGetUniformLocation(g_program_spherique, "invert");
					glUniform1i(invert,i);

					GLuint maxSize = glGetUniformLocation(g_program_spherique, "molMaxSize");

					GLuint projType = glGetUniformLocation(g_program_spherique, "projType");
					glUniform1i(projType,g_mol_state->_ProjType);

					g_renderer->setDrawRepere(false);
					glDisable(GL_CULL_FACE);

					//On en rend qu'une seule, en fonction du mode
					if(num_pass == 0 && i == 0)
					{
						float maxDim;
						g_moleculesManager->_Molecules[i]->getMaxDim(maxDim);
						glUniform1f(maxSize,maxDim);
						g_moleculesManager->_Molecules[i]->render();
					}
					if(num_pass == 1 && i == 1)
					{
						float maxDim;
						g_moleculesManager->_Molecules[i]->getMaxDim(maxDim);
						glUniform1f(maxSize,maxDim);
						g_moleculesManager->_Molecules[i]->render();
					}
				}

				if(num_pass == 2)
				{
					if(i==0)
					{
						glUseProgram(g_program_spherique_mix);
						g_renderer->sendPassTexsToShader(0,false,"colorTex1",0,"depthTex1",1);
						g_renderer->sendPassTexsToShader(1,false,"colorTex2",2,"depthTex2",3);
						g_renderer->renderFullScreenQuad();
					}
				}
			}
		}
	}

	glUseProgram(0);

	if(g_mol_state->_RenderMode == 1)
	{
		//reste du mol state a part les molecules
		if(g_mol_state->_Active)
			g_mol_state->render();

		if(!g_hideShipScreenShot)
		{
			g_ship_manager->render();
			g_proj_manager->render();
		}

		//rendu des grappins (a la fin car transparent)
		if(g_moleculesManager->_Molecules.size() >= 1)
			g_moleculesManager->_Molecules[0]->renderGrappins();
		if(g_moleculesManager->_Molecules.size() >= 2)
			g_moleculesManager->_Molecules[1]->renderGrappins();

		g_moleculesManager->renderTransparent();
	}
}

void update(void)
{
	float elapsed = g_timer->getElapsedSeconds(true);
	runApplication(elapsed);
	g_renderer->render(elapsed);
}

void resizeFunction(int width, int height)
{
	glViewport(0, 0, width, height);
	g_renderer->resize(width,height);
	g_pbar_acquire->setPos(g_renderer->_ScreenWidth/2-100,20);
	g_pbar_loading->setPos(g_renderer->_ScreenWidth/2-100,g_renderer->_ScreenHeight/2-15);
	g_label_load->X = g_renderer->_ScreenWidth/2 - g_label_load->Width/2;
	g_label_load->Y = g_pbar_loading->Y+ g_pbar_loading->Height+1;
	g_pbar_optim->setPos(g_renderer->_ScreenWidth/2-100,g_renderer->_ScreenHeight/2-15);
	g_pbar_score->setPos(g_renderer->_ScreenWidth/2-g_renderer->_ScreenWidth/6,20);
	g_pbar_score->setSize(g_renderer->_ScreenWidth/3,40);
	g_pbar_charge->setPos(g_renderer->_ScreenWidth/2-g_renderer->_ScreenWidth/6,20);
	g_pbar_charge->setSize(g_renderer->_ScreenWidth/3,40);
	g_pbar_best_score->setPos(g_renderer->_ScreenWidth/2-g_renderer->_ScreenWidth/6,20);
	g_pbar_best_score->setSize(g_renderer->_ScreenWidth/3,40);
	g_lbl_best_login->X = g_pbar_score->X;
	g_lbl_best_login->Y = g_pbar_score->Y + g_pbar_score->Height + 5;
	g_cbox_mol1->setPos(g_renderer->_ScreenWidth/2-210,g_renderer->_ScreenHeight/2-15,200,10);
	g_cbox_mol2->setPos(g_renderer->_ScreenWidth/2+10,g_renderer->_ScreenHeight/2-15,200,10);
	g_bouton_dock->X = g_renderer->_ScreenWidth/2 - 50;
	g_bouton_dock->Y = g_cbox_mol2->Y + 50;
	EdtLogin->X = g_renderer->_ScreenWidth/2-EdtLogin->Width / 2;
	EdtLogin->Y = g_renderer->_ScreenHeight/2-50;
	EdtPass->X = EdtLogin->X;
	EdtPass->Y = EdtLogin->Y + EdtLogin->Height+5;
	BtnLogin->X = g_renderer->_ScreenWidth/2-BtnLogin->Width/2;
	BtnLogin->Y = EdtPass->Y + EdtPass->Height+5;
	g_lbl_url_udock->Y = BtnLogin->Y + 50;
	g_lbl_url_udock->X = g_renderer->_ScreenWidth/2 - g_lbl_url_udock->Width/2;
	g_lbl_welcome->Y = EdtLogin->Y -40;
	g_lbl_welcome->X = g_renderer->_ScreenWidth/2 - g_lbl_welcome->Width/2;
	g_lbl_version->Y = BtnLogin->Y +30;
	g_lbl_version->X = g_renderer->_ScreenWidth/2 - g_lbl_version->Width/2;
	g_lbl_best_player->Y = BtnLogin->Y+90;
	g_lbl_best_player->X = g_renderer->_ScreenWidth/2 - g_lbl_best_player->Width/2;
	g_lbl_log->X = 10;
	g_lbl_log->Y = g_renderer->_ScreenHeight - g_lbl_log->Height-5;
	g_lst_scores->setMaxElementsFromHeight(g_renderer->_ScreenHeight - g_lst_scores->Y);
	g_loading->setPos(g_renderer->_ScreenWidth/2.0f - g_loading->Width/2.0f, g_renderer->_ScreenHeight/2.0f - g_loading->Height/2.0f);
	g_lbl_beated_names->Y = g_renderer->_ScreenHeight/2+50;
	g_lbl_beated_names->X = 0;
	g_lbl_beated_names->Width = g_renderer->_ScreenWidth;
	Tutorial::getInstance()->resize();
}

void setLights(int num_pass)
{
	//On active la light 0
	glEnable(GL_LIGHT0);

	//On définit une lumière directionelle (un soleil)
	float direction[4] = { g_renderer->_Camera->_Position.X, g_renderer->_Camera->_Position.Y, g_renderer->_Camera->_Position.Z, 1 }; ///w = 1 donc elle est positionee
	glLightfv(GL_LIGHT0, GL_POSITION, direction );
	float color[4] = {0.2f,0.2f,0.2f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, color );
	float color2[4] = {0.3f,0.3f,0.3f};
	glLightfv(GL_LIGHT0, GL_AMBIENT, color2 );

	g_renderer->setSunShaftSource(g_renderer->_Camera->_Position + NYVert3Df(0, 5000, 5000));

	glEnable(GL_LIGHT1);

	//On définit une lumière directionelle (un soleil)
	float direction2[4] = { 0, 5000, 5000, 0 }; ///w = 0 donc elle est a l'infini
	glLightfv(GL_LIGHT1, GL_POSITION, direction2);
	float color3[4] = { 0.5f, 0.5f, 0.5f };
	glLightfv(GL_LIGHT1, GL_DIFFUSE, color3);
	float color4[4] = { 0.3f, 0.3f, 0.3f };
	glLightfv(GL_LIGHT1, GL_AMBIENT, color4);

	
}

//////////////////////////////////////////////////////////////////////////
// GESTION JOYSTICK
//////////////////////////////////////////////////////////////////////////

void checkJoysticks()
{
	if (g_gamepad != NULL)
		return;

	
	if(SDL_NumJoysticks() > 0) {
		g_gamepad = SDL_JoystickOpen(0);
		Log::log(Log::USER_INFO, ("Nb joysticks found: " + toString(SDL_NumJoysticks())).c_str());
		Log::log(Log::USER_INFO, ("Joystick nb axis (6?): " + toString(SDL_JoystickNumAxes(g_gamepad))).c_str()); 
		Log::log(Log::USER_INFO, ("Joystick nb buttons (?): " + toString(SDL_JoystickNumButtons(g_gamepad))).c_str());
	}
}

void clickBtnSwitch(GUIBouton * bouton);
void clickBtnFreeze(GUIBouton * bouton);

void pollGamePad()
{
	static bool bselect_last = false;
	static bool bljoy_last = false;
	if (g_gamepad == NULL)
		return;
	
	float lx = SDL_JoystickGetAxis(g_gamepad, 0) / 32768.0;
	float ly = SDL_JoystickGetAxis(g_gamepad, 1) / 32768.0;
	float rx = SDL_JoystickGetAxis(g_gamepad, 2) / 32768.0;
	float ry = SDL_JoystickGetAxis(g_gamepad, 3) / 32768.0;
	float ltrig = ((SDL_JoystickGetAxis(g_gamepad, 4) / 32768.0)+1.0)/ 2.0;
	float rtrig = ((SDL_JoystickGetAxis(g_gamepad, 5) / 32768.0) + 1.0) / 2.0;
	bool bselect = SDL_JoystickGetButton(g_gamepad, 5) == 1;
	bool bljoy = SDL_JoystickGetButton(g_gamepad, 6) == 1;
	bool brjoy = SDL_JoystickGetButton(g_gamepad, 7) == 1;
	bool bl = SDL_JoystickGetButton(g_gamepad, 8) == 1;	
	bool br = SDL_JoystickGetButton(g_gamepad, 9) == 1;
	bool ba = SDL_JoystickGetButton(g_gamepad, 10) == 1;
	bool bb = SDL_JoystickGetButton(g_gamepad, 11) == 1;
	bool bx = SDL_JoystickGetButton(g_gamepad, 12) == 1;
	bool by = SDL_JoystickGetButton(g_gamepad, 13) == 1;
	bool bxbox = SDL_JoystickGetButton(g_gamepad, 14) == 1;
	bool bstart = SDL_JoystickGetButton(g_gamepad, 15) == 1;

	//_cprintf("%d %d %d %d : %d %d : %d %d : %d %d %d : %0.1f %0.1f \n", ba, bb, bx, by, bljoy, brjoy, bl, br, bselect, bxbox, bstart, lx, ltrig);

	if (bselect && !bselect_last)
		clickBtnSwitch(NULL);

	if (bljoy && !bljoy_last)
		clickBtnFreeze(NULL);

	if (g_mol_state->_Active)
		g_mol_state->gamePadFunction(NYVert3Df(lx, ly, 0), NYVert3Df(rx, ry, 0), ltrig, rtrig, bljoy, brjoy, bl, br, ba, bb, bx, by, bxbox);

	if (g_ship_state->_Active)
		g_ship_state->gamePadFunction(NYVert3Df(lx, ly, 0), NYVert3Df(rx, ry, 0), ltrig, rtrig, bljoy, brjoy, bl, br, ba, bb, bx, by, bxbox);

	bselect_last = bselect;
	bljoy_last = bljoy;
}

//////////////////////////////////////////////////////////////////////////
// GESTION CLAVIER SOURIS
//////////////////////////////////////////////////////////////////////////


//Interaction Utilisateur
void keyboard_key(int key, bool down)
{
	if(g_screen_manager->specialKeyCallback(key,down,0))
		return;

	if(key == SDL_SCANCODE_ESCAPE)
		g_termine = true;

	if(down)
	{
		if(g_mol_state->_Active)
			g_mol_state->specialDownFunction(key,0,0);

		if(g_ship_state->_Active)
			g_ship_state->specialDownFunction(key,0,0);

		if(key == SDL_SCANCODE_F)
		{
			NYRenderer::getInstance()->toggleFullScreen();
		}

		if(key == SDL_SCANCODE_F12)
		{
			g_program_mol = g_renderer->createProgram("shaders/psmol.glsl","shaders/vsmol.glsl");
			g_program_spherique = g_renderer->createProgram("shaders/sphere/psmol.glsl","shaders/sphere/vsmol.glsl","shaders/sphere/gsmol.glsl");
			g_program_spherique_mix = g_renderer->createProgram("shaders/sphere/sphere_mix/pshader.glsl");
			g_program_smooth_proj  = g_renderer->createProgram("shaders/smooth/proj/ps.glsl","shaders/smooth/proj/vs.glsl","shaders/smooth/proj/gs.glsl");
			g_program_smooth_tex =  g_renderer->createProgram("shaders/smooth/smooth/pshader.glsl");
			g_program_smooth_smooth  = g_renderer->createProgram("shaders/smooth/final/ps.glsl","shaders/smooth/final/vs.glsl","shaders/smooth/final/gs.glsl");
			g_renderer->reloadInternShader();
		}

		if(key == SDL_SCANCODE_LEFT)
			g_rotatex_spherique = -1;
		if(key == SDL_SCANCODE_RIGHT)
			g_rotatex_spherique = +1;
		if(key == SDL_SCANCODE_UP)
		{
			g_rotatey_spherique = +1;
			g_mol_state->_Precise = false;
		}
		if(key == SDL_SCANCODE_DOWN)
		{
			g_rotatey_spherique = -1;
			g_mol_state->_Precise = false;
		}

		if(g_mol_state->_Active)
			g_mol_state->keyboardDownFunction(key,0,0);

		if(g_ship_state->_Active)
			g_ship_state->keyboardDownFunction(key,0,0);
	}
	else
	{


		if(key == SDL_SCANCODE_LEFT && g_rotatex_spherique == -1)
			g_rotatex_spherique = 0;
		if(key == SDL_SCANCODE_RIGHT && g_rotatex_spherique == 1)
			g_rotatex_spherique = 0;
		if(key == SDL_SCANCODE_UP && g_rotatey_spherique == 1)
		{
			g_rotatey_spherique = 0;
			g_mol_state->_Precise = true;
		}
		if(key == SDL_SCANCODE_DOWN && g_rotatey_spherique == -1)
		{
			g_rotatey_spherique = 0;
			g_mol_state->_Precise = true;
		}

		if(g_mol_state->_Active)
			g_mol_state->keyboardUpFunction(key,0,0);

		if(g_ship_state->_Active)
			g_ship_state->keyboardUpFunction(key,0,0);

		if(g_mol_state->_Active)
			g_mol_state->specialUpFunction(key,0,0);

		if(g_ship_state->_Active)
			g_ship_state->specialUpFunction(key,0,0);
	}

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
		//mouseTraite = g_screen_manager->mouseCallback(x,y,g_mouse_btn_gui_state,0,0);
		mouseTraite = g_screen_manager->mouseCallback(evt.motion.x,evt.motion.y,g_mouse_btn_gui_state,0,0);

		if((evt.motion.state & SDL_BUTTON(1)) && g_screen_manager->_ActiveScreen == g_screen_params)
		{
			#ifdef TOTAL_DECO
			g_sound_grain->setGrainParam((float) g_slider_pos->Value, (float) g_slider_size->Value, (float) g_slider_random->Value, (float) g_slider_overlap->Value);
			#endif
			g_sound_ambiance->setVolume((float) g_slider_volume_ambiance->Value);
			g_ambient_level = (float)g_slider_ambient_light->Value;
			g_taille_cerne = (float)g_slider_cerne->Value;

			float h,s,v,a;
			g_color_positif.toHSV(&h,&s,&v,&a);

			float h2,s2,v2,a2;
			g_color_negatif.toHSV(&h2,&s2,&v2,&a2);

			float h3,s3,v3,a3;
			g_color_fond.toHSV(&h3,&s3,&v3,&a3);

			g_color_positif.fromHSV(g_slider_color_pos_h->Value,g_slider_color_pos_s->Value,g_slider_color_pos_v->Value,a);
			g_color_negatif.fromHSV(g_slider_color_neg_h->Value,g_slider_color_neg_s->Value,g_slider_color_neg_v->Value,a2);
			g_color_fond.fromHSV(g_slider_color_fond_h->Value,g_slider_color_fond_s->Value,g_slider_color_fond_v->Value,a2);
			g_renderer->setBackgroundColor(g_color_fond);

			#ifdef TOTAL_DECO
			g_lbl_size->Text = std::string("Size : ") + toString( g_slider_size->Value);
			g_lbl_pos->Text = std::string("Pos : ") + toString( g_slider_pos->Value);
			g_lbl_random->Text = std::string("Random : ") + toString( g_slider_random->Value);
			g_lbl_overlap->Text = std::string("Overlap : ") + toString( g_slider_overlap->Value);
			#endif
		}
	}

	if(mouseTraite)
		return;

	if(g_mol_state->_Active)
		g_mol_state->mouseMoveFunction(evt.motion.state,evt.motion.x,evt.motion.y);

	if(g_ship_state->_Active)
		g_ship_state->mouseMoveFunction(evt.motion.state,evt.motion.x,evt.motion.y);
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

	if(mouseTraite && down)
		g_sounds_attach[rand()%NB_SOUND_ATTACH]->play();
	if(mouseTraite)
	{
		if(g_screen_manager->_ActiveScreen == g_screen_params)
		{
			#ifdef TOTAL_DECO
			g_sound_grain->setGrainParam((float) g_slider_pos->Value,(float) g_slider_size->Value,(float) g_slider_random->Value,(float) g_slider_overlap->Value);
			#endif

			g_sound_ambiance->setVolume((float) g_slider_volume_ambiance->Value);
			g_ambient_level = (float)g_slider_ambient_light->Value;
			g_taille_cerne = (float)g_slider_cerne->Value;

			float h,s,v,a;
			g_color_positif.toHSV(&h,&s,&v,&a);

			float h2,s2,v2,a2;
			g_color_negatif.toHSV(&h2,&s2,&v2,&a2);

			float h3,s3,v3,a3;
			g_color_fond.toHSV(&h3,&s3,&v3,&a3);

			g_color_positif.fromHSV(g_slider_color_pos_h->Value,g_slider_color_pos_s->Value,g_slider_color_pos_v->Value,a);
			g_color_negatif.fromHSV(g_slider_color_neg_h->Value,g_slider_color_neg_s->Value,g_slider_color_neg_v->Value,a2);
			g_color_fond.fromHSV(g_slider_color_fond_h->Value,g_slider_color_fond_s->Value,g_slider_color_fond_v->Value,a2);
			g_renderer->setBackgroundColor(g_color_fond);

			#ifdef TOTAL_DECO
			g_lbl_size->Text = std::string("Size : ") + toString( g_slider_size->Value);
			g_lbl_pos->Text = std::string("Pos : ") + toString( g_slider_pos->Value);
			g_lbl_random->Text = std::string("Random : ") + toString( g_slider_random->Value);
			g_lbl_overlap->Text = std::string("Overlap : ") + toString( g_slider_overlap->Value);
			#endif
		}
	}

	if(mouseTraite)
		return;

	if(g_mol_state->_Active)
		g_mol_state->mouseFunction(evt.button.button,down,evt.button.x,evt.button.y);

	if(g_ship_state->_Active)
		g_ship_state->mouseFunction(evt.button.button,down,evt.button.x,evt.button.y);

	if(mouseTraite)
		return;




}

void mouse_wheel(SDL_Event & evt)
{
	//Log::log(Log::ENGINE_INFO,"mouse wheel");
	if(g_screen_manager->mouseCallback(g_mouse_position.X,g_mouse_position.Y,0,evt.wheel.y,0))
		return;

	if(g_mol_state->_Active)
		g_mol_state->mouseWheelFunction(1,evt.wheel.y,g_mouse_position.X,g_mouse_position.Y);

	if(g_ship_state->_Active)
		g_ship_state->mouseWheelFunction(1,evt.wheel.y,g_mouse_position.X,g_mouse_position.Y);
}

//Old Input

void mousePollFunction(float elapsed)
{
	static int lastx = -1;
	static int lasty = -1;
	static float elaspedSinceLastMove = 0;
	elaspedSinceLastMove += elapsed;
	
	if(g_mol_state->_Active)
		g_mol_state->mousePollFunction(elapsed);

	int x = 0;
	int y = 0;

	SDL_GetMouseState(&x, &y);
	if (x != lastx || y != lasty)
	{
		SDL_ShowCursor(true);
		elaspedSinceLastMove = 0;
	}

	if (elaspedSinceLastMove > 1.0)
		SDL_ShowCursor(false);

	lastx = x;
	lasty = y;
}


void clickBtnFreeze(GUIBouton * bouton)
{
	if(g_mol_state->_Active)
	{
		if(g_moleculesManager->_Molecules[0]->_Body->Frozen)
		{
			g_mol_state->freezeMols(false);
			//bouton->Titre = "Freeze";
		}
		else
		{
			g_mol_state->freezeMols(true);
			//bouton->Titre = "Unfreeze";
		}
	}

	if(g_ship_state->_Active)
	{
		if(g_moleculesManager->_Molecules[0]->_Body->Frozen)
		{
			g_ship_state->freezeMols(false);
			//bouton->Titre = "Freeze";
		}
		else
		{
			g_ship_state->freezeMols(true);
			//bouton->Titre = "Unfreeze";
		}
	}

}

void clickBtnOptimize(GUIBouton * bouton)
{
	if(g_mol_state->_Active)
		g_mol_state->startOptimisation(5000.0f);
}

void clickBtnTest (GUIBouton * bouton)
{
	g_screen_manager->setActiveScreen(g_screen_params);
}

void clickBtnHelp (GUIBouton * bouton)
{
	g_screen_manager->setActiveScreen(g_screen_controls);
}

void clickBtnCredits (GUIBouton * bouton)
{
	g_screen_manager->setActiveScreen(g_screen_credits);
}

void clickBtnSwitch (GUIBouton * bouton)
{
	g_mol_state->activate(!g_mol_state->_Active);
	g_ship_state->activate(!g_ship_state->_Active);
}

void clickBtnCloseParam (GUIBouton * bouton)
{
	g_screen_manager->setActiveScreen(g_screen_jeu);
}

void clickBtnCloseCredits (GUIBouton * bouton)
{
	g_screen_manager->setActiveScreen(g_screen_login);
}

void clickBtnCloseHelp (GUIBouton * bouton)
{
	g_screen_manager->setActiveScreen(g_screen_jeu);
}

void clickBtnLoad(GUIBouton * bouton)
{
	if(g_choose_mol_state->_Active)
		return;
	g_mol_state->activate(false);
	g_ship_state->activate(false);
	g_choose_mol_state->activate(true);
}

void clickBtnPlayGrain (GUIBouton * bouton)
{
	if(g_sound_grain->isPlaying())
	{
		g_sound_grain->stop();
		bouton->Titre = "Play Grains";
	}
	else
	{
		g_sound_grain->play();
		bouton->Titre = "Stop Grains";
	}
}

void clickAnalyseLogs(GUIBouton * bouton)
{
	/*WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(L"logs\\*.bin", &ffd);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		Log::log(Log::ENGINE_ERROR,"Logs directory does not exists, or no *.bin inside");
		return;
	}

	LogActionsReader * reader = new LogActionsReader();

	std::vector<string> files;

	do
	{
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			char ch[260];
			char DefChar = ' ';
			WideCharToMultiByte(CP_ACP,0,ffd.cFileName,-1, ch,260,&DefChar, NULL);
			files.push_back(std::string(ch));

		}
	}
	while (FindNextFile(hFind, &ffd) != 0);

	for(int i=0;i<files.size();i++)
		reader->addFile("logs\\" + files[i]);

	reader->analyze();

	SAFEDELETE(reader);*/
}

void clickGetBestComplexes(GUIBouton * bouton)
{
	float score = 0.0f;
	if(g_mol_state->_Active)
		score = g_mol_state->_CurrentEnergy;

	std::vector<HttpManagerUdock::HttpComplexStruct> complexes;
	HttpManagerUdock::getInstance()->getBestComplexes(&complexes);

	MoleculeCubes * mols[2];
	mols[0] = new MoleculeCubes();
	mols[1] = new MoleculeCubes();

	for(unsigned int i=0;i<complexes.size();i++)
	{
		Log::log(Log::USER_INFO,("Exporting complex " + toString(i+1) + " on " + toString(complexes.size())).c_str());

		mols[0]->loadFromMOL2((L"molecules/"+toWString(complexes[i].id1)+L".mol2").c_str());
		mols[1]->loadFromMOL2((L"molecules/"+toWString(complexes[i].id2)+L".mol2").c_str());

		/*mols[0]->saveToPDB(("export/"+
			toString(i) + "_" +
			toString(complexes[i].iduser) + "_" +
			toString(complexes[i].id1) + "TEST" +
			".pdb").c_str());*/

		mols[0]->saveComplexToPDB(("export/"+
			toString(i) + "_" +
			toString(complexes[i].iduser) + "_" +
			toString(complexes[i].id1) + "_" +
			toString(complexes[i].id2) + "_base" +
			".pdb").c_str(),mols[1], true);

		mols[0]->_Transform = complexes[i].pos1;
		mols[1]->_Transform = complexes[i].pos2;

		/*mols[0]->saveToMOL2(("export/"+
								 toString(i) + "_" +
								 toString(complexes[i].iduser) + "_" +
								 toString(complexes[i].id1) + "_" +
								 toString(complexes[i].id2) + "_" +
								 "a" +
								".mol2").c_str());
		mols[1]->saveToMOL2(("export/"+
								toString(i) + "_" +
								toString(complexes[i].iduser) + "_" +
								toString(complexes[i].id1) + "_" +
								toString(complexes[i].id2) + "_" +
								"b" +
								".mol2").c_str());
		mols[0]->savePosition(("export/"+
								toString(i) + "_" +
								toString(complexes[i].iduser) + "_" +
								toString(complexes[i].id1) + "_" +
								toString(complexes[i].id2) + "_" +
								"a" +
								".pos").c_str());
		mols[1]->savePosition(("export/"+
								toString(i) + "_" +
								toString(complexes[i].iduser) + "_" +
								toString(complexes[i].id1) + "_" +
								toString(complexes[i].id2) + "_" +
								"b" +
								".pos").c_str());*/
		mols[0]->saveComplexToPDB(("export/"+
									toString(i) + "_" +
									toString(complexes[i].iduser) + "_" +
									toString(complexes[i].id1) + "_" +
									toString(complexes[i].id2) +
									".pdb").c_str(),mols[1],score);
	}

	Log::log(Log::USER_INFO,"Export of best complexes done");

	SAFEDELETE(mols[0]);
	SAFEDELETE(mols[1]);

}

void clickGetBestComplex(GUIBouton * bouton)
{

	float score = 0.0f;
	if(g_mol_state->_Active)
		score = g_mol_state->_CurrentEnergy;

	std::vector<HttpManagerUdock::HttpComplexStruct> complexes;
	HttpManagerUdock::getInstance()->getBestComplexes(&complexes, PlayerInfo::getInstance()->_IdUser);

	MoleculeCubes * mols[2];
	mols[0] = new MoleculeCubes();
	mols[1] = new MoleculeCubes();

	for(unsigned int i=0;i<complexes.size();i++)
	{
		Log::log(Log::USER_INFO,("Exporting complex " + toString(i+1) + " on " + toString(complexes.size())).c_str());

		mols[0]->loadFromMOL2((L"molecules/"+toWString(complexes[i].id1)+L".mol2").c_str());
		mols[1]->loadFromMOL2((L"molecules/"+toWString(complexes[i].id2)+L".mol2").c_str());

		mols[0]->_Transform = complexes[i].pos1;
		mols[1]->_Transform = complexes[i].pos2;

		mols[0]->saveComplexToPDB(("export/"+
									toString(i) + "_" +
									toString(complexes[i].iduser) + "_" +
									toString(complexes[i].id1) + "_" +
									toString(complexes[i].id2) +
									".pdb").c_str(),mols[1],score);
	}

	Log::log(Log::USER_INFO,"Export of best complex done");

	SAFEDELETE(mols[0]);
	SAFEDELETE(mols[1]);

}

void clickExportPose(GUIBouton * bouton)
{
	float score = 0.0f;
	if(g_mol_state->_Active)
		score = g_mol_state->_CurrentEnergy;

	//Recup heure
	namespace pt = boost::posix_time;
	pt::ptime now = pt::second_clock::local_time();

	//SYSTEMTIME date;
	//GetSystemTime(&date);

	string name = g_moleculesManager->_Molecules[0]->_MolName + "_" +
		g_moleculesManager->_Molecules[1]->_MolName + "_" +
		toString(now.date().year()) + "_" +
		toString(now.date().month()) + "_" +
		toString(now.date().day()) + "_" +
		toString(now.time_of_day().hours()) + "_" +
		toString(now.time_of_day().minutes()) + "_" +
		toString(now.time_of_day().seconds());

	g_moleculesManager->_Molecules[0]->saveComplexToPDB(("export/"+
		name + ".pdb").c_str(),g_moleculesManager->_Molecules[1],score);

	//ScreenShot
	//g_hideShipScreenShot = true;
	g_hideInterfaceScreenShot = true;


	//A cause de la possible syncho vert, on va patienter que le fb soit bien a jour
	NYTimer timer;
	while (timer.getElapsedSeconds() < 0.3f)
		update();
	NYRenderer::getInstance()->snapshot(("export/"+name+".png").c_str());

	g_hideShipScreenShot = false;
	g_hideInterfaceScreenShot = false;
}

void clickGenerate(GUIBouton * bouton)
{
	MoleculeCubes * mol = new MoleculeCubes();

	int sizeCote = 10;

	mol->_NbAtomes = sizeCote*sizeCote*sizeCote;
	mol->_Atomes = new Atome[sizeCote*sizeCote*sizeCote];
	mol->_MolName = "Custom";
	mol->_MolType = "Custom";
	mol->_ChargeType = "CustomCharges";


	Mol2Atom * atom = new Mol2Atom();
	atom->_Name[0] = 'C';
	atom->_Name[1] = 0;
	atom->_Type[0] = 'C';
	atom->_Type[1] = '.';
	atom->_Type[2] = '2';
	atom->_Type[3] = 0;
	atom->_SubstName[0] = 'V';
	atom->_SubstName[1] = 'A';
	atom->_SubstName[2] = 'L';
	atom->_SubstName[3] = 0;
	atom->_SubstId = 1;

	mol->_NbAtomes = 0;
	for(int x=0;x<sizeCote;x++)
		for(int y=0;y<sizeCote;y++)
			for(int z=0;z<sizeCote;z++)
			{
				if(x>=5)
					if(y<3 || y >6 || z < 3 || z > 6)
						continue;

				atom->_SerialNumber=  mol->_NbAtomes + 1;
				if(x>=5)
					atom->_Charge = -0.15;
				else
					atom->_Charge =  0.05;
				atom->_Position.X = x*4.0f;
				atom->_Position.Y = y*4.0f;
				atom->_Position.Z = z*4.0f;
				mol->_Atomes[mol->_NbAtomes].buildFromMol2Atom(atom);
				mol->_NbAtomes++;
			}

	mol->saveToMOL2("13.mol2");

	mol->_NbAtomes = 0;
	for(int x=0;x<sizeCote;x++)
		for(int y=0;y<sizeCote;y++)
			for(int z=0;z<sizeCote;z++)
			{
				if(x>=5)
					if(y>2 && y <7 && z > 2 && z < 7)
						continue;

				atom->_SerialNumber=  mol->_NbAtomes + 1;
				if(x>=5)
					atom->_Charge =  0.06;
				else
					atom->_Charge =  -0.05;

				atom->_Position.X = x*4.0f;
				atom->_Position.Y = y*4.0f;
				atom->_Position.Z = z*4.0f;

				mol->_Atomes[mol->_NbAtomes].buildFromMol2Atom(atom);
				mol->_NbAtomes++;
			}

	mol->saveToMOL2("14.mol2");

	/*mol->_NbAtomes = 0;
	for(int x=0;x<sizeCote;x++)
		for(int y=0;y<sizeCote;y++)
			for(int z=0;z<sizeCote;z++)
			{

				atom->_SerialNumber=  mol->_NbAtomes + 1;
				if(x>=5)
					atom->_Charge =  0.1;
				else
					atom->_Charge =  -0.1;

				atom->_Position.X = x*3.0f;
				atom->_Position.Y = y*3.0f;
				atom->_Position.Z = z*3.0f;

				mol->_Atomes[mol->_NbAtomes].buildFromMol2Atom(atom);
				mol->_NbAtomes++;
			}

			mol->saveToMOL2("15.mol2");*/

	mol->_NbAtomes = 0;
	atom->_Charge = 0;
	for(int x=0;x<sizeCote;x++)
		for(int y=0;y<sizeCote;y++)
			for(int z=0;z<sizeCote;z++)
			{
				if(x>=5)
					if(y<3 || y >6 || z < 3 || z > 6)
						continue;

				atom->_SerialNumber=  mol->_NbAtomes + 1;
				atom->_Position.X = x*4.0f;
				atom->_Position.Y = y*4.0f;
				atom->_Position.Z = z*4.0f;
				mol->_Atomes[mol->_NbAtomes].buildFromMol2Atom(atom);
				mol->_NbAtomes++;
			}

	mol->saveToMOL2("15.mol2");

	mol->_NbAtomes = 0;
	for(int x=0;x<sizeCote;x++)
		for(int y=0;y<sizeCote;y++)
			for(int z=0;z<sizeCote;z++)
			{
				if(x>=5)
					if(y>2 && y <7 && z > 2 && z < 7)
						continue;

				atom->_SerialNumber=  mol->_NbAtomes + 1;

				atom->_Position.X = x*4.0f;
				atom->_Position.Y = y*4.0f;
				atom->_Position.Z = z*4.0f;

				mol->_Atomes[mol->_NbAtomes].buildFromMol2Atom(atom);
				mol->_NbAtomes++;
			}

	mol->saveToMOL2("16.mol2");

	Log::log(Log::USER_INFO,"Dummies generated");
}

void clickObj(GUIBouton * bouton)
{

}


/**
  * POINT D'ENTREE PRINCIPAL
  **/

#ifdef MSVC
#include <tchar.h>
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char* argv[])
#endif
{
	//Checks pour udock
	BOOST_STATIC_ASSERT_MSG(sizeof(float) == 4,"Float is not 4 bytes, we will have troubles with binary files (logs, ship file)");
	BOOST_STATIC_ASSERT_MSG(sizeof(int) == 4,"int is not 4 bytes, we will have troubles with binary files (logs, ship file)");

	//Creation du log
	Log::showEngineLog(true);
	Log::addLog(new LogConsole());
	Log::addLog(new LogFile());
	Log::addLog(new LogScreen());


	bool gameMode = true;
	bool adminUi = false;
	bool vaisseauUI = true;
	bool fullLog = false;
	bool public_disco = false;
	bool total_disco = false;
	bool feedbacks = true;
	for(int i=0;i<argc;i++)
	{
		if(argv[i][0] == 'w')
		{
			Log::log(Log::ENGINE_INFO,"arg w asking window mode");
			gameMode = false;
		}

		if(argv[i][0] == 'l')
		{
			Log::log(Log::ENGINE_INFO,"arg l asking to show full log");
			fullLog = true;
		}

		if(argv[i][0] == 'a')
		{
			Log::log(Log::ENGINE_INFO,"arg a asking for admin ui");
			adminUi = true;
		}

		if(argv[i][0] == 'i')
		{
			Log::log(Log::ENGINE_INFO,"arg asking for color inversion");
			g_invert_polarity = true;
		}

		if(argv[i][0] == 'v')
		{
			Log::log(Log::ENGINE_INFO,"arg asking for no ship mode");
			vaisseauUI = false;
		}

		if(argv[i][0] == 'f')
		{
			Log::log(Log::ENGINE_INFO,"arg asking for no feedbacks");
			feedbacks = false;
		}

		if(argv[i][0] == 'd')
		{
			Log::log(Log::ENGINE_INFO, "arg d asking for public disconect");
			public_disco = true;
					Log::log(Log::ENGINE_INFO, "arg d asking for total disconected");
					total_disco = true;
		
		}

		if (argv[i][0] == 'x')
		{
			Log::log(Log::ENGINE_INFO, "arg d asking for total disconected");
			total_disco = true;
		}

	}

	Log::showEngineLog(fullLog);

	if(total_disco)
		HttpManagerTotalDeco::getInstance();
	else if(public_disco)
		HttpManagerPublicDeco::getInstance();

	//Param de base
	PlayerInfo::getInstance()->_ShowFeedBacks = feedbacks;

	int screen_width = 1280;
	int screen_height = 720;


	//INIT SDL

	SDL_Window * fenetre = NULL;
	SDL_GLContext contexteOpenGL = NULL;

	// Initialisation de la SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
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
	fenetre = SDL_CreateWindow("engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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

	//Initialisation du renderer
	g_renderer = NYRenderer::getInstance();
	g_renderer->setCam(new NYCameraAnimated());
	g_renderer->setRenderObjectFun(renderObjects);
	g_renderer->setRenderObjectDepthOnlyFun(renderObjectsDepthOnly);
	g_renderer->setRender2DFun(render2D);
	g_renderer->setLightsFun(setLights);
	g_renderer->initialise(fenetre,true);
	g_renderer->setBackgroundColor(g_color_fond);
	g_renderer->resize(screen_width,screen_height);
	if (gameMode)
		g_renderer->setFullScreen(true);


	Log::log(Log::ENGINE_INFO,("OpenGL version "+std::string((char*)glGetString(GL_VERSION))).c_str());

	//Creation du programme de rendu des molecules
	g_program_mol = g_renderer->createProgram("shaders/psmol.glsl","shaders/vsmol.glsl");
	g_program_spherique = g_renderer->createProgram("shaders/sphere/psmol.glsl","shaders/sphere/vsmol.glsl","shaders/sphere/gsmol.glsl");
	g_program_spherique_mix = g_renderer->createProgram("shaders/sphere/sphere_mix/pshader.glsl");
	g_program_smooth_proj  = g_renderer->createProgram("shaders/smooth/proj/ps.glsl","shaders/smooth/proj/vs.glsl","shaders/smooth/proj/gs.glsl");
	g_program_smooth_tex =  g_renderer->createProgram("shaders/smooth/smooth/pshader.glsl");
	g_program_smooth_smooth  = g_renderer->createProgram("shaders/smooth/final/ps.glsl","shaders/smooth/final/vs.glsl","shaders/smooth/final/gs.glsl");

	//On applique la config du renderer
	glViewport(0, 0, g_renderer->_ScreenWidth, g_renderer->_ScreenHeight);
	g_renderer->resize(g_renderer->_ScreenWidth,g_renderer->_ScreenHeight);

	//Init du moteur physique
	g_phyEngine = NYPhysicEngine::getInstance();
	g_phyEngine->initialisation(NYVert3Df(0,0,0));

	//Init du moteur son
	g_sound_engine = SoundEngine::getInstance();

	//Creation des modes de jeu
	g_mol_state = new MolState();
	g_ship_state = new ShipState();
	g_load_state = new LoadState();
	g_choose_mol_state = new ChooseMolState();

	if(public_disco || total_disco)
		g_login_state = new LoginAutoState();
	else
		g_login_state = new LoginState();

	g_load_state->_ShipState = g_ship_state;
	g_load_state->_MolState = g_mol_state;
	g_choose_mol_state->_LoadState = g_load_state;
	g_login_state->_ChooseMolState = g_choose_mol_state;

	g_mol_state->init();
	g_ship_state->init();
	g_load_state->init();
	g_choose_mol_state->init();
	g_login_state->init();

	// INIT GUI

	//Manager d'ecrans
	g_screen_manager = new GUIScreenManager();
	g_choose_mol_state->_ScreenManager = g_screen_manager;
	g_load_state->_ScreenManager = g_screen_manager;
	g_mol_state->_ScreenManager = g_screen_manager;
	g_ship_state->_ScreenManager = g_screen_manager;
	g_login_state->_ScreenManager = g_screen_manager;

	uint16 x = 10;
	uint16 y = 10;

	//Leader board
	g_lst_scores = new GUILstBox();
	g_lst_scores->DrawBorder = false;
	g_lst_scores->AlphaEffect = true;
	g_lst_scores->ShowScrollBar = false;
	g_lst_scores->CenterSelected = true;
	g_lst_scores->X = 10;
	g_lst_scores->Y = 10;
	g_lst_scores->Width = 500;
	g_lst_scores->setMaxElementsFromHeight(g_renderer->_ScreenHeight - g_lst_scores->Y);
	g_lst_scores->ColorSel = NYColor(1.0f,1.0f,1.0f,1.0f);
	g_choose_mol_state->_LstHighScores = g_lst_scores;

	//ECRAN DE LOG (toujours visible)
	g_screen_log = new GUIScreen();
	g_lbl_log = new GUILabel();
	g_screen_log->addElement(g_lbl_log);
	//LogScreen::setLabel(g_lbl_log);
	//LogScreen::getInstance()->_LabelLog = g_lbl_log;
	g_loading = new GUILoading();
	g_loading->loadImage(std::string("textures/loading.png"));
	g_screen_log->addElement(g_loading);
	g_loading->Visible = false;
	g_login_state->_Loading = g_loading;

	//Credits
	g_screen_credits = new GUIScreen();

	GUIBouton * btnClose = new GUIBouton();
	btnClose->Titre = std::string("Close");
	btnClose->X = x;
	btnClose->Y = y;
	btnClose->setOnClick(clickBtnCloseCredits);
	g_screen_credits->addElement(btnClose);

	y = 50;
	x = 50;

	GUILabel * lbl = new GUILabel();
	lbl->Text = "Game Design : ";
	lbl->X = x;	lbl->Y = y;	y+= lbl->Height;
	g_screen_credits->addElement(lbl);
	lbl = new GUILabel();
	lbl->Text = "Matthieu Montes & Guillaume Levieux";
	lbl->X = x + 10;	lbl->Y = y;	y+= lbl->Height;
	g_screen_credits->addElement(lbl);
	y+= lbl->Height;
	lbl = new GUILabel();
	lbl->Text = "Software : ";
	lbl->X = x;	lbl->Y = y;	y+= lbl->Height;
	g_screen_credits->addElement(lbl);
	lbl = new GUILabel();
	lbl->Text = "Guillaume Levieux";
	lbl->X = x + 10;	lbl->Y = y;	y+= lbl->Height;
	g_screen_credits->addElement(lbl);
	y+= lbl->Height;
	lbl = new GUILabel();
	lbl->Text = "Sound Design : ";
	lbl->X = x;	lbl->Y = y;	y+= lbl->Height;
	g_screen_credits->addElement(lbl);
	lbl = new GUILabel();
	lbl->Text = "Guillaume Tiger";
	lbl->X = x + 10;	lbl->Y = y;	y+= lbl->Height;
	g_screen_credits->addElement(lbl);

	//Controls
	g_screen_controls = new GUIScreen();

	x = 10;
	y = 10;

	btnClose = new GUIBouton();
	btnClose->Titre = std::string("Close");
	btnClose->X = x;
	btnClose->Y = y;
	btnClose->setOnClick(clickBtnCloseHelp);
	g_screen_controls->addElement(btnClose);

	y = 50;
	x = 50;

	lbl = new GUILabel();
	lbl->Text = "Move molecule : Left Click";
	lbl->X = x;	lbl->Y = y;	y+= lbl->Height;
	g_screen_controls->addElement(lbl);
	lbl = new GUILabel();
	lbl->Text = "Move camera : Right Click ";
	lbl->X = x;	lbl->Y = y;	y+= lbl->Height;
	g_screen_controls->addElement(lbl);
	lbl = new GUILabel();
	lbl->Text = "Add anchor on molecule : Ctrl + Left Click ";
	lbl->X = x;	lbl->Y = y;	y+= lbl->Height;
	g_screen_controls->addElement(lbl);
	lbl = new GUILabel();
	lbl->Text = "Remove anchor on molecule : Ctrl + Right Click ";
	lbl->X = x;	lbl->Y = y;	y+= lbl->Height;
	g_screen_controls->addElement(lbl);
	lbl = new GUILabel();
	lbl->Text = "Use anchors to dock molecules : Space Bar ";
	lbl->X = x;	lbl->Y = y;	y+= lbl->Height;
	g_screen_controls->addElement(lbl);
	lbl = new GUILabel();
	lbl->Text = "Optimize : F5 ";
	lbl->X = x;	lbl->Y = y;	y+= lbl->Height;
	g_screen_controls->addElement(lbl);
	lbl = new GUILabel();
	lbl->Text = "Reload last best pos and optimize : F6 ";
	lbl->X = x;	lbl->Y = y;	y+= lbl->Height;
	g_screen_controls->addElement(lbl);
	lbl = new GUILabel();
	lbl->Text = "Reset : r ";
	lbl->X = x;	lbl->Y = y;	y+= lbl->Height;
	g_screen_controls->addElement(lbl);
	lbl = new GUILabel();
	lbl->Text = "Fullscreen : f ";
	lbl->X = x;	lbl->Y = y;	y+= lbl->Height;
	g_screen_controls->addElement(lbl);



	//ECRAN DE LOGIN
	g_screen_login = new GUIScreen();
	g_login_state->_Screen = g_screen_login;

	BtnCredits = new GUIBouton();
	BtnCredits->Titre = std::string("Credits");
	BtnCredits->X = 10;
	BtnCredits->Y = 10;
	BtnCredits->setOnClick(clickBtnCredits);
	g_screen_login->addElement(BtnCredits);

	y += BtnCredits->Height + 10;

	EdtLogin = new GUIEdtBox();
	EdtLogin->Width = 150;
	g_screen_login->addElement(EdtLogin);
	g_login_state->_EdtLogin = EdtLogin;

	EdtPass = new GUIEdtBox();
	EdtPass->Width = 150;
	EdtPass->setPassword(true);
	g_screen_login->addElement(EdtPass);
	g_login_state->_EdtPass = EdtPass;

	BtnLogin = new GUIBouton();
	BtnLogin->Titre = "login";
	g_screen_login->addElement(BtnLogin);
	g_login_state->_BtnLogin = BtnLogin;

	g_lbl_url_udock = new GUILabel();
	g_lbl_url_udock->Width = 400;
	g_lbl_url_udock->Text = "http://udock.fr";
	g_lbl_url_udock->Centering = true;
	g_screen_login->addElement(g_lbl_url_udock);

	g_lbl_welcome = new GUILabel();
	g_lbl_welcome->Width = 400;
	g_lbl_welcome->Text = "Welcome to UDock " + toString(UDOCK_VERSION);
	g_lbl_welcome->Centering = true;
	g_screen_login->addElement(g_lbl_welcome);

	g_lbl_version = new GUILabel();
	g_lbl_version->Width = 400;
	g_lbl_version->Centering = true;
	g_lbl_version->Text = "";
	g_login_state->_LblVersion = g_lbl_version;
	g_screen_login->addElement(g_lbl_version);

	g_lbl_best_player= new GUILabel();
	g_lbl_best_player->Width = 700;
	g_lbl_best_player->Centering = true;
	g_lbl_best_player->Text = "";
	g_login_state->_LblBestPlayer = g_lbl_best_player;
	g_screen_login->addElement(g_lbl_best_player);

	//ECRAN DE CHARGEMENT
	g_screen_load_mols = new GUIScreen();
	g_load_state->_Screen = g_screen_load_mols;

	//PBar de chargement
	g_pbar_loading = new GUIPBar();
	g_pbar_loading->setSize(200,30);
	g_pbar_loading->setPos(g_renderer->_ScreenWidth/2-100,g_renderer->_ScreenHeight/2-15);
	g_pbar_loading->setMaxMin(1,0);
	g_pbar_loading->setValue(0.5);
	g_pbar_loading->Visible = false;
	g_load_state->_LoadBar = g_pbar_loading;
	g_screen_load_mols->addElement(g_pbar_loading);

	//label de chargement
	g_label_load = new GUILabel();
	g_label_load->Text = "Loading...";
	g_label_load->Width = 400;
	g_label_load->X = 1;
	g_label_load->X = g_renderer->_ScreenWidth/2 - g_label_load->Width/2;
	g_label_load->Centering = true;
	g_label_load->Visible = true;
	g_load_state->_LoadLabel = g_label_load;
	g_screen_load_mols->addElement(g_label_load);

	//ECRAN DE CHOIX DES MOLECULES
	g_screen_choose_mol = new GUIScreen();

	g_cbox_mol1 = new GUIComboBox();
	g_cbox_mol1->Visible = false;
	g_cbox_mol1->LstBox->setMaxElements(5);
	g_cbox_mol1->setPos(100,100,200,10);
	g_screen_choose_mol->addElement(g_cbox_mol1);
	g_choose_mol_state->_CbMol1 = g_cbox_mol1;

	g_cbox_mol2 = new GUIComboBox();
	g_cbox_mol2->Visible = false;
	g_cbox_mol2->LstBox->setMaxElements(5);
	g_cbox_mol2->setPos(100,120,200,10);
	g_screen_choose_mol->addElement(g_cbox_mol2);
	g_choose_mol_state->_CbMol2 = g_cbox_mol2;

	g_bouton_dock = new GUIBouton();
	g_bouton_dock->Titre = "dock";
	g_bouton_dock->X = 100;
	g_bouton_dock->Y = 150;
	g_bouton_dock->Width = 100;
	g_screen_choose_mol->addElement(g_bouton_dock);

	//Ajoute a la fin pour focus en dernier
	g_screen_choose_mol->addElement(g_lst_scores);

	g_choose_mol_state->_BoutonVal = g_bouton_dock;
	g_choose_mol_state->_Screen = g_screen_choose_mol;

	//Ecran de jeu
	x=10;
	y=10;
	g_screen_jeu = new GUIScreen();

	g_mol_state->_Screen = g_screen_jeu;
	g_ship_state->_Screen = g_screen_jeu;

	//PBAR pour afficher la capture d'une molecule
	g_pbar_acquire = new GUIPBar();
	g_pbar_acquire->Width = 200;
	g_pbar_acquire->setPos(g_renderer->_ScreenWidth/2-100,20);
	g_pbar_acquire->setMaxMin(1,0);
	g_pbar_acquire->setValue(0.5);
	g_pbar_acquire->Visible = false;
	g_screen_jeu->addElement(g_pbar_acquire);
	g_ship_state->_PbarAcquire = g_pbar_acquire;

	//PBar de score
	g_pbar_best_score = new GUIPBar();
	g_pbar_best_score->Width = g_renderer->_ScreenWidth/3;
	g_pbar_best_score->setPos(g_renderer->_ScreenWidth/2-g_renderer->_ScreenWidth/6,40);
	g_pbar_best_score->setMaxMin(10,0);
	g_pbar_best_score->setValue(0);
	g_pbar_best_score->setColorFond(NYColor(0.8f,0.9f,0.8f,1.0f));
	g_screen_jeu->addElement(g_pbar_best_score);
	g_mol_state->_PbarBestScore = g_pbar_best_score;

	g_pbar_score = new GUIPBar();
	g_pbar_score->Width = g_renderer->_ScreenWidth/3;
	g_pbar_score->setPos(g_renderer->_ScreenWidth/2-g_renderer->_ScreenWidth/6,40);
	g_pbar_score->setMaxMin(10,0);
	g_pbar_score->setValue(0);
	g_pbar_score->setColorFond(NYColor(0.7f,0.9f,0.7f,1.0f));
	g_screen_jeu->addElement(g_pbar_score);
	g_mol_state->_PbarScore = g_pbar_score;

	g_pbar_charge = new GUIPBar();
	g_pbar_charge->Width = g_renderer->_ScreenWidth/3;
	g_pbar_charge->setPos(g_renderer->_ScreenWidth/2-g_renderer->_ScreenWidth/6,40);
	g_pbar_charge->setMaxMin(10,0);
	g_pbar_charge->setValue(0);
	g_pbar_charge->setColorFond(NYColor(0.7f,0.8f,0.7f,1.0f));
	g_screen_jeu->addElement(g_pbar_charge);
	g_mol_state->_PbarCharge = g_pbar_charge;

	g_lbl_best_login = new GUILabel();
	g_lbl_best_login->X = g_pbar_score->X;
	g_lbl_best_login->Y = g_pbar_score->Y + g_pbar_score->Height + 5;
	g_screen_jeu->addElement(g_lbl_best_login);
	g_mol_state->_LblBestLogin = g_lbl_best_login;

	g_pbar_optim = new GUIPBar();
	g_pbar_optim->setSize(200,30);
	g_pbar_optim->setPos(g_renderer->_ScreenWidth/2-100,g_renderer->_ScreenHeight/2-15);
	g_pbar_optim->setMaxMin(100,0);
	g_pbar_optim->setValue(0.0);
	g_pbar_optim->Visible = false;
	g_mol_state->_PbarOptim = g_pbar_optim;
	g_screen_jeu->addElement(g_pbar_optim);

	//On ajoute le mode mols à l'écran de jeu
	g_mol_state->addToGui(x,y,g_screen_jeu);

	LabelFps = new GUILabel();
	LabelFps->Text = "FPS";
	LabelFps->X = x;
	LabelFps->Y = y;
	LabelFps->Visible = true;
	g_screen_jeu->addElement(LabelFps);

	y += LabelFps->Height + 10;

	//Bouton pour charger une nouvelle scène
	BtnLoad  = new GUIBouton();
	BtnLoad->Titre = std::string("Load");
	BtnLoad->X = x;
	BtnLoad->Y = y;
	BtnLoad->setOnClick(clickBtnLoad);
	g_screen_jeu->addElement(BtnLoad);

	y += BtnLoad->Height + 10;

	if(adminUi)
	{
		//Bouton pour afficher les params
		BtnParams = new GUIBouton();
		BtnParams->Titre = std::string("Params");
		BtnParams->X = x;
		BtnParams->Y = y;
		BtnParams->setOnClick(clickBtnTest);
		g_screen_jeu->addElement(BtnParams);

		y += BtnParams->Height + 10;
	}

	BtnHelp = new GUIBouton();
	BtnHelp->Titre = std::string("Controls");
	BtnHelp->X = x;
	BtnHelp->Y = y;
	BtnHelp->setOnClick(clickBtnHelp);
	g_screen_jeu->addElement(BtnHelp);

	y += BtnHelp->Height + 10;

	if(vaisseauUI)
	{
		//Bouton pour switcher entre modes
		BtnSwitch = new GUIBouton();
		BtnSwitch->Titre = std::string("Mobile Cam");
		BtnSwitch->X = x;
		BtnSwitch->Y = y;
		BtnSwitch->setOnClick(clickBtnSwitch);
		g_screen_jeu->addElement(BtnSwitch);

		y += BtnSwitch->Height + 10;
	}

	//Bouton pour freezer
	BtnFreezePhysics = new GUIBouton();
	BtnFreezePhysics->Titre = std::string("Freeze");
	BtnFreezePhysics->X = x;
	BtnFreezePhysics->Y = y;
	BtnFreezePhysics->ColorFond = NYColor(1.0f,0.7f,0.7f,1.0f);
	BtnFreezePhysics->setOnClick(clickBtnFreeze);
	g_screen_jeu->addElement(BtnFreezePhysics);

	g_mol_state->_BtnFreeze = BtnFreezePhysics;
	g_ship_state->_BtnFreeze = BtnFreezePhysics;

	y += BtnFreezePhysics->Height + 10;

	//Bouton pour optimizer
	GUIBouton * btnOptimize = new GUIBouton();
	btnOptimize->Titre = std::string("Optimize");
	btnOptimize->X = x;
	btnOptimize->Y = y;
	btnOptimize->setOnClick(clickBtnOptimize);
	g_screen_jeu->addElement(btnOptimize);

	y += btnOptimize->Height + 10;


	//Bouton pour charger son meilleur score
	/*GUIBouton * btnGetBestComplex = new GUIBouton();
	btnGetBestComplex->Titre = std::string("Dld best");
	btnGetBestComplex->X = x;
	btnGetBestComplex->Y = y;
	btnGetBestComplex->setOnClick(clickGetBestComplex);
	g_screen_jeu->addElement(btnGetBestComplex);

	y += btnGetBestComplex->Height + 10;*/

	GUIBouton * btnExportPose = new GUIBouton();
	btnExportPose->Titre = std::string("Export");
	btnExportPose->X = x;
	btnExportPose->Y = y;
	btnExportPose->setOnClick(clickExportPose);
	g_screen_jeu->addElement(btnExportPose);

	y += btnExportPose->Height + 10;

	//Le label des joueurs battus
	g_lbl_beated_names = new GUILabel();
	g_lbl_beated_names->Centering = true;
	g_lbl_beated_names->Visible = false;
	g_lbl_beated_names->FontNum = 1;
	g_lbl_beated_names->Border = true;
	g_lbl_beated_names->FondPlein = true;
	g_lbl_beated_names->ColorBorder = NYColor(1.0f,1.0f,1.0f,0.5f);
	g_lbl_beated_names->ColorFond = NYColor(0.0f,0.0f,0.0f,0.5f);
	g_screen_jeu->addElement(g_lbl_beated_names);
	g_mol_state->_LblBeatedNames = g_lbl_beated_names;

	//Ecran de parametrage


	x = 10;
	y = 10;
	g_screen_params = new GUIScreen();

	btnClose = new GUIBouton();
	btnClose->Titre = std::string("Close");
	btnClose->X = x;
	btnClose->setOnClick(clickBtnCloseParam);
	g_screen_params->addElement(btnClose);

	y += btnClose->Height + 1;
	y+=10;
	x+=10;

	GUILabel * label;

	label = new GUILabel();
	label->X = x;
	label->Y = y;
	label->Text = "Ambiance :";
	g_screen_params->addElement(label);

	y += label->Height + 1;

	g_slider_volume_ambiance = new GUISlider();
	g_slider_volume_ambiance->setPos(x,y);
	g_slider_volume_ambiance->setMaxMin(1,0);
	g_slider_volume_ambiance->Visible = true;
	g_screen_params->addElement(g_slider_volume_ambiance);

	y += g_slider_volume_ambiance->Height + 1;
	y+=10;

	//QUE POUR LE MODE TOTAL DECO
	#ifdef TOTAL_DECO

	GUIBouton * btnGetBestComplexes = new GUIBouton();
	btnGetBestComplexes->Titre = std::string("Get best complexes");
	btnGetBestComplexes->Width = 160;
	btnGetBestComplexes->X = x;
	btnGetBestComplexes->Y = y;
	btnGetBestComplexes->setOnClick(clickGetBestComplexes);
	g_screen_params->addElement(btnGetBestComplexes);

	y += btnGetBestComplexes->Height + 10;

	GUIBouton * btnAnalyseLogs = new GUIBouton();
	btnAnalyseLogs->Titre = std::string("Analyze Logs");
	btnAnalyseLogs->Width = 160;
	btnAnalyseLogs->X = x;
	btnAnalyseLogs->Y = y;
	btnAnalyseLogs->setOnClick(clickAnalyseLogs);
	g_screen_params->addElement(btnAnalyseLogs);

	y += btnAnalyseLogs->Height + 10;

	GUIBouton * btnGenerate = new GUIBouton();
	btnGenerate->Titre = std::string("Generate dummies");
	btnGenerate->Width = 160;
	btnGenerate->X = x;
	btnGenerate->Y = y;
	btnGenerate->setOnClick(clickGenerate);
	g_screen_params->addElement(btnGenerate);

	y += btnAnalyseLogs->Height + 10;

	/*GUIBouton * btnObj = new GUIBouton();
	btnObj->Titre = std::string("Save to obj");
	btnObj->Width = 160;
	btnObj->X = x;
	btnObj->Y = y;
	btnObj->setOnClick(clickObj);
	g_screen_params->addElement(btnObj);*/

	y += btnAnalyseLogs->Height + 10;

	label = new GUILabel();
	label->X = x;
	label->Y = y;
	label->Text = "Lower FPS :";
	g_screen_params->addElement(label);

	y += label->Height + 1;

	g_slider_lower_fps = new GUISlider();
	g_slider_lower_fps->setPos(x,y);
	g_slider_lower_fps->setMaxMin(100,0);
	g_slider_lower_fps->setValue(0);
	g_slider_lower_fps->Visible = true;
	g_screen_params->addElement(g_slider_lower_fps);

	y += g_slider_lower_fps->Height + 1;
	y+=10;



	GUIBouton * btnPlayGrain = new GUIBouton();
	btnPlayGrain->Titre = std::string("Play Grains");
	btnPlayGrain->X = x;
	btnPlayGrain->Y = y;
	btnPlayGrain->setOnClick(clickBtnPlayGrain);
	g_screen_params->addElement(btnPlayGrain);

	y += btnPlayGrain->Height + 1;

	g_slider_pos = new GUISlider();
	g_slider_pos->setPos(x,y);
	g_slider_pos->setMaxMin(0.95,0.05);
	g_slider_pos->Visible = true;
	g_screen_params->addElement(g_slider_pos);
	g_lbl_pos = new GUILabel();
	g_lbl_pos->X = x + g_slider_pos->Width + 10;
	g_lbl_pos->Y = y;
	g_screen_params->addElement(g_lbl_pos);

	y += g_slider_pos->Height + 1;

	g_slider_size = new GUISlider();
	g_slider_size->setPos(x,y);
	g_slider_size->setMaxMin(3.0,0.01);
	g_slider_size->Visible = true;
	g_screen_params->addElement(g_slider_size);
	g_lbl_size = new GUILabel();
	g_lbl_size->X = x + g_slider_pos->Width + 10;
	g_lbl_size->Y = y;
	g_screen_params->addElement(g_lbl_size);

	y += g_slider_size->Height + 1;

	g_slider_overlap = new GUISlider();
	g_slider_overlap->setPos(x,y);
	g_slider_overlap->setMaxMin(0.95,0.0);
	g_slider_overlap->Visible = true;
	g_screen_params->addElement(g_slider_overlap);
	g_lbl_overlap = new GUILabel();
	g_lbl_overlap->X = x + g_slider_pos->Width + 10;
	g_lbl_overlap->Y = y;
	g_screen_params->addElement(g_lbl_overlap);

	y += g_slider_overlap->Height + 1;



	g_slider_random = new GUISlider();
	g_slider_random->setPos(x,y);
	g_slider_random->setMaxMin(20.0,0);
	g_slider_random->Visible = true;
	g_screen_params->addElement(g_slider_random);
	g_lbl_random = new GUILabel();
	g_lbl_random->X = x + g_slider_pos->Width + 10;
	g_lbl_random->Y = y;
	g_screen_params->addElement(g_lbl_random);

	y += g_slider_random->Height + 1;

	//Params de rendu
	x = 400;
	y = 10;
	#endif


	label = new GUILabel();
	label->X = x;
	label->Y = y;
	label->Text = "Shading :";
	g_screen_params->addElement(label);

	y += label->Height + 1;

	g_slider_ambient_light = new GUISlider();
	g_slider_ambient_light->setPos(x,y);
	g_slider_ambient_light->setMaxMin(1.0,0);
	g_slider_ambient_light->Visible = true;
	g_screen_params->addElement(g_slider_ambient_light);
	g_slider_ambient_light->setValue(g_ambient_level);

	y += g_slider_ambient_light->Height + 1;

	label = new GUILabel();
	label->X = x;
	label->Y = y;
	label->Text = "Cerne :";
	g_screen_params->addElement(label);

	y += label->Height + 1;

	g_slider_cerne = new GUISlider();
	g_slider_cerne->setPos(x,y);
	g_slider_cerne->setMaxMin(1.0,0);
	g_slider_cerne->Visible = true;
	g_screen_params->addElement(g_slider_cerne);
	g_slider_cerne->setValue(g_taille_cerne);

	y += g_slider_cerne->Height + 1;

	//Colors
	float h,s,v,a;
	g_color_positif.toHSV(&h,&s,&v,&a);

	label = new GUILabel();
	label->X = x;
	label->Y = y;
	label->Text = "Positive H S V :";
	g_screen_params->addElement(label);

	y += label->Height + 1;

	g_slider_color_pos_h = new GUISlider();
	g_slider_color_pos_h->setPos(x,y);
	g_slider_color_pos_h->setMaxMin(360.0,0);
	g_slider_color_pos_h->Visible = true;
	g_screen_params->addElement(g_slider_color_pos_h);
	g_slider_color_pos_h->setValue(h);
	y += g_slider_color_pos_h->Height + 1;

	g_slider_color_pos_s = new GUISlider();
	g_slider_color_pos_s->setPos(x,y);
	g_slider_color_pos_s->setMaxMin(1,0);
	g_slider_color_pos_s->Visible = true;
	g_screen_params->addElement(g_slider_color_pos_s);
	g_slider_color_pos_s->setValue(s);
	y += g_slider_color_pos_s->Height + 1;

	g_slider_color_pos_v = new GUISlider();
	g_slider_color_pos_v->setPos(x,y);
	g_slider_color_pos_v->setMaxMin(1,0);
	g_slider_color_pos_v->Visible = true;
	g_screen_params->addElement(g_slider_color_pos_v);
	g_slider_color_pos_v->setValue(v);
	y += g_slider_color_pos_v->Height + 1;

	g_color_negatif.toHSV(&h,&s,&v,&a);

	label = new GUILabel();
	label->X = x;
	label->Y = y;
	label->Text = "Negative H S V :";
	g_screen_params->addElement(label);

	y += label->Height + 1;

	g_slider_color_neg_h = new GUISlider();
	g_slider_color_neg_h->setPos(x,y);
	g_slider_color_neg_h->setMaxMin(360.0,0);
	g_slider_color_neg_h->Visible = true;
	g_screen_params->addElement(g_slider_color_neg_h);
	g_slider_color_neg_h->setValue(h);
	y += g_slider_color_neg_h->Height + 1;

	g_slider_color_neg_s = new GUISlider();
	g_slider_color_neg_s->setPos(x,y);
	g_slider_color_neg_s->setMaxMin(1,0);
	g_slider_color_neg_s->Visible = true;
	g_screen_params->addElement(g_slider_color_neg_s);
	g_slider_color_neg_s->setValue(s);
	y += g_slider_color_neg_s->Height + 1;

	g_slider_color_neg_v = new GUISlider();
	g_slider_color_neg_v->setPos(x,y);
	g_slider_color_neg_v->setMaxMin(1,0);
	g_slider_color_neg_v->Visible = true;
	g_screen_params->addElement(g_slider_color_neg_v);
	g_slider_color_neg_v->setValue(v);
	y += g_slider_color_neg_v->Height + 1;

	g_color_fond.toHSV(&h,&s,&v,&a);

	label = new GUILabel();
	label->X = x;
	label->Y = y;
	label->Text = "Background H S V :";
	g_screen_params->addElement(label);

	y += label->Height + 1;

	g_slider_color_fond_h = new GUISlider();
	g_slider_color_fond_h->setPos(x,y);
	g_slider_color_fond_h->setMaxMin(360.0,0);
	g_slider_color_fond_h->Visible = true;
	g_screen_params->addElement(g_slider_color_fond_h);
	g_slider_color_fond_h->setValue(h);
	y += g_slider_color_fond_h->Height + 1;

	g_slider_color_fond_s = new GUISlider();
	g_slider_color_fond_s->setPos(x,y);
	g_slider_color_fond_s->setMaxMin(1,0);
	g_slider_color_fond_s->Visible = true;
	g_screen_params->addElement(g_slider_color_fond_s);
	g_slider_color_fond_s->setValue(s);
	y += g_slider_color_fond_s->Height + 1;

	g_slider_color_fond_v = new GUISlider();
	g_slider_color_fond_v->setPos(x,y);
	g_slider_color_fond_v->setMaxMin(1,0);
	g_slider_color_fond_v->Visible = true;
	g_screen_params->addElement(g_slider_color_fond_v);
	g_slider_color_fond_v->setValue(v);
	y += g_slider_color_fond_v->Height + 1;

	//Ecran a rendre
	g_screen_manager->setActiveScreen(g_screen_jeu);

	//On charge les sons
	g_sound_ambiance = new SoundBasic();
	g_sound_ambiance->load("sound/snd_background_1.wav");
	g_sound_ambiance->setVolume(0.2f);
	g_slider_volume_ambiance->setValue(0.2);
	if(!PlayerInfo::getInstance()->_ShowFeedBacks)
	{
		g_sound_ambiance->setVolume(0.0f);
		g_slider_volume_ambiance->setValue(0.0);
	}

	g_sound_ambiance->_Loop = true;
	g_sound_ambiance->play();
	g_sound_engine->addSound(g_sound_ambiance);

	char fileName[100];
	for(int i=0;i<NB_SOUND_ATTACH;i++)
	{
		memset(fileName,0,100);
		sprintf(fileName,"sound/att_det/snd_attach_%d.wav",i+1);
		g_sounds_attach[i]= new SoundBasic();
		g_sounds_attach[i]->load(fileName);
		g_sound_engine->addSound(g_sounds_attach[i]);
	}

	g_sound_detach =  new SoundBasic();
	g_sound_detach->load("sound/att_det/snd_detach_1.wav");
	g_sound_engine->addSound(g_sound_detach);
	g_sound_high_score =  new SoundBasic();
	g_sound_high_score->load("sound/snd_high_score_5.wav");
	g_sound_high_score->setVolume(0.7f);
	g_sound_engine->addSound(g_sound_high_score);
	g_sound_reset =  new SoundBasic();
	g_sound_reset->load("sound/snd_reset.wav");
	g_sound_reset->setVolume(0.5f);
	g_sound_engine->addSound(g_sound_reset);

	//On attache les sons
	g_mol_state->_SoundsAttach = g_sounds_attach;
	g_mol_state->_NbSoundAttach = NB_SOUND_ATTACH;
	g_mol_state->_SoundDetach = g_sound_detach;
	g_mol_state->_SoundHighScore = g_sound_high_score;
	g_mol_state->_SoundReset = g_sound_reset;
	g_ship_state->_SoundsAttach = g_sounds_attach;
	g_ship_state->_NbSoundAttach = NB_SOUND_ATTACH;
	g_ship_state->_SoundDetach = g_sound_detach;

	#ifdef TOTAL_DECO
	g_sound_grain = new SoundGrain();
	g_sound_grain->loadBaseFile("sound/grain_2.wav");
	g_sound_grain->_Loop = true;
	g_sound_grain->setGrainParam(0.5f,0.05f,0.05f,0.7f);
	g_sound_engine->addSound(g_sound_grain);

	g_slider_pos->setValue(0.5);
	g_slider_size->setValue(0.05);
	g_slider_random->setMaxMin(g_sound_grain->getDureeBase(),0);
	g_slider_random->setValue(0.05);
	g_slider_overlap->setValue(0.7);
	#endif

	//Init Timer
	g_timer = new NYTimer();

	g_moleculesManager = new MoleculesManager();

	/*bool lowPoly = false;
	#ifdef _DEBUG
	lowPoly = true;
	#endif

	Molecule * mol = new MoleculeCubes();
	mol->loadFromMOL2("molecules/molecule1.mol2");
	mol->makeGeometry(lowPoly,true,0.0f);
	mol->addToOpengl();
	mol->makeCollisionShapeFromGeom(g_phyEngine, true);
	//mol->makeGeometry(true,false);
	mol->_Position.Translate(NYVert3Df(75,0,0));
	mol->addToPhysicEngine(g_phyEngine);
	//mol->_Body->setFreeze(true);

	//float size = mol->_Body->getSize();

	//mol->_Body->setTransRotFactors(NYVert3Df(0,0,0),NYVert3Df(1,1,1));
	g_moleculesManager->addMolecule(mol);
	mol->releaseTempGeometry();

	unsigned long memoryBefore = getMemoryUsage();
	mol = new MoleculeCubes();
	mol->loadFromMOL2("molecules/molecule2.mol2");
	mol->makeGeometry(lowPoly,true,0.0f);
	mol->addToOpengl();
	mol->makeCollisionShapeFromGeom(g_phyEngine, true);
	//mol->makeGeometry(true,false);
	mol->_Position.Translate(NYVert3Df(-75,0,0));
	mol->addToPhysicEngine(g_phyEngine);
	//mol->_Body->setFreeze(true);
	g_moleculesManager->addMolecule(mol);
	unsigned long memoryAfter = getMemoryUsage();
	mol->releaseTempGeometry();
	unsigned long memoryAfterRelease = getMemoryUsage();
	_cprintf("used %ld ko to make mol, but only %ld ko after relase\n",(memoryAfter - memoryBefore)/1024,(memoryAfterRelease - memoryBefore)/1024);*/


	//Ajout au state
	g_mol_state->_MoleculesManager = g_moleculesManager;
	g_ship_state->_MoleculesManager = g_moleculesManager;
	g_load_state->_MoleculesManager = g_moleculesManager;

	//CHARGEMENT DES VAISSEAUX
	g_ship_manager = new ShipManager();
	g_ship_state->g_ship_manager = g_ship_manager;
	g_ship_manager->buildBaseShip(g_phyEngine);
	Ship * ship = g_ship_manager->Ships[0];
	ship->loadShip("myShip.bin");
	ship->addToPhysicEngine(g_phyEngine);
	ship->_OwnedByPlayer = true;
	g_ship_state->MyShip = ship;
	g_load_state->MyShip = ship;
	ship->setPosition(NYVert3Df(0,-160,0));

	//Projectiles
	g_proj_manager =  ProjectilesManager::getInstance();

	//On start
	g_choose_mol_state->start();
	g_load_state->start();
	g_mol_state->start();
	g_ship_state->start();
	g_timer->start();

	//On charge le premier couple
	g_login_state->activate(true);
	g_choose_mol_state->activate(false);
	g_mol_state->activate(false);
	g_ship_state->activate(false);
	g_load_state->activate(false);

	//LogScreen::getInstance()->setMessage("Welcome to UDock");

	resizeFunction(g_renderer->_ScreenWidth,g_renderer->_ScreenHeight);

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
		update();

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
				case SDL_WINDOWEVENT:
					switch(evt.window.event)
					{
						case SDL_WINDOWEVENT_RESIZED: resizeFunction(evt.window.data1,evt.window.data2); break;
						case SDL_WINDOWEVENT_SIZE_CHANGED : resizeFunction(evt.window.data1,evt.window.data2); break;
					}
					break;
				//default : Log::log(Log::USER_INFO,("SDL Event "+toString(evt.type)).c_str());
			}
			//Log::log(Log::USER_INFO,("SDL Event "+toString(evt.type)).c_str());
		}
	}

	// On quitte la SDL
	SDL_GL_DeleteContext(contexteOpenGL);
	SDL_DestroyWindow(fenetre);
	SDL_Quit();

	return 0;

	return 0;
}
