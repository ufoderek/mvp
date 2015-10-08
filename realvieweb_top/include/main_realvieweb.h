#ifndef _MAIN_REALVIEWEB_H_
#define _MAIN_REALVIEWEB_H_

#include <debug_utils.h>

#ifndef RELEASE
void init_debug_units();
#else
#define init_debug_units() {;}
#endif

#endif
