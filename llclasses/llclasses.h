
#pragma once

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
