#ifndef __HTTP_MANAGER_TOTAL_DECO_H__
#define __HTTP_MANAGER_TOTAL_DECO_H__

#include "curl/curl.h"
#include "engine/utils/ny_utils.h"
#include "log/log_screen.h"
#include "global.h"
#include "http_manager_public_deco.h"
#include "http_manager_udock.h"

class HttpManagerTotalDeco : public HttpManagerPublicDeco
{
	protected:

		HttpManagerTotalDeco()
		{

		}

	private:

		virtual bool init (void)
		{
			return true;
		}

	public:

		static HttpManagerUdock * getInstance()
		{
			if(_Instance == NULL)
			{
				_Instance = new HttpManagerTotalDeco();
			}
			return _Instance;
		}

		virtual bool logAnalytics()
		{
			Log::log(Log::ENGINE_INFO, "Log to analytics mytho, total deco");
			return true;
		}

		virtual bool getLeaderboard(std::vector<std::string> * ldb, int iduser, int * posUser)
		{
			Log::log(Log::ENGINE_INFO,"Getting leaderboard from server (mytho)");
			for(int i=0;i<8;i++)
				ldb->push_back(toString(i) + " " + std::string("Name"));
			*posUser = 3;
			return true;
		}

		virtual bool addPlayTime(int iduser, float time)
		{
			Log::log(Log::ENGINE_INFO,"Adding play time to server (mytho)");
			return true;
		}

		virtual int getUser(std::string & user, std::string & pass, std::string & message)
		{

			Log::log(Log::ENGINE_INFO,"Checking user login (mytho)");
			//LogScreen::getInstance()->setMessage("Contacting server");

			return 9999;
		}

		virtual bool getHighScore(int iduser, int id1, int id2, float* myScore, std::vector<HighScoreServer> & scores)
		{


			Log::log(Log::ENGINE_INFO,"Getting scores from server (mytho)");


			* myScore = -2.0f;
			/*scores.resize(100);
			scores[0].Login = "SandMan0";
			scores[0].Score = -20.0f;
			for(int i=1;i<100;i++)
			{
				scores[i].Login = "SandMan"+toString(i);
				scores[i].Score = scores[i-1].Score-randf();
			}*/

			scores.resize(3);
			scores[0].Login = "Kipper";
			scores[0].Score = -20.0f;
			scores[1].Login = "MaitreZu";
			scores[1].Score = -30.0f;
			scores[2].Login = "MrKoala";
			scores[2].Score = -40.0f;

			return true;
		}

		virtual bool postScore(int iduser, int id1, int id2, float score, Positionator & pos1, Positionator & pos2)
		{


			Log::log(Log::ENGINE_INFO,"Posting scores to server (mytho)");

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
