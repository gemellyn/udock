#ifndef __JOB_HTTP_UPLOAD_LOG_H__
#define __JOB_HTTP_UPLOAD_LOG_H__

#include "engine/network/http_manager.h"
#include "engine/network/http_job.h"
#include "engine/log/log.h"



class HttpJobUploadLog : public HttpJob
{
	public:

		HttpJobUploadLog() : HttpJob()
		{
		}

		//In
		std::string _FileName;
		int _UserId;
		//Out
		bool _UploadOk;

		static unsigned int thread(void * jobparam)
		{
			HttpJobUploadLog * job = (HttpJobUploadLog*) jobparam;
			job->_UploadOk = HttpManagerUdock::getInstance()->uploadLogFile(job->_UserId,job->_FileName);
			Log::log(Log::ENGINE_INFO,"End of upload job");
			job->_JobDone = true;
			return 0;
		}

		void launchJob(void)
		{
			_JobRunning = true;
			_JobDone = false;
			Log::log(Log::ENGINE_INFO,"Starting upload job");
			//_beginthreadex(NULL,0,thread,this,0,NULL);
            boost::thread t(thread,this);
		}

};

#endif
