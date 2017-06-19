#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "session/player_info.h"
#include "engine/log/log.h"

#ifndef NULL
#define NULL 0
#endif

#define UDOCK_VERSION "1.2.0"
#define UDOCK_OVERALL_VERSION 1
#define UDOCK_MAJOR_VERSION 2
#define UDOCK_MINOR_VERSION 0


//#define PUBLIC_ONLINE 1
#define PUBLIC_DECO 1
//#define TOTAL_DECO 1

#ifdef PUBLIC_ONLINE
#ifdef PUBLIC_DECO
#error "Cannot define public deco and online at the same time : please select only one mode"  
#endif
#endif

#define PUBLIC_RELEASE 1


 
#endif 