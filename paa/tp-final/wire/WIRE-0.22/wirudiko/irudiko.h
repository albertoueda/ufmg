/* Default settings for Irudiko (0.5) */
#ifndef _IRUDIKO_H
#define _IRUDIKO_H

#include <config.h>

// Base settings (you can personalize it as far as you want!)
#define IRUDIKO_SKETCHSIZE     100
#define IRUDIKO_SELECTIONTYPE  2   // 1: Min function; 2: Mod function; 3: None
#define IRUDIKO_SELECTIONPARAM 0   // no param

// Header files (you need all of them)
#include "irudikosimplereader.h"

// Type redefinitions (WIRE implementation)
typedef unsigned long irudiko_t;
typedef struct {
  irudiko_t sketch[IRUDIKO_SKETCHSIZE];
} irudiko_sketch_t;

#endif
