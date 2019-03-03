//-------------------------------------------------------------------------------------------------
//
// File: Parser.cpp
// Author: Dennis Lang
// Desc: Top level file parsing
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


#include <string>
#include "llclasses.h"
#include "DirectoryFiles.h"

#include "Parser.h"
#include "ParseJava.h"
#include "utils.h"


// ---------------------------------------------------------------------------
// Return true if inPath (filename part) matches pattern in patternList
static bool fileMatches(const string& inPath, const PatternList& patternList)
{
    size_t nameStart = inPath.rfind(DIR_SLASH_CHR) + 1;
    if (patternList.empty() || nameStart == 0 || nameStart == inPath.length())
        return false;
    
    for (size_t idx = 0; idx != patternList.size(); idx++)
        if (regex_match(inPath.begin() + nameStart, inPath.end(), patternList[idx]))
            return true;
    
    return false;
}

// ---------------------------------------------------------------------------
size_t Parser::findClasses(const string& dirname, ClassList& clist, const Presenter& presenter)
{
    DirectoryFiles directory(dirname);
    string fullname;
    string ext;
    
    size_t fileCount = 0;
    while (directory.more())  {
        directory.fullName(fullname);
        if (directory.is_directory()) {
            fileCount += findClasses(fullname, clist, presenter);
        } else if (fullname.length() > 0 && !fileMatches(fullname, ignorePatterns)) {
            if (getExtension(ext, fullname).length() > 1) {
                toLower(ext, ext);
                
                // TODO - have file types register by extension
                if (ext == ".java") {
                    ParseJava parseJava;
                    parseJava.parseJava(fullname.c_str(), clist, presenter);
                    fileCount++;
                }
            }
        }
    }
    return fileCount;
}
