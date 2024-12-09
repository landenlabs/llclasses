//-------------------------------------------------------------------------------------------------
//
//  llclass      3-Feb-2019        Dennis Lang
//
//  Parse Java/Cpp files and generate class names and class dependence tree.
//
//  Created by Dennis Lang on 3-Feb-2019
//  Copyright © 2019 Dennis Lang. All rights reserved.
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(_WIN32) || defined(_WIN64)
#define HAVE_WIN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS   // define before all includes
typedef unsigned long DWORD;
#else
typedef unsigned int  DWORD;
#endif

#include <string>
#include <vector>
#include <regex>

using namespace std;

typedef vector<regex> PatternList;
typedef vector<string> Indent;
typedef vector<string> StringList;

#ifdef HAVE_WIN
    static char DIR_SLASH_CHR      = '\\';
    static const string DIR_SLASH_STR = "\\";
#else
    static char DIR_SLASH_CHR      = '/';
    static const string DIR_SLASH_STR = "/";
#endif
