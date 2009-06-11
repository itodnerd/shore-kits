/* -*- mode:C++; c-basic-offset:4 -*-
     Shore-kits -- Benchmark implementations for Shore-MT
   
                       Copyright (c) 2007-2009
               Ecole Polytechnique Federale de Lausanne

                       Copyright (c) 2007-2008
                      Carnegie-Mellon University

                         All Rights Reserved.
   
   Permission to use, copy, modify and distribute this software and
   its documentation is hereby granted, provided that both the
   copyright notice and this permission notice appear in all copies of
   the software, derivative works or modified versions, and any
   portions thereof, and that both notices appear in supporting
   documentation.
   
   This code is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. THE AUTHORS
   DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER
   RESULTING FROM THE USE OF THIS SOFTWARE.
*/

/** @file:   prcinfo.h
 * 
 *  @brief:  Processor usage information
 * 
 *  @author: Ippokratis Pandis, Nov 2008
 */


/*
 * Prints all field resource, usage and microstat accounting fields
 *
 * @note: Taken from -  http://safari.oreilly.com/0131482092/app02lev1sec2
 *
 */

#ifndef __UTIL_PRCINFO_H
#define __UTIL_PRCINFO_H

#include <sys/types.h>
#include <sys/time.h>
#include <procfs.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>

#ifdef __SUNPRO_CC
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#else
#include <cstdio>
#include <cstring>
#include <cstdlib>
#endif

#include "util/stopwatch.h"

#include <iostream>

using namespace std;


struct processinfo_t 
{
    int _fd;

    prusage_t   _old_prusage;
    prusage_t   _prusage;
    stopwatch_t _timer;

    bool _print_at_exit;
    int _is_ok;

    processinfo_t(const bool printatexit = false);
    ~processinfo_t();

    // prints information and resets
    const int print();
    const int reset();

    static void hr_min_sec(char*, long);
    static void prtime(string label, timestruc_t* ts);
    static void prtime(string label, long long& delay);

    static void tsadd(timestruc_t* result, timestruc_t* a, timestruc_t* b);
    static void tssub(timestruc_t* result, timestruc_t* a, timestruc_t* b);

}; // EOF: processinfo_t

#endif /** __UTIL_PRCINFO_H */
