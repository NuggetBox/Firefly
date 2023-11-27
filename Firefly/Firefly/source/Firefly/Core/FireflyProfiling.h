#pragma once
#include "Optick/optick.h"
#define OPTICK_OFF

//TODO: Inlämning define
#ifndef FF_INLAMNING
	#ifndef OPTICK_OFF
		#define FF_PROFILEFRAME(...) OPTICK_FRAME(__VA_ARGS__)
		#define FF_PROFILESCOPE(NAME) OPTICK_EVENT_DYNAMIC(NAME)
		#define FF_PROFILEFUNCTION(...) OPTICK_EVENT(__VA_ARGS__)
		#define FF_PROFILETHREAD(NAME) OPTICK_THREAD(NAME)
	#else
		#define FF_PROFILEFRAME(...)
		#define FF_PROFILESCOPE(NAME)
		#define FF_PROFILEFUNCTION(...)
		#define FF_PROFILETHREAD(NAME)
	#endif
#else
	#define FF_PROFILEFRAME(...)
	#define FF_PROFILESCOPE(NAME)
	#define FF_PROFILEFUNCTION(...)
	#define FF_PROFILETHREAD(NAME)
#endif