//-------------------------------------------------------------------------------------------------
//
//  present-text.cpp      2-Feb-2019       Dennis Lang
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

// 4291 - No matching operator delete found
#pragma warning( disable : 4291 )


#include <iostream>
#include "Presenter.hpp"
#include "SwapStream.hpp"
#include "utils.hpp"

#ifdef WIN32
static char vline[]   = "   \xb3";
static char lcorner[] = "   \xc0";
static char tsplit[]  = "   \xc3";
#else
// https://en.wikipedia.org/wiki/Box-drawing_character
static char vline[]   = "  \u2503";
static char lcorner[] = "  \u2517";
static char tsplit[]  = "  \u2523";
#endif

// <img class='img0'> <img src='n.png'>

//                               graph     text    spaces   html
static string none[]         = {"    ",    "    ", "    ", "&nbsp;&nbsp;&nbsp;&nbsp;"  /* "<img class='imgN'>" */, ""};
static string more[]         = { vline,    "   |", "    ",  vline  /* "<img class='img0'>" */, ""};
static string more_and_me[]  = {tsplit,    "   +", "    ", tsplit  /* "<img class='img2'>" */, ""};
static string just_me[]      = {lcorner,   "   -", "    ", lcorner /* "<img class='img1'>" */, ""};

static string dc("\nClass Tree\n");
static string cc(": ");
static string doc_begin[]        = {""  , ""  , ""  , "<html>", ""};
static string doc_classes[]      = {dc  , dc  , dc  , "<table>", ""};
static string doc_classesChild[]  = {cc  , cc  , cc  , "<td>", ""};
static string doc_classesBLine[] = {""  , ""  , ""  , "<tr><td>", ""};
static string doc_classesELine[] = {"\n", "\n", "\n", "</tr>\n", ""};
static string doc_end[]          = {""  , ""  , ""  , "</table></html>", ""};


// -------------------------------------------------------------------------------------------------
void PublishText::present() const {
    
    if (presenter.show_names)
        displayNames();
    if (presenter.show_tree)
        displayDependencies();
}

// -------------------------------------------------------------------------------------------------
static const char* ifEmpty(const RelationList& inStr, const char* emptyStr) {
    return inStr.empty() ? emptyStr : (*inStr.begin())->name.c_str();
}
static const char* ifEmpty(const string& inStr, const char* emptyStr) {
    return inStr.empty() ? emptyStr : inStr.c_str();
}
// -------------------------------------------------------------------------------------------------
// Output Tab separated Values  (TSV) of class list.
void PublishText::displayNames() const
{
    RelationPtr crel_ptr;
    
    cout << "Type\tClassName\tGeneric\tModifiers\tFirstParent\tFirstChild\tInterfaces\tFile\tPackage\tMeta\n";
    ClassList::const_iterator iter;
    for (iter = clist.begin(); iter != clist.end(); iter++)
    {
        crel_ptr = iter->second;
        cout << crel_ptr->type;
        cout << "\t" << crel_ptr->name;
        cout << "\t" << crel_ptr->generic;
        cout << "\t" << crel_ptr->modifier;
        cout << "\t" << ifEmpty(crel_ptr->parents, "_NoParent_");
        cout << "\t" << ifEmpty(crel_ptr->children, "_NoChildren_");
        cout << "\t";
        presenter.displayInterfaces(crel_ptr, "");
        cout << "\t" << ifEmpty(crel_ptr->filename, "_NoFilename_");
        cout << "\t" << ifEmpty(crel_ptr->package, "_NoPackage_");
        cout << "\t" << crel_ptr->meta;
        
        cout << endl;
    }
}

// -------------------------------------------------------------------------------------------------
void PublishText::displayOtherParents(const RelationPtr parentPtr, const RelationList& parentList) const
{
    for (RelationList::const_iterator iter = parentList.begin(); iter != parentList.end(); iter++)
    {
        if (*iter != parentPtr)
        {
            const string& name = (*iter)->name;
            printf("  (%s)", name.c_str());
        }
    }
}

// -------------------------------------------------------------------------------------------------
void PublishText::displayChildren(
        Indent& indent,
        size_t fnWidth,     // Filename presentation width
        size_t fnOffset,
        size_t modWidth,
        const RelationPtr parentPtr) const
{
    const RelationList& children = parentPtr->children;
    RelationPtr crel_ptr;
    int cset = presenter.cset;
    
    for (RelationList::const_iterator iter = children.begin(); iter != children.end(); iter++)
    {
        crel_ptr = *iter;
        if (presenter.canShowChildren(crel_ptr))
        {
            presenter.hasShown(crel_ptr);   // Prevent circular display
            
            string name =crel_ptr->name;
            bool last = crel_ptr == *children.rbegin();
            indent.push_back(!last ? more_and_me[cset] : just_me[cset]);
            
            fputs(doc_classesBLine[cset].c_str(), stdout);
            const char* fname = crel_ptr->filename.c_str() + min(crel_ptr->filename.length(), fnOffset);
            printf("%*.*s%s",
                   (unsigned)fnWidth, (unsigned)fnWidth, fname,
                   doc_classesChild[cset].c_str()
                   );
            printf(" %*.*s%s", (unsigned)modWidth, (unsigned)modWidth, crel_ptr->modifier.c_str(),
                   doc_classesChild[cset].c_str());
            printIndent(indent);
            printf(" %s", name.c_str());
            
            displayOtherParents(parentPtr, crel_ptr->parents);
            presenter.displayInterfaces(crel_ptr);
            fputs(doc_classesELine[cset].c_str(), stdout);
            
            indent.pop_back();
            indent.push_back(!last ? more[cset] : none[cset]);
            
            // TODO - avoid circular children
            displayChildren(indent, fnWidth, fnOffset, modWidth, crel_ptr);
            indent.pop_back();
        }
    }
}


// -------------------------------------------------------------------------------------------------
void PublishText::displayDependencies() const
{
    RelationPtr crel_ptr;
    ClassList::const_iterator iter;
    
    size_t fileWidth = 14;
    size_t nameWidth = 14;
    size_t modWidth = 6;       // public
    string basepath;
    
    int cset = presenter.cset;
    
    // Compute maximum field sizes
    for (iter = clist.begin(); iter != clist.end(); iter++)
    {
        crel_ptr = iter->second;
        crel_ptr->flags = 0;
        
        fileWidth = max(fileWidth, crel_ptr->filename.length());
        nameWidth = max(nameWidth, crel_ptr->name.length());
        modWidth = max(modWidth, crel_ptr->modifier.length());
        basepath = equalSubStr(basepath, crel_ptr->filename);
    }
    size_t baseLen = basepath.rfind(DIR_SLASH_STR) + 1;
    fileWidth -= baseLen;
    
        fprintf(stdout, "Base path=%s\n", basepath.c_str());
        fputs(doc_classes[cset].c_str(), stdout);
  
    SwapStream swapStream(cout);
    
    for (iter = clist.begin(); iter != clist.end(); iter++)
    {
        crel_ptr = iter->second;
        if ((crel_ptr->isSuper() || crel_ptr->isCircular())
            && presenter.canShowChildren(crel_ptr))  // Find Super class (no parent)
        {
            presenter.hasShown(crel_ptr);
            size_t fnOffset = min(crel_ptr->filename.length(), baseLen);
            const char* fname = crel_ptr->filename.c_str() + fnOffset;
            fputs(doc_classesBLine[cset].c_str(), stdout);
            printf("%*.*s%s %*.*s %s %s %s",
                   (unsigned)fileWidth, (unsigned)fileWidth,
                   fname,
                   doc_classesChild[cset].c_str(),
                   -(int)modWidth, (unsigned)modWidth, crel_ptr->modifier.c_str(),
                   crel_ptr->name.c_str(),
                   crel_ptr->generic.c_str(),
                   crel_ptr->meta.c_str()
                   );
            presenter.displayInterfaces(crel_ptr);
            fputs(doc_classesELine[cset].c_str(), stdout);
            Indent indent;
            displayChildren(indent, fileWidth, baseLen, modWidth, crel_ptr);
        }
    }
}
