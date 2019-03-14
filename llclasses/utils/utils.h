//-------------------------------------------------------------------------------------------------
//
// File: utils.h
// Author: Dennis Lang
// Desc: Utility classes
//
//-------------------------------------------------------------------------------------------------
//
// Author: Dennis Lang - 2019
// http://landenlabs.com
//
// This file is part of llclasses project.
//
// ----- License ----
//
// Copyright (c) 2019 Dennis Lang
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


#include "llclasses.h"
#include <map>
#include <limits>


// =================================================================================================
// Split string into parts.
class Split : public vector<string>
{
public:
    Split() {
    }
    
    // Examples
    //   Split args(inString, ",", false, 2);
    //   Split args(inString, ",");
    Split(const string& str, const char* delimList, bool keepEmpty=true, int maxSplit=numeric_limits<int>::max())
    {
        size_t lastPos = 0;
        size_t pos = str.find_first_of(delimList);
        
        while (pos != string::npos && --maxSplit > 0)
        {
            if (keepEmpty || pos != lastPos)
                push_back(str.substr(lastPos, pos - lastPos));
            lastPos = pos + 1;
            pos = str.find_first_of(delimList, lastPos);
        }
        if (lastPos < str.length())
            push_back(str.substr(lastPos, (maxSplit == 0) ? str.length() : pos - lastPos));
    }
    
    // Split string into parts.
    //
    // Example:
    //
    //  Split split(parents, ", ", FindSplit);
    //
    //  size_t FindSplit(const string& str, const char* delimList, int begIdx)
    //  {
    //     size_t off = strcpsn(str+begIdx, delimList);
    //     return (off== lstring::npos) ? lstring::npos : off + begIdx;
    //  }
    //
    //
    //
    typedef size_t(*Find_of)(const string& str, const char* delimList, size_t begIdx);
    
    Split(const string& str, const char* delimList, Find_of find_of)
    {
        size_t lastPos = 0;
        // size_t pos = str.find_first_of(delimList);
        size_t pos = (*find_of)(str, delimList, 0);
        
        while (pos != string::npos)
        {
            if (pos != lastPos)
                push_back(str.substr(lastPos, pos - lastPos));
            lastPos = pos + 1;
            // pos = str.find_first_of(delimList, lastPos);
            pos = (*find_of)(str, delimList, lastPos);
        }
        if (lastPos < str.length())
            push_back(str.substr(lastPos, pos - lastPos));
    }

};


// C O N T A I N E R   H E L P E R S
// =================================================================================================
inline bool contains(const vector<string>& vec, const string& findStr) {
    return find(vec.begin(), vec.end(), findStr) != vec.end();
}

template <typename tValue>
bool containsKey(const map<string, tValue>& mapData, const string& findStr) {
    return mapData.find(findStr) != mapData.end();
}
template <typename tValue, typename dValue>
tValue getIt(const vector<tValue>& list, unsigned idx, dValue defValue) {
    return (idx < list.size()) ? list[idx] : defValue;
}

// S T R I N G   H E L P E R S
// ===============================================================================================
// Return common consecutive sequence.
inline string equalSubStr(const string& left, const string& right) {
    size_t idx = 0;
    size_t len = min(left.length(), right.length());
    if (len == 0)
        return (left.length() > right.length()) ? left : right;
    
    while (idx < len && left[idx] == right[idx])
        idx++;
    
    return left.substr(0, idx);
}

//-------------------------------------------------------------------------------------------------
// Remove leading and trailing spaces.
// Convert tab to space
// Compress multiple spaces to one
inline string trim(string& inOut) {
    unsigned oIdx = 0;
    unsigned iIdx = 0;
    unsigned len = (unsigned)inOut.length();
    while (iIdx < len && isspace(inOut[iIdx]) ) {
        iIdx++;
    }
    while (len > iIdx && isspace(inOut[len-1])) {
        len--;
    }
    while (iIdx < len) {
        char chr = inOut[iIdx++];
        inOut[oIdx++] = isspace(chr) ? ' ' : chr;    // Force tab to spc
        // Compress multiple spaces to one.
        if (isspace(chr)) {
            while (iIdx < len && isspace(inOut[iIdx])) {
                iIdx++;
            }
        }
    }
    inOut.resize(oIdx);
    return inOut;
}

//-------------------------------------------------------------------------------------------------
// Join vector of strings:  aa, bb, cc, dd
// join(vec, "-", "")  out = aa-bb-cc-dd
// join(vec, "-", "-") out = aa-bb-cc-dd-  
template<typename TT, typename STR>
inline string join(TT list, STR separator, STR terminator)
{
    string result;
    string sep = "";
    for (const auto & piece : list)
    {
        result += sep;
        result += piece;
        sep = separator;
    }
    if (!result.empty())
        result += terminator;
 
    return result;
}

//-------------------------------------------------------------------------------------------------
// Replace (modify inOut) all occurances of 'find' with 'replace'
inline string& replaceAll(string& inOut, const string& find, const string& replace)
{
    size_t pos = 0;
    while ((pos = inOut.find(find, pos)) != string::npos)
    {
        inOut.replace(pos, find.length(), replace);
        pos += replace.length();
    }
    return inOut;
}

//-------------------------------------------------------------------------------------------------
// Replace using regular expression
inline string& replaceRE(string& inOut, const char* findRE, const char* replaceWith)
{
    regex pattern(findRE);
    regex_constants::match_flag_type flags = regex_constants::match_default;
    inOut = regex_replace(inOut, pattern, replaceWith, flags);
    return inOut;
}


// ---------------------------------------------------------------------------
// Return true if inStr  matches pattern in patternList
inline bool matchesRE(const string& inStr, const PatternList& patternList)
{
    // size_t nameStart = inStr(DIR_SLASH_CHR) + 1;
    if (patternList.empty() /* || nameStart == 0 || nameStart == inStr() */)
        return false;
    
    for (size_t idx = 0; idx != patternList.size(); idx++)
        if (regex_match(inStr.begin() /* + nameStart*/, inStr.end(), patternList[idx]))
            return true;
    
    return false;
}

//-------------------------------------------------------------------------------------------------
inline bool hasExtension(const string& filepath, const char* extn)
{
    size_t pos = filepath.rfind(extn);
    return (pos != string::npos) && (filepath.length() == pos + strlen(extn));
}

//-------------------------------------------------------------------------------------------------
inline string& getExtension(string& outExt, const string& filename)
{
    size_t pos = min(filename.rfind('.'), filename.length());
    outExt = filename.substr(pos);
    return outExt;
}

//-------------------------------------------------------------------------------------------------
inline string& toLower(string& outStr, const string& inStr) {
    transform(inStr.begin(), inStr.end(), outStr.begin(), ::tolower);
    return outStr;
}

// =================================================================================================
#include <sstream>

//  dateTimeToString(now(), "%A %B, %d %Y %I:%M%p");
//     Sunday July, 24 2005 05:48PM
//  dateTimeToString(now(), "%Y-%m-%d %H:%M:%S");
//     2005-07-24 17:48:11
inline string dateTimeToString(const tm& t, const char* fmt) {
    stringstream out;
    const time_put<char>& dateWriter = use_facet<time_put<char> >(out.getloc());
    size_t n = strlen(fmt);
    if (dateWriter.put(out, out, ' ', &t, fmt, fmt + n).failed()) {
        // throw runtime_error("failure to format date time");
        return "Failed to format date time";
    }
    return out.str();
}

inline tm now() {
    time_t now = time(0);
#ifdef HAVE_WIN
	tm now_tm;
	localtime_s(&now_tm, &now);
	return now_tm;
#else
    return *localtime(&now);
#endif
}



