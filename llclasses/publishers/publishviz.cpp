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


#include "Publish.hpp"
#include "Presenter.hpp"
#include "SwapStream.hpp"
#include "utils.hpp"

static const char sDot[] = ".";
static const char sNL[] = "\\n";

// -------------------------------------------------------------------------------------------------
// Output GraphViz header.
void PublishViz::outHeader() const
{
    needHeader = false;
    cout << "digraph " << replaceAll(graphName, " ", "_") << " {\n"
    "bgcolor=transparent\n"
    "overlap=false;\n"
    "label=\"" << title << " Class Hierarchy - Dennis Lang & GraphViz\";\n"
    "fontsize=12;\n"
    "node [shape=box,style=filled,fillcolor=white];\n";
}

// -------------------------------------------------------------------------------------------------
// Complete GraphViz output file.
void PublishViz::outTrailer() const
{
    cout << "}\n";
    needHeader = true;
}


// -------------------------------------------------------------------------------------------------
// Make title from code path, converting special characters to '_'
static string getTitle(const string& codePath)
{
    //   d:\dir1\dir2\file.java  =>  _dir1_dir2_file
    
    size_t extn = codePath.find_last_of('.');
    string title = codePath;
    if (extn != string::npos)
        title.resize(extn);
    size_t dirPos = title.find_last_of(DIR_SLASH_CHR);
    if (dirPos != string::npos)
        title.resize(dirPos);
    size_t colonPos = title.find_first_of(':');
    if (colonPos != string::npos)
        title.erase(0, colonPos+1);
    
    regex specialCharRe("[*?-]+");
    regex_constants::match_flag_type flags = regex_constants::match_default;
    title = regex_replace(title, specialCharRe, "_", flags);
    replace(title.begin(), title.end(), DIR_SLASH_CHR, '_');
    
    // graphName = title;
    return title;
}

// -------------------------------------------------------------------------------------------------
static size_t countInterfaces(const RelationPtr parentPtr)
{
    return (parentPtr != NULL) ? parentPtr->interfaces.size() : 0;
}

// -------------------------------------------------------------------------------------------------
static size_t countChildren(const RelationPtr parentPtr, const RelationPtr pparent_ptr)
{
    size_t nodeCnt = 0;
    const RelationList& children = parentPtr->children;
    

    nodeCnt += countInterfaces(parentPtr);
    
    for (RelationList::const_iterator iter = children.begin(); iter != children.end(); iter++)
    {
        nodeCnt += 1 + countChildren(*iter, parentPtr);
    }
    
    return nodeCnt;
}

// -------------------------------------------------------------------------------------------------
size_t displayInterfaces(unsigned parentNum, size_t width, const RelationPtr relPtr)
{
    size_t nodeCnt = 0;
    if (relPtr != NULL)
    {
        // [color=red,penwidth=3.0]
        const RelationList& interfaces = relPtr->interfaces;
        for (RelationList::const_iterator iter = interfaces.begin(); iter != interfaces.end(); iter++)
        {
            string name = (*iter)->name;
            cout << "\"" << name << "\"  [style=filled, fillcolor=yellow] \n";
            cout << "\"" << name << "\" -> \"" << relPtr->name << "\" [color=red,penwidth=3.0] \n";
            nodeCnt++;
        }
    }
    
    return nodeCnt;
}

// -------------------------------------------------------------------------------------------------
// Return true when time to make a new output file.
//   * no output file
//   * -Z split graphviz by subtree
//   * -N split by nodes per file.
bool PublishViz::nextFile(ostream& out, size_t& nodeCnt, size_t nextNodeCnt) const
{
    bool next = false;
    if (presenter.outPath.empty())
        return false;
    if (!out.good() || nodeCnt == -1)
        next = true;
    if (presenter.vizSplit)
        next = true;
    if (presenter.nodesPerFile != 0
        && nodeCnt * 2 > presenter.nodesPerFile
        && nodeCnt + nextNodeCnt/2 > presenter.nodesPerFile)
        next = true;
    
    if (next)
        nodeCnt = 0;
    
    return next;
}

// -------------------------------------------------------------------------------------------------
size_t PublishViz::displayChildren(
      unsigned parentNum,
      size_t width,
      const RelationPtr parentPtr,
      const RelationPtr pparentPtr) const
{
    size_t nodeCnt = 0;
    RelationPtr child_ptr;
    const char* parendModStr = "";
    const char* childModStr = "";
    
    //  if (cset == VIZ_CHAR)
    {
        nodeCnt += displayInterfaces(parentNum, width, parentPtr);
        
        if (parentPtr == NULL)
            parendModStr = " [fillcolor=cyan1]";
        else
            parendModStr = " [fillcolor=cyan4]";
    }
    
    const RelationList& children =  parentPtr->children;
    if (!children.empty()) {
        for (RelationList::const_iterator iter = children.begin(); iter != children.end(); iter++)
        {
            child_ptr = *iter;
            
            if (child_ptr != NULL)
            {
                string chilNname = child_ptr->name;
                // if (cset == VIZ_CHAR)
                {
                    replaceAll(chilNname, sDot, sNL);
                    string parentName = parentPtr->name;
                    replaceAll(parentName, sDot, sNL);
                    
                    if (parentPtr->modifier.find("abstract") != -1)
                        parendModStr = (parentPtr == NULL) ? " [color=green] " : " [fillcolor=chartreuse] ";
                    if (child_ptr->modifier.find("abstract") != -1)
                        childModStr = (parentPtr == NULL) ? " [color=green] " : " [fillcolor=chartreuse] ";
                    if (child_ptr->modifier.find("public") == -1)
                        childModStr = " [color=red] ";
                    
                    if (*parendModStr != 0)
                        cout << "\"" << parentName << "\" " << parendModStr << endl;
                    if (*childModStr != 0)
                        cout << "\"" << chilNname << "\" " << childModStr << endl;
                    cout << "\"" << parentName << "\" -> \"" << chilNname << "\"\n";
                    parendModStr = "";
                }
                nodeCnt++;
                nodeCnt += displayChildren(presenter.sNodeNum++, width, child_ptr, parentPtr);
            }
        }
    }
    else
    {
        // Single node - no children
        string name = parentPtr->name;
        // if (cset == VIZ_CHAR)
        {
            if (presenter.importPackage)
            {
                if (parentPtr->parents.empty())
                    cerr << "Isolated package " << name << endl;
            }
            else
            {
                replaceAll(name, sDot, sNL);
                if (parentPtr->modifier.find("abstract") != -1)
                    childModStr = (parentPtr == NULL) ? " [color=green] " : " [fillcolor=chartreuse] ";
                if (parentPtr->modifier.find("public") == -1)
                    childModStr = " [color=red] ";
                
                if (*childModStr != 0)
                    cout << "\"" << name << "\" " << childModStr << endl;
                cout << "\"" << name << "\"\n";
                
                nodeCnt++;
            }
        }
    }
    return nodeCnt;
}


// -------------------------------------------------------------------------------------------------
void PublishViz::displayDependencies() const
{
    RelationPtr crel_ptr;
    ClassList::const_iterator iter;
    
    size_t fileWidth = 14;
    size_t nameWidth = 14;
    size_t modWidth = 6;       // public
    string basepath;
    
    // Compute maximum field sizes
    for (iter = clist.begin(); iter != clist.end(); iter++)
    {
        crel_ptr = iter->second;
        fileWidth = max(fileWidth, crel_ptr->filename.length());
        nameWidth = max(nameWidth, crel_ptr->name.length());
        modWidth = max(modWidth, crel_ptr->modifier.length());
        basepath = equalSubStr(basepath, crel_ptr->filename);
    }
    size_t baseLen = basepath.rfind(DIR_SLASH_STR) + 1;
    fileWidth -= baseLen;
    
    SwapStream swapStream(cout);
    title = graphName = getTitle(getIt(presenter.titles, 0, "Class Diagram"));
    
    size_t nodeCnt = -1;
    for (iter = clist.begin(); iter != clist.end(); iter++)
    {
        crel_ptr = iter->second;
        if (crel_ptr->parents.empty())  // Find Super class (no parent)
        {
            // Have super class - now display subclasses.
            
            // if (cset == VIZ_CHAR)
            {
                if (nextFile(outStream, nodeCnt, countChildren(crel_ptr, NULL)))
                {
                    if (outStream.good())
                    {
                        outTrailer();
                        swapStream.restore();
                        outStream.close();
                    }
                    needHeader = true;
                    string outFile = presenter.outPath + crel_ptr->name + ".gv";
                    regex dosSpecial("[<,>?]");
                    outFile = regex_replace(outFile, dosSpecial, "_");
                    outStream.open(outFile);
                    if (outStream.good())
                        swapStream.swap(outStream);
                    else
                        cerr << "Failed to open " << outFile << endl;
                }
                
                if (needHeader)
                    outHeader(); // outVizHeader();
                
                // cout << crel_ptr->name() << " -> " <<   crel_ptr->file() << endl;
                nodeCnt += displayChildren(presenter.sNodeNum , fileWidth, crel_ptr, NULL);
                // display_interfaces(sNodeNum , fileWidth, crel_ptr);
                presenter.sNodeNum++;
            }
        }
    }
    
    // if (cset == VIZ_CHAR)
    {
        if (outStream.good())
        {
            outTrailer();
            swapStream.restore();
            outStream.close();
        }
    }
}
