#include "engine/network/http_manager.h"

std::string HttpManager::_Buffer;
char HttpManager::_ErrorBuffer[CURL_ERROR_SIZE]; 
