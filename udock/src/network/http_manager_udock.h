#ifndef __HTTP_MANAGER_UDOCK_H__
#define __HTTP_MANAGER_UDOCK_H__

#include "engine/network/http_manager.h"
#include "engine/utils/types_3d.h"
#include "engine/log/log.h"
#include "log/log_screen.h"


class HttpManagerUdock : public HttpManager
{

	protected:
		static HttpManagerUdock * _Instance;

	public:

		static HttpManagerUdock * getInstance()
		{
			if(_Instance == NULL)
			{
				_Instance = new HttpManagerUdock();
			}
			return _Instance;
		}

		virtual bool logAnalytics()
		{
			CURLcode res;

			Log::log(Log::ENGINE_INFO,"Log to analytics");

			if(init()) {

				//Realise en copiant l'url wireshark et en regardant le code de php-ga
				string request = "http://www.google-analytics.com/collect?";
				request += "v=1&_v=j23&a="+toString(rand()*0x7fffffff);
				request += "&t=pageview&_s=1";
				request += "&dl=http%3A%2F%2Fudock.fr%2F%3Fm%3DmagicLog";
				request += "&ul=fr&de=ISO-8859-1&dt=UDock&sd=24-bit&sr=1920x1200&vp=1903x1032&je=1&fl=14.0%20r0&_u=MACAAAQ~&cid=892074017.1393512387";
				request += "&tid=UA-4089207-13";

				//Log::log(Log::USER_INFO,("Analytics request : "+request).c_str());

				///collect?v=1&_v=j23&a=2076715758&t=pageview&_s=1&dl=http%3A%2F%2Fudock.fr%2F%3Fm%3Dmain&ul=fr&de=ISO-8859-1&dt=UDock&sd=24-bit&sr=1920x1200&vp=1903x1032&je=1&fl=14.0%20r0&_u=MACAAAQ~&cid=892074017.1393512387&tid=UA-4089207-13&z=441652460 HTTP/1.1

				curl_easy_setopt(_Curl, CURLOPT_URL, request.c_str());

				res = curl_easy_perform(_Curl);
				if (res != CURLE_OK)
				{
					Log::log(Log::ENGINE_ERROR,("Failed to contact url : "+toString(_ErrorBuffer)).c_str());
					//LogScreen::getInstance()->setMessage("Failed to contact server.");
					close();
					return false;
				}

				Log::log(Log::ENGINE_INFO,("Server response : "+_Buffer).c_str());

				close();
				return true;
			}

			close();
			return false;
		}

		virtual bool getLeaderboard(std::vector<std::string> * ldb, int iduser, int * posUser)
		{
			CURLcode res;

			Log::log(Log::ENGINE_INFO,"Getting leaderboard from server");

			if(init()) {

				curl_easy_setopt(_Curl, CURLOPT_URL, "http://udock.fr/getLdb.php");

				std::string post;
				post += "id=" + toString(iduser);

				curl_easy_setopt(_Curl, CURLOPT_POSTFIELDS, post.c_str());

				res = curl_easy_perform(_Curl);
				if (res != CURLE_OK)
				{
					Log::log(Log::ENGINE_ERROR,("Failed to contact url : "+toString(_ErrorBuffer)).c_str());
					//LogScreen::getInstance()->setMessage("Failed to contact server.");
					close();
					return false;
				}

				Log::log(Log::ENGINE_INFO,("Server response : "+_Buffer).c_str());

				//On recup les mols
				std::vector<string> champs;
				splitString(champs,_Buffer,'|');

				*posUser = stringTo<int>(champs[0]);

				for(unsigned int i=1;i<champs.size();)
				{
					ldb->push_back(champs[i++]);
				}

				close();
				return true;
			}

			close();
			return false;
		}

		virtual bool addPlayTime(int iduser, float time)
		{
			CURLcode res;

			Log::log(Log::ENGINE_INFO,"Adding play time to server");

			if(init()) {

				curl_easy_setopt(_Curl, CURLOPT_URL, "http://udock.fr/addPlayTime.php");

				std::string post;
				post += "iduser=" + toString(iduser);
				post += "&time=" + toString(time);

				curl_easy_setopt(_Curl, CURLOPT_POSTFIELDS, post.c_str());

				res = curl_easy_perform(_Curl);
				if (res != CURLE_OK)
				{
					Log::log(Log::ENGINE_ERROR,("Failed to contact url : "+toString(_ErrorBuffer)).c_str());
					//LogScreen::getInstance()->setMessage("Failed to contact server.");
					close();
					return false;
				}

				Log::log(Log::ENGINE_INFO,("Server response : "+_Buffer).c_str());

				close();
				return true;
			}

			close();
			return false;
		}

		virtual int getUser(std::string & user, std::string & pass, std::string & message)
		{
			CURLcode res;

			Log::log(Log::ENGINE_INFO,"Checking user login");
			//LogScreen::getInstance()->setMessage("Contacting server");

			if(init()) {

				curl_easy_setopt(_Curl, CURLOPT_URL, "http://udock.fr/getUser.php");

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
					//LogScreen::getInstance()->setMessage("Failed to contact server.");
					close();
					return 0;
				}

				int userId;
				sscanf(_Buffer.c_str(),"%d",&userId);

				Log::log(Log::ENGINE_INFO,("Server response : "+_Buffer).c_str());
				Log::log(Log::ENGINE_INFO,("User id : "+toString(userId)).c_str());

				if(userId == 0)
				{
					Log::log(Log::USER_INFO,"There was an error during login process");
				}

				close();
				return userId;
			}

			close();
			return 0;
		}

		class HighScoreServer
		{
			public :
				std::string Login;
				float Score;

			HighScoreServer & operator=(const HighScoreServer & s)
			{
				Login = s.Login;
				Score = s.Score;
				return *this;
			}
		};
		virtual bool getHighScore(int iduser, int id1, int id2, float* myScore, std::vector<HighScoreServer> & scores)
		{
			CURLcode res;

			Log::log(Log::ENGINE_INFO,"Getting scores from server");

			if(init()) {

				curl_easy_setopt(_Curl, CURLOPT_URL, "http://udock.fr/getScore.php");

				std::string post;
				post += "id1=" + toString(id1);
				post += "&id2=" + toString(id2);
				post += "&iduser=" + toString(iduser);

				curl_easy_setopt(_Curl, CURLOPT_POSTFIELDS, post.c_str());

				res = curl_easy_perform(_Curl);
				if (res != CURLE_OK)
				{
					Log::log(Log::ENGINE_ERROR,("Failed to contact url : "+toString(_ErrorBuffer)).c_str());
					//LogScreen::getInstance()->setMessage("Failed to contact server.");
					close();
					return false;
				}

				Log::log(Log::ENGINE_INFO,("Server response : "+_Buffer).c_str());

				std::vector<string> champs;
				splitString(champs,_Buffer,'|');

				Log::log(Log::ENGINE_INFO,(toString(champs.size()) + " strings in response").c_str());

				*myScore = 0.0f;

				if(champs.size() > 0)
				{
					if(champs[0].size())
						*myScore = stringTo<float>(champs[0]);

					scores.clear();
					scores.resize((champs.size()-1)/2);
					for(unsigned int i=0;i<scores.size();i++)
					{
						scores[i].Login = champs[(2*i)+1];
						if(champs[(2*i)+2].size())
							scores[i].Score = stringTo<float>(champs[(2*i)+2]);
					}
				}
				else
				{
					Log::log(Log::USER_INFO,"No current best score for these mols");
				}

				Log::log(Log::USER_INFO,("My score : "+toString(*myScore)).c_str());

				close();
				return true;
			}

			close();
			return false;
		}

		virtual bool postScore(int iduser, int id1, int id2, float score, Positionator & pos1, Positionator & pos2)
		{
			CURLcode res;

			Log::log(Log::ENGINE_INFO,"Posting scores to server");

			if(init()) {

				curl_easy_setopt(_Curl, CURLOPT_URL, "http://udock.fr/postScore.php");

				//On calcule le petit champ de sécurisation
				sint32 secu = (sint32)floor(score);
				secu ^= 0xabdefabd;
				string sSecu;
				for(int i=0;i<8;i++)
				{
					sSecu += toString((char)('a'+(secu & 0x0000000f)));
					secu >>= 4;
				}

				std::string post;
				float xyz[3];
				post += "id1=" + toString(id1);
				post += "&id2=" + toString(id2);
				post += "&iduser=" + toString(iduser);
				post += "&score=" + toString(score);
				post += "&x1=" + toString(pos1._Pos.X);
				post += "&y1=" + toString(pos1._Pos.Y);
				post += "&z1=" + toString(pos1._Pos.Z);
				pos1._Rot.toEulerXYZ(xyz);
				post += "&rx1=" + toString(xyz[0]);
				post += "&ry1=" + toString(xyz[1]);
				post += "&rz1=" + toString(xyz[2]);
				post += "&x2=" + toString(pos2._Pos.X);
				post += "&y2=" + toString(pos2._Pos.Y);
				post += "&z2=" + toString(pos2._Pos.Z);
				pos2._Rot.toEulerXYZ(xyz);
				post += "&rx2=" + toString(xyz[0]);
				post += "&ry2=" + toString(xyz[1]);
				post += "&rz2=" + toString(xyz[2]);
				post += "&ttt=" + sSecu;

				curl_easy_setopt(_Curl, CURLOPT_POSTFIELDS, post.c_str());

				res = curl_easy_perform(_Curl);
				if (res != CURLE_OK)
				{
					Log::log(Log::ENGINE_ERROR,("Failed to contact url : "+toString(_ErrorBuffer)).c_str());
					//LogScreen::getInstance()->setMessage("Failed to contact server.");
					close();
					return false;
				}

				Log::log(Log::ENGINE_INFO,("Server response : "+_Buffer).c_str());

				close();
				return true;
			}

			close();
			return false;
		}


		typedef struct HttpMolStruct_Struct
		{
			int id;
			std::string name;
			std::wstring filename;
		}HttpMolStruct;

		virtual bool getMolList(std::vector<HttpMolStruct> * mols)
		{
			CURLcode res;

			Log::log(Log::ENGINE_INFO,"Getting mol list from server");

			if(init()) {

				curl_easy_setopt(_Curl, CURLOPT_URL, "http://udock.fr/getMols.php");

				res = curl_easy_perform(_Curl);
				if (res != CURLE_OK)
				{
					Log::log(Log::ENGINE_ERROR,("Failed to contact url : "+toString(_ErrorBuffer)).c_str());
					//LogScreen::getInstance()->setMessage("Failed to contact server.");
					close();
					return false;
				}

				Log::log(Log::ENGINE_INFO,("Server response : "+_Buffer).c_str());

				//On recup les mols
				std::vector<string> champs;
				splitString(champs,_Buffer,'|');

				for(unsigned int i=0;i<champs.size();)
				{
					HttpMolStruct mol;
					mol.id = stringTo<int>(champs[i++]);
					mol.name = champs[i++];
					mol.filename = L"molecules/" + toWString(mol.id)+ L".mol2";
					mols->push_back(mol);
				}

				close();
				return true;
			}

			close();
			return false;
		}

		virtual bool getBestPlayerMessage(std::string & message)
		{
			CURLcode res;

			Log::log(Log::ENGINE_INFO,"Getting best player message");

			if(init()) {

				curl_easy_setopt(_Curl, CURLOPT_URL, "http://udock.fr/getBestPlayer.php");

				res = curl_easy_perform(_Curl);
				if (res != CURLE_OK)
				{
					Log::log(Log::ENGINE_ERROR,("Failed to contact url : "+toString(_ErrorBuffer)).c_str());
					//LogScreen::getInstance()->setMessage("Failed to contact server.");
					close();
					return false;
				}

				Log::log(Log::ENGINE_INFO,("Server response : "+_Buffer).c_str());

				//On recup les mols
				message = _Buffer;

				close();
				return true;
			}

			close();
			return false;
		}



		virtual bool uploadLogFile(int userId, std::string & logfilename)
		{
			CURLcode res;

			struct curl_httppost *formpost=NULL;
			struct curl_httppost *lastptr=NULL;
			struct curl_slist *headerlist=NULL;
			static const char buf[] = "Expect:";

			Log::log(Log::ENGINE_INFO,"Uploading log file");
			//LogScreen::getInstance()->setMessage("Log sync");

			if(init())
			{

				/* Fill in the file upload field */
				curl_formadd(&formpost,
					&lastptr,
					CURLFORM_COPYNAME, "logfile",
					CURLFORM_FILE, logfilename.c_str(),
					CURLFORM_END);

				/* Fill in the filename field */
				curl_formadd(&formpost,
					&lastptr,
					CURLFORM_COPYNAME, "filename",
					CURLFORM_COPYCONTENTS, logfilename.c_str(),
					CURLFORM_END);


				/* Fill in the submit field too, even if this is rarely needed */
				curl_formadd(&formpost,
					&lastptr,
					CURLFORM_COPYNAME, "submit",
					CURLFORM_COPYCONTENTS, "send",
					CURLFORM_END);

				/* Fill in the submit field too, even if this is rarely needed */
				curl_formadd(&formpost,
					&lastptr,
					CURLFORM_COPYNAME, "userId",
					CURLFORM_COPYCONTENTS,  toString(userId).c_str(),
					CURLFORM_END);

				/* initalize custom header list (stating that Expect: 100-continue is not
					wanted */
				headerlist = curl_slist_append(headerlist, buf);

				/* what URL that receives this POST */
				curl_easy_setopt(_Curl, CURLOPT_URL, "http://udock.fr/postLog.php");
				curl_easy_setopt(_Curl, CURLOPT_HTTPPOST, formpost);

				res = curl_easy_perform(_Curl);
				/* Check for errors */
				if(res != CURLE_OK)
				{
					Log::log(Log::ENGINE_ERROR,("Failed to contact url : "+toString(_ErrorBuffer)).c_str());
					//LogScreen::getInstance()->setMessage("Failed to contact server.");
					close();
					return false;
				}

				//LogScreen::getInstance()->setMessage("Log sync ok.");

				Log::log(Log::ENGINE_INFO,("Server response : "+_Buffer).c_str());

				/* then cleanup the formpost chain */
				curl_formfree(formpost);
				/* free slist */
				curl_slist_free_all (headerlist);

				close();
				return true;

			}

			close();
			return false;
		}

		typedef struct HttpComplexStruct_Struct
		{
			int id1;
			int id2;
			int iduser;
			Positionator pos1;
			Positionator pos2;
		}HttpComplexStruct;

		virtual bool getBestComplexes(std::vector<HttpComplexStruct> * complexes, int idUser = -1)
		{
			CURLcode res;

			Log::log(Log::ENGINE_INFO,"Getting best complexes list from server");

			if(init()) {

				std::string params;
				if(idUser > 0)
					params = "?user="+toString(idUser);

				curl_easy_setopt(_Curl, CURLOPT_URL, ("http://udock.fr/getComplexes.php"+params).c_str());

				res = curl_easy_perform(_Curl);
				if (res != CURLE_OK)
				{
					Log::log(Log::ENGINE_ERROR,("Failed to contact url : "+toString(_ErrorBuffer)).c_str());
					close();
					return false;
				}

				Log::log(Log::ENGINE_INFO,("Server response : "+_Buffer).c_str());

				//On recup les complexes
				std::vector<string> champs;
				splitString(champs,_Buffer,'|');

				for(unsigned int i=0;i<champs.size();)
				{
					HttpComplexStruct complStr;
					complStr.iduser = stringTo<int>(champs[i++]);
					complStr.id1 = stringTo<int>(champs[i++]);
					complStr.id2 = stringTo<int>(champs[i++]);
					complStr.pos1._Pos = NYVert3Df(stringTo<float>(champs[i]),stringTo<float>(champs[i+1]),stringTo<float>(champs[i+2]));
					i+=3;
					complStr.pos1._Rot.createRotateXYZ(stringTo<float>(champs[i]),stringTo<float>(champs[i+1]),stringTo<float>(champs[i+2]));
					i+=3;
					complStr.pos2._Pos = NYVert3Df(stringTo<float>(champs[i]),stringTo<float>(champs[i+1]),stringTo<float>(champs[i+2]));
					i+=3;
					complStr.pos2._Rot.createRotateXYZ(stringTo<float>(champs[i]),stringTo<float>(champs[i+1]),stringTo<float>(champs[i+2]));
					i+=3;
					complexes->push_back(complStr);
				}

				close();
				return true;
			}

			close();
			return false;
		}

};

#endif
