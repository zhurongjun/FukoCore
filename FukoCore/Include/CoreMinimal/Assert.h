#pragma once
#include "CoreConfig.h"
#include <assert.h>

#if FUKO_DEBUG

#define check(InExpr) assert(InExpr);
#define checkf(InExpr, InFormat, ...) assert(InExpr);
#define checkNoEntry() assert(false);
#define checkNoReentry() { static bool __Check__ = false; if (__Check__) assert(false); __Check__ = true; }

#else

#define check(InExpr)
#define checkf(InExpr, InFormat, ...)
#define checkNoEntry()
#define checkNoReentry()

#endif

#define always_check(InExpr) assert(InExpr);
#define always_checkf(InExpr, InFormat, ...) assert(InExpr);
#define always_checkNoEntry() assert(false);
#define always_checkNoReentry() { static bool __Check__ = false; if (__Check__) assert(false); __Check__ = true; }

#define ensure(InExpr)
#define ensuref(InExpr,InFormat, ... )