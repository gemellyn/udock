#include "engine/log/log.h"

bool Log::_HideEngineLog = false;
unsigned long Log::_LastMessageTime =  SDL_GetTicks();
vector<Log*> Log::_Logs;
