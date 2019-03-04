//
//  colors.h
//  llclasses
//
//  Created by Dennis Lang on 3/3/19.
//  Copyright © 2019 Dennis Lang. All rights reserved.
//

#pragma once

#include <string>
using namespace std;

class Colors
{
#ifdef HAVE_WIN
    const char RED[] =    "\033[01;31m";
    const char GREEN[] =  "\033[01;32m";
    const char YELLOW[] = "\033[01;33m";
    const char BLUE[] =   "\033[01;34m";
    const char PINK[] =   "\033[01;35m";
    const char LBLUE[] =  "\033[01;36m";
    const char WHITE[] =  "\033[01;37m";
    const char OFF[] =    "\033[00m";
#else
    const char RED[] =    "";
    const char GREEN[] =  "";
    const char YELLOW[] = "";
    const char BLUE[] =   "";
    const char PINK[] =   "";
    const char LBLUE[] =  "";
    const char WHITE[] =  "";
    const char OFF[] =    "";
#endif
    
    string colorize(const char* inStr)
    {
        string str(inStr);
        return replaceRE(inStr, "_y_([^ ]+)", YELLOW "\1"  OFF);
    }
};
