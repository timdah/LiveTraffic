//
//  LTVersion.cpp
//  LiveTraffic

/*
 * Copyright (c) 2018, Birger Hoppe
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "LiveTraffic.h"

//
// MARK: Version Information (CHANGE VERSION HERE)
//
#define LIVETRAFFIC_VERSION_NUMBER "0.8"

//
// MARK: global variables referred to via extern declarations in Constants.h
//
char szLT_VERSION_FULL[30] = LIVETRAFFIC_VERSION_NUMBER ".";

const char* LT_VERSION = LIVETRAFFIC_VERSION_NUMBER;
const char* LT_VERSION_FULL = szLT_VERSION_FULL;
const char* HTTP_USER_AGENT = LIVE_TRAFFIC "/" LIVETRAFFIC_VERSION_NUMBER;

//
// As __DATE__ has weird format
// we fill the internal buffer once...need to rely on being called, though
//

const char* InitFullVersion ()
{
    // Example of __DATE__ string: "Nov 12 2018"
    //                              01234567890
    char buildDate[12] = __DATE__;
    buildDate[3]=0;                                     // separate elements
    buildDate[6]=0;
    strcat_s(szLT_VERSION_FULL, sizeof(szLT_VERSION_FULL), buildDate + 9);  // year (last 2 digits)
    strcat_s(szLT_VERSION_FULL, sizeof(szLT_VERSION_FULL),                  // month converted to digits
           strcmp(buildDate,"Jan") == 0 ? "01" :
           strcmp(buildDate,"Feb") == 0 ? "02" :
           strcmp(buildDate,"Mar") == 0 ? "03" :
           strcmp(buildDate,"Apr") == 0 ? "04" :
           strcmp(buildDate,"May") == 0 ? "05" :
           strcmp(buildDate,"Jun") == 0 ? "06" :
           strcmp(buildDate,"Jul") == 0 ? "07" :
           strcmp(buildDate,"Aug") == 0 ? "08" :
           strcmp(buildDate,"Sep") == 0 ? "09" :
           strcmp(buildDate,"Oct") == 0 ? "10" :
           strcmp(buildDate,"Nov") == 0 ? "11" :
           strcmp(buildDate,"Dec") == 0 ? "12" : "??"
           );
    strcat_s(szLT_VERSION_FULL, sizeof(szLT_VERSION_FULL), buildDate + 4);           // day

    return szLT_VERSION_FULL;
}