#ifndef __JOB_HTTP_LOGIN_H__
#define __JOB_HTTP_LOGIN_H__

#include "engine/network/http_manager.h"

#include "engine/network/http_job.h"

#include "engine/log/log.h"

class HttpJobLogin : public HttpJob
{
	public:

		HttpJobLogin() : HttpJob()
		{
		}

		//In
		std::string _Login;
		std::string _Pass;
		//Out
		int _IdUser;

		static unsigned int threadLogin(void * loginjob)
		{
			//On log d'abord pour analytics
			HttpManagerUdock::getInstance()->logAnalytics();

			HttpJobLogin * job = (HttpJobLogin*) loginjob;
			job->_IdUser = HttpManagerUdock::getInstance()->getUser(job->_Login,job->_Pass, job->_MessageEnCours);
			Log::log(Log::ENGINE_INFO,"End of login job");
			job->_JobDone = true;
			return 0;
		}

		void launchJob(void)
		{
			_JobRunning = true;
			_JobDone = false;
			Log::log(Log::ENGINE_INFO,"Starting login job");
			//_beginthreadex(NULL,0,threadLogin,this,0,NULL);
			boost::thread t(threadLogin,this);
		}

};

#endif
