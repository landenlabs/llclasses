//-------------------------------------------------------------------------------------------------
//
// File: ParseCpp
// Author: Dennis Lang
// Desc: Parse C++ code
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


#include "ParseCpp.hpp"

#include <assert.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <ctype.h>
#include <string>
#include <iosfwd>

#include <vector>
#include <map>
#include <algorithm>
#include <regex>

#include "ParseJava.hpp"
#include "Presenter.hpp"
#include "utils.hpp"

static string emptyStr;
// eoc=end-of-code semicolon or { or }
enum State { none, quote1, quote2, comment, eoc };

//-------------------------------------------------------------------------------------------------
static unsigned findEnd(string &line, string& part, unsigned idx, State& state)
{
    const char Q1 = '\'';
    const char Q2 = '"';
    const char C1 = '/';
    const char C2B = '*';       // comment block
    const char C2L = '/';       // comment line
    const char ESC = '\\';
    const char PP = '#';        // Preprocessor directive
    
    unsigned eol = (unsigned)part.length();
    State prevState = none;
    
    switch (state) {
        default:
        case eoc:
        case none:
            if (part.length() > 0 && part[0] == PP)
                idx = eol;
            
            for (; idx < part.length()-1; idx++) {
                char chr1 = part[idx];
                if (chr1 == Q1) {
                    state = quote1;
                    idx = findEnd(line, part, idx+1, state);
                } else if (chr1 == Q2) {
                    state = quote2;
                    idx = findEnd(line, part, idx+1, state);
                } else if (chr1 == C1) {
                    char chr2 = part[idx+1];
                    if (chr2 == C2B) {
                        state = comment;
                        idx = findEnd(line, part, idx+2, state);
                    } else if (chr2 == C2L) {
                        // if (state == comment || state == quote1 || state == quote2) {
                        state = prevState;
                        // }
                        idx = eol;
                        break;
                    } else {
                        line += (chr1);
                        line += (chr2);
                        idx++;
                    }
                } else if (chr1 == ';' || chr1 == '{' || chr1 == '}' ) {
                    line += (chr1);
                    prevState = state = eoc;
                    return idx;
                } else {
                    line += (chr1);
                }
            }
            break;
        case quote1:
            for (; idx < part.length()-1; idx++) {
                char chr1 = part[idx];
                if (chr1 == Q1) {
                    state = none;
                    return idx;
                } else if (chr1 == ESC && part[idx+1] == Q1) {
                    idx++;
                }
            }
            if (part[idx] == Q1)
                state = none;
            break;
            
        case quote2:
            for (; idx < part.length()-1; idx++) {
                char chr1 = part[idx];
                if (chr1 == Q2) {
                    state = none;
                    return idx;
                } else if (chr1 == ESC && part[idx+1] == Q2) {
                    idx++;
                }
            }
            if (part[idx] == Q2)
                state = none;
            break;
            
        case comment:
            for (; idx < part.length()-1; idx++) {
                char chr1 = part[idx];
                if (chr1 == '*' && part[idx+1] == '/') {
                    state = prevState;
                    return idx+1;
                }
            }
            break;
    }
    
    if (idx < eol) {
        char chr1 = part[idx];
        if (state == none || state == eoc) {
            line += chr1;
            if (chr1 == ';' || chr1 == '{' || chr1 == '}' ) {
                state = eoc;
            }
        }
    }
    return eol;
}

//-------------------------------------------------------------------------------------------------
static istream& getCppLine(istream& in, string& line, string& buffer)
{
    State state = none;
    
    line.clear();
    while (buffer.length()>0 || getline(in, buffer).good())
    {
        unsigned idx;
        for (idx = 0; idx < buffer.length() && state != eoc; idx++) {
            idx = findEnd(line, buffer, idx, state);
        }
        buffer.erase(0, idx);
        if (!line.empty() && (state == eoc) ) {
            trim(line);
            return in;
        }
        line += " ";    // EOL
    }
    
    return in;
}

// ===============================================================================================
// Function used with Split object to perform custom split to handle nested
// Java Generic (template) typing.   ex:  foo<bar<car>>>
static size_t FindSplit(const string& str, const char* delimList, size_t begIdx)
{
    int depth = 0;
    for (size_t idx = begIdx; idx < str.length(); idx++)
    {
        const char c = str[idx];
        if (c == '<')
            depth++;
        if (c == '>')
            depth--;
        if (depth == 0)
        {
            for (const char* cptr = delimList; *cptr != '\0'; cptr++)
            {
                if (c == *cptr)
                    return idx;
            }
        }
    }
    
    return string::npos;
}

//-------------------------------------------------------------------------------------------------
static string& removeTemplate(string& inOut)
{
    return replaceRE(inOut, "<.*>", "");
}

//-------------------------------------------------------------------------------------------------
// Parse java source code and extract class or import relationships
int ParseCpp::parseCode(const string& filename, ClassList& clist, const Presenter& presenter) {
    
    if (presenter.parseImports != 0)
    {
        return parseCppIncludes(filename, clist, presenter);
    }
    else
    {
        return parseCppClasses(filename, clist, presenter);
    }
}

//-------------------------------------------------------------------------------------------------
// Parse java source code and extract class definitions
int ParseCpp::parseCppClasses(const string& filename, ClassList& clist, const Presenter& presenter)
{
    // Pattern     class_p(" class ([~/ ]* |) *`[A-Z][A-Za-z0-9_:]*` *( :|@{|;|)");
    // Pattern     parents_p("[ :,]*(public|protected|private)  *`[A-Z][A-Za-z0-9_:]*`");
    /*
     https://www-01.ibm.com/support/docview.wss?uid=swg27002103&aid=1
     
     https://docs.microsoft.com/en-us/cpp/cpp/class-cpp?view=vs-2017
     
     [template-spec]
     class [ms-decl-spec] [tag [: base-list ]]
     {
     member-list
     } [declarators];
     [ class ] tag declarators;
     
     
     class foo {
     };
     
     struct foo {
     };
     
     <template typename TT>
     virtual public class foo : public Base1, protected Base2, private Base3 {
     };
    */
    
    string line;
    string buffer;
    static regex cppClassRE("(.+ |)class ([A-Za-z0-9_]+)( *: *[A-Za-z0-9_.<>, ]+|) *[{]");
    smatch match;
    ifstream in(filename);
    string package;
    string outer;
    string classOrInterface = "class";   // "class" or "struct"
    string meta;
    string modifiers;           // "public" ... "abstract", etc
    string implements;          // implementa iname1, iname2
    
    // classNames.clear();
    
    while (getCppLine(in, line, buffer)) {
        replaceAll(line, ", ", ",");
        if (regex_match(line, match, cppClassRE, regex_constants::match_default)) {
            
            // string outer = join(classNames, ".", ".");
            string name = outer + string(match[2]);
            string templateStr = match[1];
            string parent = match[3];
            modifiers = "public";       // TODO - handle modifiers correctly, find ctor and determine if its public
            
            // Add class definition
            RelationPtr crel_ptr =
            clist.addClass(classOrInterface, name, trim(modifiers), filename, package, true);
            if (crel_ptr != NULL) {
                crel_ptr->meta = meta;
                crel_ptr->generic = templateStr;
                
                // Add all parent (extend) classes
                if (parent.length() > 0) {
                    parent = trim(replaceRE(parent, " *: *(virtual |)(public|protected|private|)", ""));
                    Split multipleParents(parent, ",", FindSplit);
                    for (string parentItem : multipleParents) {
                        parentItem = trim(removeTemplate(parentItem));
                        clist.addParent(crel_ptr, parentItem, filename, package);
                    }
                }
                
                // classNames.push_back(name); // Used for nested (inner classes)
                
                if (presenter.logLevel >= LOG_ADDING_CLASSES) {
                    cout << line << endl;
                    cout << "  Modifiers       =" << modifiers << endl;
                    cout << "  classOrInterface=" << classOrInterface << endl;
                    cout << "  name            =" << name << endl;
                    cout << "  template        =" << templateStr << endl;
                    cout << "  parent          =" << parent << endl;
                    cout << "  implements      =" << implements << endl;
                }
            }
        } else {
            
            /*
            // Count nesting of braces, used for nested inner classes
            regex bracesRE ("[{}]");
            regex_iterator<string::iterator> rit ( line.begin(), line.end(), bracesRE );
            regex_iterator<string::iterator> rend;
            
            while (rit != rend) {
                if ( rit->str() == "{") {
                    classNames.push_back("");
                }
                else if (classNames.size() > 0)
                {
                    classNames.pop_back();
                }
                rit++;
            }
             */
        }
    }
    
    //classNames.clear(); // Should be empty already.
    
    return 0;
}

//--------------------------------------------------------------------------------------------------
// Remove trailing parts, return true if empty.
static bool removeParts(string& inOutStr, size_t parts, const char* find=".")
{
    if (parts > 0) {
        size_t dotPos = inOutStr.length();
        while (parts-- > 0 && (dotPos = inOutStr.rfind(find, dotPos-1)) > 0);
        if (dotPos > 0)
            inOutStr.erase(dotPos);
        else if (dotPos == 0)
            return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
// Parse java source code and extract imported packages
int ParseCpp::parseCppIncludes(const string& filename, ClassList& clist, const Presenter& presenter)
{
    
    string line;
    string buffer;
    static regex javePackageRE("package ([A-Za-z0-9_.]+);");
    static regex javeImportRE("include [<\"]([A-Za-z][A-Za-z0-9_.*]*)[>\"]");
    smatch match;
    string package = "none";
    ifstream in(filename);
    RelationPtr crel_ptr = NULL;
    
    while (getCppLine(in, line, buffer)) {
        replaceAll(line, ", ", ",");
        if (regex_match(line, match, javePackageRE, regex_constants::match_default))
        {
            assert(crel_ptr == NULL);
            package = match[1];
            if (removeParts(package, presenter.parseImports)) {
                continue;
            }
            crel_ptr =
            clist.addClass("class", package, "", filename, package, true);
        }
        else if (regex_match(line, match, javeImportRE, regex_constants::match_default))
        {
            assert(crel_ptr != NULL);
            string modifiers;
            string name = match[1];
            
            if (removeParts(name, presenter.parseImports)) {
                continue;
            }
            
            replaceAll(name, ".*", "");
            if (false) {
                clist.addParent(crel_ptr, name, filename, package);
            } else {
                RelationPtr child_ptr =
                clist.addClass("class", name, modifiers, filename, name, false);
                // child_ptr->addParent(crel_ptr);
                // crel_ptr->addChild(child_ptr);
                if (child_ptr != NULL) {
                    clist.addParent(child_ptr, package, filename, package);
                }
            }
            
            if (presenter.logLevel >= LOG_ADDING_CLASSES) {
                cout << line;
                cout << "\n  Modifiers =" << modifiers;
                cout << "\n  name      =" << name;
                cout << "\n  #children =" << crel_ptr->children.size();
                cout << "\n  #parents  =" << crel_ptr->parents.size();
                cout << "\n";
            }
        }
    }
    
    if (presenter.logLevel >= LOG_CLASS_LIST) {
        ClassList::const_iterator iter;
        RelationPtr crel_ptr;
        
        cout << "\n ---- Class List ---- \n";
        for (iter = clist.begin(); iter != clist.end(); iter++)
        {
            crel_ptr = iter->second;
            cout << "\nname    =" << crel_ptr->name;
            cout << "\nparents =" << crel_ptr->parents.size() << " " << crel_ptr->parents;
            cout << "\nchildren=" << crel_ptr->children.size() << " " << crel_ptr->children;
            cout << "\nfile    =" << crel_ptr->filename;
            cout << "\ndefined =" << crel_ptr->definition;
            cout << "\n";
        }
    }
    
    return 0;
}


