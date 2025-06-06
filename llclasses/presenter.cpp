//-------------------------------------------------------------------------------------------------
//
//  Presenter.cpp      2-Feb-2019       Dennis Lang
//
//  Present reports from parsed files (ex java -> html)
//
//-------------------------------------------------------------------------------------------------
//
// Author: Dennis Lang - 2019
// https://landenlabs.com/
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

// 4291 - No matching operator delete found
#pragma warning( disable : 4291 )

#include <stdio.h>
#include <ctype.h>
#include <fstream>
#include <iostream>
#include <iomanip>

#include <vector>
#include <map>
#include <algorithm>
#include <regex>
#include <memory>

#include "presenter.hpp"
#include "classrel.hpp"
#include "utils.hpp"
#include "SwapStream.hpp"
#include "utils/colors.hpp"
#include "log.hpp"

static const string sPackageTag("package");


// -------------------------------------------------------------------------------------------------
void loadfile(const char* filename, Publish& publish, SetData setPtr) {
    ifstream in(filename);
    std::stringstream buffer;
    buffer << in.rdbuf();
    string str = buffer.str();

    // cerr << "Read file " << filename << " content size=" << str.length() << endl;

    if (str.length() > 0) {
        PublishHtml* publishHtmlPtr = dynamic_cast<PublishHtml*>(&publish);
        if (publishHtmlPtr != NULL) {
            (publishHtmlPtr->*setPtr)(str);
        } else {
            cerr << "Loaded file ignored, wrong Publish type " << typeid(publish).name() << endl;
            cerr << "Try and specify -j before html file sections" << endl;
        }
    }
}

void showHelp(const char* argv0, const char* version) {
    cerr << "\n" << argv0 << " ("
        << version << ")\n"
        <<  Colors::colorize(
    "\nDes: _P_Generate class dependence tree_X_ (" __DATE__ ")"
    "\nBy:  _W_LanDen Labs  www.landenlabs.com _X_"
    "\nUse: llclasses [-+ntgxshjz] header_files...\n"
    "\nSwitches (*=default)(-=off, +=on):"
    "\n  _y_n  ; Show alphabetic class name list"
    "\n* _y_t  ; Show class dependency tree"
    "\n"
    "\n_P_Output format (single choice):_X_"
    "\n* _y_g  ; Use graphics for tree connections"
    "\n  _y_x  ; Use (+|-) for tree connections"
    "\n  _y_s  ; Use spaces for tree connections"
    "\n  _y_h  ; Html tree connections (needs images 0.png, 1.png, 2.png, n.png)"
    "\n  _y_j  ; Java tree connections (needs dtree www.destroydrop.com/javascript/tree/)"
    "\n  _y_z  ; GraphViz (see https://graphviz.gitlab.io/)"
    "\n"
    "\n  With -_y_j to customize java _y_1=header, _y_2=body begin, _y_3=body end"
    "\n     Specify file which contains html text to insert in output"
    "\n  -_y_1=head.html -_y_2=bodybegin.html -_y_3=bodyend.html"
    "\n     Optional replacement words applied to html sections"
    "\n  -_y_0=x1,foo1 -_y_0=x2,foo2 -_y_0=x3,foo3"
    "\n"
    "\n_P_Modifiers:_X_"
    "\n  _y_Z               ; Split GraphViz by tree, use with -O"
    "\n  _y_N =nodesPerFile ; Split by nodes per file, use with -O"
    "\n  _y_O =outpath      ; Save output in file"
    "\n  _y_T =tabular      ; Tabular html "
    "\n  _y_V =filePattern  ; Ignore file path pattern, ex *Test*"
    "\n  _y_v =classPattern ; Ignore class pattern, ex [Tt]est[0-9]"
    "\n  _y_A =allClasses   ; Include Protected and Private classes"
    "\n  _y_M =level        ; Parse imports instead of Classes"
    "\n                       level +n=keep left n levels, -n=remove last n levels"
    "\n  _y_I =interfaces   ; Include Interfaces in report"
    "\n  _y_i =classPattern ; Include class pattern, opposite of -v"
            //      "\n  _y_F =full path    ; Defaults to relative"
    "\n  _y_L =Title        ; Set optional title"
    "\n  _V_l =logLevel     ; Log level (0=default), -l=2 show extra logging"
    "\n"
    "\n_P_Examples (assumes java source code in directory src):_X_"
    "\n  llclasses -t +n  src\\*.java  ; *.java prevent recursion"
    "\n  llclasses -x  src > llclasses.txt"
    "\n  llclasses -h  dir1 dir2 foo*.java  > llclasses.html"
    "\n  llclasses -h -I -T \"-v=*Test*\" src > javaTable.html"
    "\n  llclasses -j  src > llclassesWithJs.html"
    "\n  llclasses -j -1=head.html -2=bodybegin.html -3=bodyend.html src > jtrees.html"
    "\n"
    "\n  -V is case sensitive "
    "\n  llclasses -z -Z -O=.\\viz\\ -V=*Test* -V=*Exception* src >directgraph.dot"
    "\n  llclasses -z -N=10 -O=.\\viz\\ -V=*Test* -V=*Exception* src >directgraph.dot"
    "\n");
}

// =================================================================================================
int Presenter::init(int argc, const char* argv[], const char* version) {
    unsigned fileCnt = 0;
    int returnVal = -1;
    publishPtr = std::make_unique<PublishText>(clist, *this);    // Set default publisher

    if (argc == 1) {
        showHelp(argv[0], version);
        return 0;
    } else {
        string title;
        for (int argn = 1; argn < argc; argn++) {
            if (*argv[argn] == '-' || *argv[argn] == '+') {
                bool polarity = (*argv[argn] == '+');
                size_t argLen = strlen(argv[argn]);
                switch (argv[argn][1]) {
                    case '0':   // -0=replacementStrings
                        if (argLen > 3)
                            replacements.push_back(argv[argn] + 3);
                        break;
                    case '1':   // -1=headerfile
                        if (argLen > 3)
                            loadfile(argv[argn] + 3, *publishPtr, &PublishHtml::SetHead);
                        break;
                    case '2':
                        if (argLen > 3)
                            loadfile(argv[argn] + 3, *publishPtr, &PublishHtml::SetBodyBegin);
                        break;
                    case '3':
                        if (argLen > 3)
                            loadfile(argv[argn] + 3, *publishPtr, &PublishHtml::SetBodyEnd);
                        break;
                        
                    case 'l':
                        if (argLen > 3)
                            logLevel = (int)strtol(argv[argn] + 3, NULL, 10);
                        else
                            logLevel++;  // Increase log level
                        Log::W_LEVEL = logLevel;
                        break;
                        
                    case 'L':   // Set Title (label)
                        if (argLen > 3)
                            titles.push_back(argv[argn] + 3);
                        break;
                        
                        // Base options
                    case 'n': show_names = polarity;       break;
                    case 't': show_tree = polarity;       break;
                        
                        // Output format
                    case 'g': cset = Presenter::GRAPHICS_CHAR;  break;
                    case 'x': cset = Presenter::TEXT_CHAR;      break;
                    case 's': cset = Presenter::SPACE_CHAR;     break;
                    case 'h': cset = Presenter::HTML_CHAR;
                        publishPtr = std::make_unique<PublishHtml>(clist, *this);
                        break;
                    case 'j': cset = Presenter::JAVA_CHAR;
                        publishPtr = std::make_unique<PublishHtml>(clist, *this);
                        break;
                    case 'z': cset = Presenter::VIZ_CHAR;
                        publishPtr = std::make_unique<PublishViz>(clist, *this);
                        break;
                        
                        // Modifiers
                    case 'A': allClasses = true;        break;
                    case 'F': fullPath = true;          break;
                    case 'I': showInterfaces = true;    break;
                    case 'M': allClasses = true;
                        if (argLen > 3)
                            parseImports = (int)strtol(argv[argn] + 3, NULL, 10);
                        else
                            parseImports = 1000;    // -M = keep entire import
                        break;
                        
                    case 'T': tabularList = true;
                        publishPtr = std::make_unique<PublishHtml>(clist, *this);
                        break;
                    case 'Z': vizSplit = true;          break;
                    case 'N':
                        if (argLen > 3)
                            nodesPerFile = (int)strtol(argv[argn] + 3, 0, 10);
                        break;
                    case 'O':   // -O=<outPath>
                        if (argLen > 3)
                            outPath = argv[argn] + 3;
                        break;
                    case 'v':  // -v=<pattern>   ignore list of class patterns
                        if (argLen > 3) {
                            string str = argv[argn] + 3;
                            replaceAll(str, "*", ".*");
                            clist.ignoreClassPatterns.push_back(regex(str));
                        }
                        break;
                    case 'V':  // -V=<path_RegEx_pattern>   ignore list of path patterns
                        if (argLen > 3) {
                            string str = argv[argn] + 3;
                            replaceAll(str, "*", ".*");
                            parser.ignorePathPatterns.push_back(regex(str));
                        }
                        break;
                    case 'i':  // -i=<pattern>   include list of class patterns
                        if (argLen > 3) {
                            string str = argv[argn] + 3;
                            replaceAll(str, "*", ".*");
                            clist.includeClassPatterns.push_back(regex(str));
                        }
                        break;
                        
                    default:
                        std::cerr << "Uknown option:" << argv[argn] << std::endl;
                    case '?':
                        showHelp(argv[0], version);
                        return 0;
                }
            } else {
                string codePath = argv[argn];
                fileCnt += parser.findClasses(codePath, clist, *this);
                if (title.empty())
                    title = codePath;
            }
        }

        returnVal = (int)clist.size();
        cerr << "llclasses - Files read:" << fileCnt << " classes extracted:" << clist.size() << endl;


        if (! clist.empty()) {
            publishPtr->present();

            clist.release();
        }
        cerr << endl;
    }

    return returnVal;   // Return parsed class count, else -1
}

const unsigned SHOWN = 1;
// -------------------------------------------------------------------------------------------------
bool Presenter::canShow(const RelationPtr relPtr) const {
    if ((relPtr->flags & SHOWN) == SHOWN)
        return false;

    if (relPtr->type == "class")
        return (allClasses || relPtr->modifier.find("public") != string::npos);
    else
        return showInterfaces;
}

// -------------------------------------------------------------------------------------------------
bool Presenter::canShowChildren(const RelationPtr relPtr) const {
    if ((relPtr->flags & SHOWN) == SHOWN)
        return false;

    for (const RelationPtr childPtr : relPtr->children) {
        if (relPtr == childPtr)
            return true;
        if (canShowChildren(childPtr))
            return true;
    }
    return canShow(relPtr);
}

// -------------------------------------------------------------------------------------------------
void Presenter::hasShown(const RelationPtr relPtr) const {
    relPtr->flags |= SHOWN;
}

// -------------------------------------------------------------------------------------------------
size_t Presenter::displayInterfaces(
    const RelationPtr relPtr,
    const char* label) const {
    size_t nodeCnt = 0;
    if (relPtr != NULL) {
        // [color=red,penwidth=3.0]
        const RelationList& interfaces = relPtr->interfaces;
        const char* sep = label; // " Interfaces:";
        for (RelationList::const_iterator iter = interfaces.begin(); iter != interfaces.end(); iter++) {
            string name = (*iter)->name;
            {
                cout << sep << name;
                sep = ", ";
            }
            nodeCnt++;
        }
    }

    return nodeCnt;
}

// -------------------------------------------------------------------------------------------------
void Presenter::applyReplacements(string& str) const {
    for (string val : replacements) {
        Split split(val, ",");
        if (split.size() == 2) {
            replaceAll(str, split[0], split[1]);
        }
    }
}

/*

 // -------------------------------------------------------------------------------------------------
 // Convert DOS filename to Browser url file locator.
 // Ex: \dir\subdir\file.ext => file:///dir/subdir/file.ext
 static string filePathToURL(string filepath)
 {
 static string  diskDrive("file:///");

 for (unsigned idx = 0; idx < filepath.length(); idx++)
 if (filepath[idx] == '\\')
 filepath[idx] = '/';

 return diskDrive + filepath;
 }


*/
