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
#include "llclasses.hpp"
#include "directoryfiles.hpp"

#include "parser.hpp"
#include "parsejava.hpp"      // TODO - have parsers register
#include "parsecpp.hpp"       // TODO - have parsers register
#include "utils.hpp"
#include "log.hpp"


// ---------------------------------------------------------------------------
size_t Parser::findClasses(const string& dirname, ClassList& clist, const Presenter& presenter) {
    DirectoryFiles directory(dirname);
    string fullname;
    string ext;

    size_t fileCount = 0;
    while (directory.more())  {
        directory.fullName(fullname);
        if (directory.is_directory()) {
            fileCount += findClasses(fullname, clist, presenter);
        } else if (fullname.length() > 0 && ! matchesRE(fullname, ignorePathPatterns)) {
            if (getExtension(ext, fullname).length() > 1) {
                toLower(ext, ext);

                if (DirectoryFiles::isFile(fullname)) {
                    // TODO - have file types register by extension
                    if (ext == ".java") {
                        ParseJava parseJava;
                        parseJava.parseCode(fullname.c_str(), clist, presenter);
                        fileCount++;
                    } else if (ext == ".hpp" || ext == ".hpp") {
                        ParseCpp parseCpp;
                        parseCpp.parseCode(fullname.c_str(), clist, presenter);
                        fileCount++;
                    }
                } else {
                    Log::error(LOG_BAD_FILE).out() << "Invalid file " << fullname;
                }
            }
        } else {
            Log::warning(LOG_IGNORE).out() << "Ignoring file:" << fullname;
        }
    }

    return fileCount;
}
