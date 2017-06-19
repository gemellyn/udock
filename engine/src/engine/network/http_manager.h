#ifndef __HTTP_MANAGER_H__
#define __HTTP_MANAGER_H__

#include <curl/curl.h>
#include <boost/thread/mutex.hpp>
#include "engine/utils/ny_utils.h"
#include "engine/log/log.h"

class HttpManager
{
	protected:

		boost::mutex _Mutex;
		HttpManager()
		{
			/*_Mutex = CreateMutex(
				NULL,              // default security attributes
				FALSE,             // initially not owned
				NULL);             // unnamed mutex

			if (_Mutex == NULL)
			{
				Log::log(Log::ENGINE_ERROR,("CreateMutex error: " + toString(GetLastError())).c_str());
			}*/
		}


		CURL * _Curl;

		static std::string _Buffer;
		static char _ErrorBuffer[CURL_ERROR_SIZE];

		static int writer(char *data, size_t size, size_t nmemb,
			std::string *writerData)
		{
			if (writerData == NULL)
				return 0;

			writerData->append(data, size*nmemb);

			return size * nmemb;
		}

		virtual bool init (void)
		{
			CURLcode res;

			//On attend que les autres call soient finis
			/*DWORD dwWaitResult = WaitForSingleObject( _Mutex, INFINITE);
			if(dwWaitResult != WAIT_OBJECT_0)
				return false;*/
            _Mutex.lock();

			_Curl = curl_easy_init();
			if (_Curl == NULL)
			{
				Log::log(Log::ENGINE_ERROR,"Failed to create CURL connection");
				return false;
			}

			res = curl_easy_setopt(_Curl, CURLOPT_ERRORBUFFER, _ErrorBuffer);
			if (res != CURLE_OK)
			{
				Log::log(Log::ENGINE_ERROR,("Failed to set CURL error buffer : "+toString(res)).c_str());
				return false;
			}

			res = curl_easy_setopt(_Curl, CURLOPT_WRITEFUNCTION, writer);
			if (res != CURLE_OK)
			{
				Log::log(Log::ENGINE_ERROR,("Failed to set CURL write function : "+toString(res)).c_str());
				return false;
			}


			res = curl_easy_setopt(_Curl, CURLOPT_WRITEDATA, &_Buffer);
			if (res != CURLE_OK)
			{
				Log::log(Log::ENGINE_ERROR,("Failed to set CURL reception buffer : "+toString(res)).c_str());
				return false;
			}

			_Buffer.clear();

			return true;
		}

		virtual void close()
		{
			curl_easy_cleanup(_Curl);
			_Buffer.clear();

			 _Mutex.unlock();
		}

	public:

};

#endif
