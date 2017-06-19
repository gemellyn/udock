#ifndef __HTTP_MANAGER_PUBLIC_DECO_H__
#define __HTTP_MANAGER_PUBLIC_DECO_H__

#include "curl/curl.h"
#include "engine/utils/ny_utils.h"
#include "log/log_screen.h"
#include "global.h"
#include "http_manager_udock.h"
#include "boost/filesystem.hpp"

#include <locale>


class HttpManagerPublicDeco : public HttpManagerUdock
{
	protected:

		HttpManagerPublicDeco()
		{

		}

	public:


		static HttpManagerUdock * getInstance()
		{
			if(_Instance == NULL)
			{
				_Instance = new HttpManagerPublicDeco();
			}
			return _Instance;
		}

		virtual bool getLeaderboard(std::vector<std::string> * ldb, int iduser, int * posUser)
		{
			Log::log(Log::ENGINE_INFO,"Getting leaderboard from server (mytho)");
			*posUser = 0;
			return true;
		}

		virtual bool addPlayTime(int iduser, float time)
		{
			Log::log(Log::ENGINE_INFO,"Adding play time to server (mytho)");
			return true;
		}

		virtual int getUser(std::string & user, std::string & pass, std::string & message)
		{

			CURLcode res;

			Log::log(Log::ENGINE_INFO,"Checking user login (public deco)");

			if(init()) {


				curl_easy_setopt(_Curl, CURLOPT_URL, "http://udock.fr/getUser.php?ano=1");


				std::string post = "tlogin=";
				post += curl_easy_escape(_Curl,user.c_str(),user.length());
				post += "&tpass=";
				post += curl_easy_escape(_Curl,pass.c_str(),pass.length());
				post += "&toverall=";
				post += toString(UDOCK_OVERALL_VERSION);
				post += "&tmajor=";
				post += toString(UDOCK_MAJOR_VERSION);
				post += "&tminor=";
				post += toString(UDOCK_MINOR_VERSION);

				curl_easy_setopt(_Curl, CURLOPT_POSTFIELDS, post.c_str());

				res = curl_easy_perform(_Curl);
				if (res != CURLE_OK)
				{
					Log::log(Log::ENGINE_ERROR,("Failed to contact url : "+toString(_ErrorBuffer)).c_str());
					close();
					return 9998;
				}

				int userId;
				sscanf(_Buffer.c_str(),"%d",&userId);

				Log::log(Log::ENGINE_INFO,("Server response : "+_Buffer).c_str());
				Log::log(Log::ENGINE_INFO,("User id : "+toString(userId)).c_str());

				if(userId == 0)
				{
					Log::log(Log::USER_INFO,"There was an error during login process");
					return 9998;
				}

				close();
				return 9999;
			}

			close();
			return 9998;


		}

		virtual bool getHighScore(int iduser, int id1, int id2, float* myScore, std::vector<HighScoreServer> & scores)
		{


			Log::log(Log::ENGINE_INFO,"Getting scores from server (mytho)");

			* myScore = -0.0f;
			scores.clear();

			return true;
		}

		virtual bool postScore(int iduser, int id1, int id2, float score, Positionator & pos1, Positionator & pos2)
		{


			Log::log(Log::ENGINE_INFO,"Posting scores to server (mytho)");

			return true;
		}


		virtual bool getMolList(std::vector<HttpMolStruct> * mols)
		{

			Log::log(Log::ENGINE_INFO,"Getting mol list from disc");

			namespace fs = boost::filesystem;
            fs::path moldir("./import/");
            fs::directory_iterator end_iter;

            if ( fs::exists(moldir) && fs::is_directory(moldir))
            {
                bool num = 1;
                for( fs::directory_iterator dir_iter(moldir) ; dir_iter != end_iter ; ++dir_iter)
                {
                    if (fs::is_regular_file(dir_iter->status()) )
                    {
                        fs::path path = *dir_iter;
                        //Log::log(Log::ENGINE_INFO,path.extension().filename().c_str());
                        //if(path.extension().filename().c_str() == ".mol2")


                        HttpMolStruct mol;
                        mol.id = num++;
                        mol.name = path.stem().filename().string();
                        mol.filename = utf8_to_wstring("import/"+path.filename().string());
                        mols->push_back(mol);

                    }
                }
            }

            return true;
		}

		virtual bool getBestPlayerMessage(std::string & message)
		{
			Log::log(Log::ENGINE_INFO,"Getting best player message (mytho)");

			message = "You are not connected to the server";

			return true;
		}



		virtual bool uploadLogFile(int userId, std::string & logfilename)
		{

			Log::log(Log::ENGINE_INFO,"Uploading log file (mytho)");
			//LogScreen::getInstance()->setMessage("Log sync");

			return true;
		}

		virtual bool getBestComplexes(std::vector<HttpComplexStruct> * complexes)
		{

			Log::log(Log::ENGINE_INFO,"Getting best complexes list from server (mytho)");

			return true;
		}

};

#endif
