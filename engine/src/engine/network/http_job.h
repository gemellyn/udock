#ifndef __JOB_HTTP_H__
#define __JOB_HTTP_H__

class HttpJob
{
	public:
		std::string _MessageEnCours;
		bool _JobRunning;
		bool _JobDone;

		virtual void launchJob(void) = 0;

		HttpJob()
		{
			_JobRunning = false;
			_JobDone = false;
		}
};

#endif