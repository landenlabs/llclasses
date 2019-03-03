//-------------------------------------------------------------------------------------------------
//
//  Presenter.cpp      2-Feb-2019       Dennis Lang
//
//  Present reports from parsed files (ex java -> html)
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

#include "Presenter.h"
#include "ClassRel.h"
#include "utils.h"
#include "SwapStream.h"

static const string sPackageTag("package");

// -------------------------------------------------------------------------------------------------
void loadfile(const char* filename, Publish& publish, SetData setPtr)
{
    ifstream in(filename);
    std::stringstream buffer;
    buffer << in.rdbuf();
    string str = buffer.str();
    
    // cerr << "Read file " << filename << " content size=" << str.length() << endl;
    
    if (str.length() > 0)
    {
        PublishHtml* publishHtmlPtr = dynamic_cast<PublishHtml*>(&publish);
        if (publishHtmlPtr != NULL)
        {
            (publishHtmlPtr->*setPtr)(str);
        } else {
            cerr << "Loaded file ignored, wrong Publish type " << typeid(publish).name() << endl;
            cerr << "Try and specify -j before html file sections" << endl;
        }
    }
}

// =================================================================================================
int Presenter::init(int argc, const char * argv[], const char* version)
{
    unsigned fileCnt = 0;
    int returnVal = -1;
    publishPtr = make_unique<PublishText>(clist, *this);    // Set default publisher

    if (argc == 1)
    {
        cerr << "\n" << argv[0] << " ("
             << version << ")\n"
             << "\nDes: Generate Java class dependence tree (" __DATE__ ")"
                "\nUse: Javatree [-+ntgxshjz] header_files...\n"
                "\nSwitches (*=default)(-=off, +=on):"
                "\n  n  ; Show alphabetic class name list"
                "\n* t  ; Show class dependency tree"
                "\n"
                "\nOutput format (single choice):"
                "\n* g  ; Use graphics for tree connections"
                "\n  x  ; Use (+|-) for tree connections"
                "\n  s  ; Use spaces for tree connections"
                "\n  h  ; Html tree connections (needs images 0.png, 1.png, 2.png, n.png)"
                "\n  j  ; Java tree connections (needs dtree www.destroydrop.com/javascript/tree/)"
                "\n  z  ; GraphViz (see https://graphviz.gitlab.io/)"
                "\n"
                "\n  With -j to customize java 1=header, 2=body begin, 3=body end"
                "\n     Specify file which contains html text to insert in output"
                "\n  -1=head.html -2=bodybegin.html -3=bodyend.html"
                "\n     Optional replacement words applied to html sections"
                "\n  -0=x1,foo1 -0=x2,foo2 -0=x3,foo3"
                "\n"
                "\nModifiers:"
                "\n  Z               ; Split GraphViz by tree, use with -O"
                "\n  N =nodesPerFile ; Split by nodes per file, use with -O"
                "\n  O =outpath      ; Save output in file"
                "\n  T =tabular      ; Tabular html "
                "\n  V =filePattern  ; Ignore files"
                "\n  A =allClasses   ; Defaults to public"
                "\n  F =full path    ; Defaults to relative"
                "\n  L =Title        ; Set optional title"
                "\n"
                "\nExamples (assumes java source code in directory src):"
                "\n  javatree -t +n  src\\*.java  ; *.java prevent recursion"
                "\n  javatree -x  src > javaTree.txt"
                "\n  javatree -h  src > javaTree.html"
                "\n  javatree -h -T src > javaTable.html"
                "\n  javatree -j  src > javaTreeWithJs.html"
                "\n  javatree -j -1=head.html -2=bodybegin.html -3=bodyend.html src > jtrees.html"
                "\n"
                "\n  -V is case sensitive "
                "\n  javatree -z -Z -O=.\\viz\\ -V=*Test* -V=*Exception* src >directgraph.dot"
                "\n  javatree -z -N=10 -O=.\\viz\\ -V=*Test* -V=*Exception* src >directgraph.dot"
                "\n";
    }
    else
    {
        string title;
        for (int argn = 1; argn < argc; argn++)
        {
            if (*argv[argn] == '-' || *argv[argn] == '+')
            {
                bool polarity = (*argv[argn] == '+');
                
                switch (argv[argn][1])
                {
                    case '0':
                        replacements.push_back(argv[argn] + 3);
                        break;
                    case '1':
                        loadfile(argv[argn] + 3, *publishPtr, &PublishHtml::SetHead);
                        break;
                    case '2':
                        loadfile(argv[argn] + 3, *publishPtr, &PublishHtml::SetBodyBegin);
                        break;
                    case '3':
                        loadfile(argv[argn] + 3, *publishPtr, &PublishHtml::SetBodyEnd);
                        break;
                        
                    case 'L':   // Set Title (label)
                        titles.push_back(argv[argn] + 3);
                        break;
                        
                        // Base options
                    case 'n': show_names= polarity;       break;
                    case 't': show_tree = polarity;       break;
                        
                        // Output format
                    case 'g': cset = Presenter::GRAPHICS_CHAR;  break;
                    case 'x': cset = Presenter::TEXT_CHAR;      break;
                    case 's': cset = Presenter::SPACE_CHAR;     break;
                    case 'h': cset = Presenter::HTML_CHAR;
                        publishPtr = make_unique<PublishHtml>(clist, *this);
                        break;
                    case 'j': cset = Presenter::JAVA_CHAR;
                         publishPtr = make_unique<PublishHtml>(clist, *this);
                        break;
                    case 'z': cset = Presenter::VIZ_CHAR;
                        publishPtr = make_unique<PublishViz>(clist, *this);
                        break;
                        
                        // Modifiers
                    case 'A': allClasses = true;        break;
                    case 'F': fullPath = true;          break;
                    case 'T': tabularList = true;
                        publishPtr = make_unique<PublishHtml>(clist, *this);
                        break;
                    case 'Z': vizSplit = true;          break;
                    case 'N':
                        nodesPerFile = (int)strtol(argv[argn] + 3, 0, 10);
                        break;
                    case 'O':   // -O=<outPath>
                        outPath = argv[argn]+3;
                        break;
                    case 'V':  // -V=<pattern>   ignore list of patterns
                        string str = argv[argn]+3;
                        replaceAll(str, "*", ".*");
                        parser.ignorePatterns.push_back(regex(str));
                        break;
                }
            }
            else
            {
                string codePath = argv[argn];
                fileCnt += parser.findClasses(codePath, clist, *this);
                if (title.empty())
                    title = codePath;
            }
        }
        
        returnVal = (int)clist.size();
        cerr << "llclasses - Files read:" << fileCnt << " classes extracted:" << clist.size() << endl;
        
        
        if (!clist.empty()) {
            publishPtr->present();
            
            clist.release();
        }
        cerr << endl;
    }
    
    return returnVal;   // Return parsed class count, else -1
}

// -------------------------------------------------------------------------------------------------
size_t Presenter::displayInterfaces(
        const RelationPtr relPtr,
        const char* label) const
{
    size_t nodeCnt = 0;
    if (relPtr != NULL)
    {
        // [color=red,penwidth=3.0]
        const RelationList& interfaces = relPtr->interfaces;
        const char* sep = label; // " Interfaces:";
        for (RelationList::const_iterator iter = interfaces.begin(); iter != interfaces.end(); iter++)
        {
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
