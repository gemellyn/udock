#ifndef __JOB_HTTP_POST_SCORES_H__
#define __JOB_HTTP_POST_SCORES_H__

#include "network/http_manager_udock.h"
#include "engine/network/http_job.h"
#include "engine/log/log.h"



class HttpJobPostScores : public HttpJob
{
	public:

		HttpJobPostScores() : HttpJob()
		{
		}

		//In
		int _IdMol1;
		int _IdMol2;
		float _Score;
		int _UserId;
		Positionator _Pos1;
		Positionator _Pos2;
		//Out
		bool _PostOk;

		static unsigned int thread(void * jobparam)
		{
			HttpJobPostScores * job = (HttpJobPostScores*) jobparam;
			job->_PostOk = HttpManagerUdock::getInstance()->postScore(job->_UserId,
																 job->_IdMol1,
																 job->_IdMol2,
																 job->_Score,
																 job->_Pos1,
																 job->_Pos2);
			Log::log(Log::ENGINE_INFO,"End of upload job");
			job->_JobDone = true;
			return 0;
		}

		void launchJob(void)
		{
			_JobRunning = true;
			_JobDone = false;
			Log::log(Log::ENGINE_INFO,"Starting post scores job");
			//_beginthreadex(NULL,0,thread,this,0,NULL);
			boost::thread t(&thread,this);
		}

};

#endif
