//-------------------------------------------------------------------------------------------------
//
//  Publish      2-Feb-2019       Dennis Lang
//
//  Publish reports of parse classes. 
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
#include "llclasses.hpp"
#include "ClassRel.hpp"

// Forward reference
class Presenter;

// =================================================================================================
// Abstract base class for Publishers
class Publish
{
public:
    virtual ~Publish() {}
    
    virtual void present() const = 0;
    virtual void displayDependencies() const = 0;
    virtual void displayNames() const = 0;

    inline static void printIndent(const Indent& indent)
    {
        for(string val : indent)
            printf("%s", val.c_str());
    }

protected:
};

// =================================================================================================
// Present in Viz node graph format
class PublishText : public Publish
{
public:
    PublishText(const ClassList& clist, Presenter& presenter) : clist(clist), presenter(presenter) {}
    void present() const override;

private:
    void displayDependencies() const override;
    void displayNames() const override;
    void displayOtherParents(const RelationPtr parentPtr, const RelationList& parentList) const;

protected:
    void displayChildren(
            Indent& indent,
            size_t fnWidth,
            size_t fnOffset,
            size_t modWidth,
            const RelationPtr parentPtr) const;
    
protected:
    const ClassList& clist;
    Presenter& presenter;
};

// =================================================================================================
// Present in Viz node graph format
class PublishViz : public PublishText
{
public:
    PublishViz(const ClassList& clist, Presenter& presenter) : PublishText(clist, presenter) {}
    
    void displayDependencies() const override;

private:
    void outHeader() const;
    void outTrailer() const;
    
    bool nextFile(ostream& out, size_t& nodeCnt, size_t nextNodeCnt) const;
    size_t displayChildren(unsigned parentNum,  size_t width,
                           const RelationPtr parentPtr, const RelationPtr pparentPtr) const;
    
    // Present stat variables
    mutable bool needHeader = true;
    mutable ofstream outStream;
    mutable string graphName;
    mutable string title;
};

// =================================================================================================
// Present in HTML tabular formats
class PublishHtml : public PublishText
{
public:
    PublishHtml(const ClassList& clist, Presenter& presenter) : PublishText(clist, presenter) {}

    void present() const override;
    void displayDependencies() const override;
    
    // Alternate html components
    // Replacement words may be applied to these section before output.
    mutable string htmlHead;
    mutable string bodyBegin;
    mutable string bodyEnd;
    
    void SetHead(const string& str) {
        htmlHead = str;
    }
    void SetBodyBegin(const string& str) {
        bodyBegin = str;
    }
    void SetBodyEnd(const string& str) {
        bodyEnd = str;
    }
    
private:
    void outputHtmlMetaHeader2(const char* auxStyle) const;
    void outputHtmlTitle3() const;
    
    size_t displayChildren(unsigned parentNum,  size_t width,
            const RelationPtr parentPtr, const string& basepath) const;
};


typedef void (PublishHtml::*SetData)(const string&);
