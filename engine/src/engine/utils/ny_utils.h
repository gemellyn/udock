#ifndef __NY_UTILS__
#define __NY_UTILS__

/*
#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <process.h>
#include <psapi.h>
#include <math.h>
#include <assert.h>
#include <conio.h>
#include <cstdio>
*/

#include "types.h"

#include <string>
//#include <strstream>
#include <sstream>
#include <iostream>
#include <limits>
#include <vector>
#include <locale>
#include <stdlib.h>
#include <algorithm>

#include "boost/date_time/posix_time/posix_time.hpp"
//#include <codecvt>

using namespace std;

#ifndef nynan
#define nynan(x) (x!=x)
#endif

#ifndef nymax
#define nymax(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef nymin
#define nymin(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef nysign
#define nysign(a)            (((a) >= (0)) ? (1) : (-1))
#endif

#ifndef nyabs
#define nyabs(a)            (((a) >= (0)) ? (a) : -(a))
#endif



template<class T>
inline T nylerp(float facteur, T start, T end)
{
	return (T)(facteur * (float)end) + (T)((1.0f - facteur) * (float)start);

}

///Avoir une valeur aléatoire entre 0 et max
inline double randf(void)
{
	double rnd = rand();
	rnd /= RAND_MAX;
	return rnd;
}

template<class T>
inline T round(T val)
{
	T rest;
	modf(val,&rest);
	if (rest > 0.5)
		return ceil(val);
	return floor(val);
}

/// Nettoyage d'un pointeur (cree avec new)
#define SAFEDELETE(a) { if (a) {delete a; a = NULL; }}

/// Nettoyage d'un tableau (cree par new[])
#define SAFEDELETE_TAB(a) { if (a) {delete [] a; a = NULL; }}

///Connaitre la taille d'un fichier
uint32 getFileSize(const char* name);

template<class T>
string toString(const T& val)
{
    stringstream strm;
    strm << val;
    return strm.str();
}

template<class T>
wstring toWString(const T& val)
{
	wstringstream strm;
	strm << val;
	return strm.str();
}

template<typename T>
T stringTo( const std::string& s )
{
	std::istringstream iss(s);
	T x;
	iss >> x;
	return x;
}

//string ws_to_s(wstring in);

std::wstring utf8_to_wstring(const std::string& str);
std::string wstring_to_utf8(const std::wstring& str);

inline void splitString (std::vector<string> & elements, string & toSplit, char delim)
{
	std::stringstream ss(toSplit);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elements.push_back(item);
	}
}


inline unsigned long getLocalNowInSeconds()
{
	boost::posix_time::ptime time_t_epoch(boost::gregorian::date(1970, 1, 1));
	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	boost::posix_time::time_duration diff = now - time_t_epoch;
	//x = diff.total_milliseconds();

	//boost::posix_time::ptime t1 = boost::posix_time::second_clock::local_time();
	return diff.total_seconds();
}

inline string getLocalNowString()
{
	std::string s;
	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	s = toString(static_cast<int>(now.date().year()))  + toString(now.date().month()) + toString(now.date().day());
	return s;
}


uint16 fgetu16 (FILE * fe);

uint32 fgetu32 (FILE * fe);

bool isPowerOfTwo(uint32 value);

unsigned long getMemoryUsage(void);

void nyStartTimer(void);

float nyGetElapsedTimer(void);

template<typename T>
float ny_sign(T val)
{
	if(val >= 0)
		return 1.0f;
	else
		return -1.0f;

}

#endif
