//-------------------------------------------------------------------------------------------------
//
// File: Colors.h
// Author: Dennis Lang
// Desc: Terminal colors
//
//-------------------------------------------------------------------------------------------------
//
// Author: Dennis Lang - 2020
// http://landenlabs.com
//
// This file is part of llcsv project.
//
// ----- License ----
//
// Copyright (c) 2020 Dennis Lang
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include <string>

using namespace std;

//-------------------------------------------------------------------------------------------------
// Replace using regular expression
inline string& replaceRE(string& inOut, const char* findRE, const char* replaceWith)
{
    regex pattern(findRE);
    regex_constants::match_flag_type flags = regex_constants::match_default;
    inOut = regex_replace(inOut, pattern, replaceWith, flags);
    return inOut;
}

class Colors
{
public:
#ifdef HAVE_WIN
	#define RED    "\033[01;31m"
	#define GREEN  "\033[01;32m"
	#define YELLOW "\033[01;33m"
	#define BLUE   "\033[01;34m"
	#define PINK   "\033[01;35m"
	#define LBLUE  "\033[01;36m"
	#define WHITE  "\033[01;37m"
	#define OFF    "\033[00m"
#else
    #define RED    "\033[01;31m"
    #define GREEN  "\033[01;32m"
    #define YELLOW "\033[01;33m"
    #define BLUE   "\033[01;34m"
    #define PINK   "\033[01;35m"
    #define LBLUE  "\033[01;36m"
    #define WHITE  "\033[01;37m"
    #define OFF    "\033[00m"
#endif
    
    static string colorize(const char* inStr)
    {
        string str(inStr);
        
        // _x_  where x lowercase, colorize following word
        replaceRE(str, "_y_(\\w+)", YELLOW "$1" OFF);
        replaceRE(str, "_r_(\\w+)",    RED "$1" OFF);
        replaceRE(str, "_g_(\\w+)",  GREEN "$1" OFF);
        replaceRE(str, "_p_(\\w+)",   PINK "$1" OFF);
        replaceRE(str, "_lb_(\\w+)", LBLUE "$1" OFF);
        replaceRE(str, "_w_(\\w+)",  WHITE "$1" OFF);
        
        // _X_  where X uppercase, colorize until _X_
        replaceRE(str, "_r_", YELLOW);
        replaceRE(str, "_R_", RED);
        replaceRE(str, "_r_", GREEN);
        replaceRE(str, "_P_", PINK);
        replaceRE(str, "_B_", BLUE);
        replaceRE(str, "_LB_", LBLUE);
        replaceRE(str, "_W_", WHITE);
        replaceRE(str, "_X_", OFF);
        return str;
    }
};


