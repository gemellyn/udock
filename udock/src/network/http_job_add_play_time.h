#ifndef __JOB_HTTP_ADD_PLAY_TIME_H__
#define __JOB_HTTP_ADD_PLAY_TIME_H__

#include "engine/network/http_manager.h"
#include "engine/network/http_job.h"
#include "engine/log/log.h"

class HttpJobAddPlayTime : public HttpJob
{
	public:

		HttpJobAddPlayTime() : HttpJob()
		{
		}

		//In
		int _UserId;
		float _Time;

		//Out
		bool _PostOk;

		static unsigned int thread(void * jobparam)
		{
			HttpJobAddPlayTime * job = (HttpJobAddPlayTime*) jobparam;
			job->_PostOk = HttpManagerUdock::getInstance()->addPlayTime(job->_UserId,
																 job->_Time);
			Log::log(Log::ENGINE_INFO,"End of add play time job");
			job->_JobDone = true;
			return 0;
		}

		void launchJob(void)
		{
			_JobRunning = true;
			_JobDone = false;
			Log::log(Log::ENGINE_INFO,"Starting add play time job");
			//_beginthreadex(NULL,0,thread,this,0,NULL);
			boost::thread(thread,this);
		}

};

#endif
