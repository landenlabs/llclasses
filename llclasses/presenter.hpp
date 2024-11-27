//-------------------------------------------------------------------------------------------------
//
//  Presenter.cpp      2-Feb-2019       Dennis Lang
//
//  Parse Java/C++/Swift files and generate class names and class dependence tree.
//
//-------------------------------------------------------------------------------------------------
//
// Author: Dennis Lang - 2019
// http://landenlabs.com/
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
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <memory>

#include "llclasses.hpp"
#include "parser.hpp"
#include "classrel.hpp"
#include "Publish.hpp"



#define LOG_CLASS_LIST 3        // See -l
#define LOG_ADDING_CLASSES 4    // See -l

// =================================================================================================
// Contoller - Present class tree files (nodes)
class Presenter {
public:
    int init(int argc, const char* argv[], const char* version);

    //  Return true if current Presentation settings allow item to be shown.
    //  See -A allClasses
    bool canShow(const RelationPtr relPtr) const;
    bool canShowChildren(const RelationPtr relPtr) const;
    void hasShown(const RelationPtr relPtr) const;

    Parser parser;
    unique_ptr<Publish> publishPtr;


    friend PublishViz;
    friend PublishText;
    friend PublishHtml;


    bool show_names    = false;     // -n
    bool show_tree     = true;      // -t
    bool print         = false;     // Not implemented
    bool vizSplit      = false;     // -Z
    bool needHeader    = true;
    bool allClasses    = false;     // -A
    bool showInterfaces = false;    // -I
    bool importPackage = false;     // Not implemented
    bool tabularList   = false;     // -T
    bool fullPath      = false;     // -F
    unsigned logLevel  = 0;         // -l Log level
    int parseImports   = 0;         // -M=<level>,  Parse imports instead of classes

    StringList titles;
    StringList replacements;    // List of find,replacewith word pairs separted by comma


protected:
    // Output format types.
    enum {GRAPHICS_CHAR = 0, TEXT_CHAR = 1, SPACE_CHAR = 2, HTML_CHAR = 3, JAVA_CHAR = 4, VIZ_CHAR = 5 };
    int  cset          = GRAPHICS_CHAR;
    int  nodesPerFile  = 0; // used by VIZ

    string outPath;
    string codePath;

    ClassList clist;
    mutable unsigned sNodeNum = 1;

    // Internal methods.
    //
    size_t displayInterfaces(const RelationPtr parent_ptr, const char* label = " Interfaces:") const;
    void displayChildren(Indent& indent, size_t fnWidth, size_t fnOffset,
        size_t metaWidth, const RelationPtr parent_ptr) const;
    //size_t displayChildren(unsigned parentNum, size_t width,  const RelationPtr parent_ptr,
    //                        const RelationPtr pparent_ptr) const;
    size_t countChildren(const RelationPtr parent_ptr, const RelationPtr pparent_ptr) const;

    // Replace any matches in str with values in replacements
    void applyReplacements(string& str) const;
};


inline ostream& operator<<(ostream& os, const RelationList& list) {
    for (RelationList::const_iterator iter = list.begin(); iter != list.end(); iter++) {
        RelationPtr child_ptr = *iter;
        os << " " << child_ptr->name;
    }
    return os;
}
