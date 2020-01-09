//-------------------------------------------------------------------------------------------------
//
// File: ClassRel
// Author: Dennis Lang
// Desc: Class relations (parents, children, interfaces, meta, modifiers, file, package, ...)
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

#include "ClassRel.h"
#include "utils.h"
#include "Log.h"

static const RelationPtr RELATION_NULL = NULL;

//-------------------------------------------------------------------------------------------------
void ClassRelations::addRelationTo(RelationList& list, const RelationPtr relPtr)
{
    RelationPtr oldPtr = find(list, relPtr);
    if (oldPtr == RELATION_NULL)
        list.insert(relPtr);
    else if (oldPtr->filename != relPtr->filename)
        Log::warning(LOG_DUPLCATE).out()  << "Duplicate " << relPtr->name << " files:"
            << oldPtr->filename  << " and " << relPtr->filename;
}

//-------------------------------------------------------------------------------------------------
RelationPtr ClassRelations::find(RelationList& list, const RelationPtr findPtr)
{
    return list.find(findPtr) == list.end() ? RELATION_NULL : findPtr;
    /*
    for (auto item : list)
    {
        if (item == findPtr)
            return item;
    }
    return RELATION_NULL;
    */
}

// =================================================================================================
// ClassList

// Add a class to list
RelationPtr ClassList::addClass(
     const string& type,           // class or interface
     const string& className,
     const string& classModifier,
     const string& filename,
     const string& package,
     bool definition)
{
    if (matchesRE(className, ignoreClassPatterns))
    {
        Log::warning(LOG_IGNORE).out() << "Ignoring class:" << className;
        return NULL;
    }
    if (!includeClassPatterns.empty() && !matchesRE(className, includeClassPatterns))
    {
        return NULL;
    }
    
    // multimap, get range of matches.
    pair<ClassList::iterator, ClassList::iterator> iter = equal_range(className);
    RelationPtr crel_ptr = NULL;
    bool found = false;
    
    // Find any duplicates or partially populated entry
    while (iter.first != iter.second)
    {
        // if (package.length() == 0
        //     || iter.first->second->package == package
        //    || iter.first->second->package.length() == 0)
        if (definition == false
            || iter.first->second->package == package
            || iter.first->second->definition == false)
        {
            found = true;
            crel_ptr = iter.first->second;
            break;
        }
        iter.first++;
    }
    
    if (!found)
    {
        crel_ptr = new ClassRelations(type, className, classModifier, filename, package, definition);
        insert(make_pair(className, crel_ptr));
    }
    else
    {
        if ( definition && crel_ptr->definition)
        {
            Log::warning(LOG_DUPLCATE).out()
                << "Duplicate class=" << className
                << "\n File " << filename
                << "\n  and " << crel_ptr->filename;
        }
        if (definition) {
            crel_ptr->definition = true;
            crel_ptr->type = type;
            crel_ptr->modifier = classModifier;
            crel_ptr->filename = filename;
            crel_ptr->package = package;
        }
    }
    
    return crel_ptr;
}

// ---------------------------------------------------------------------------
// Add a parent class to list
void ClassList::addParent(
        RelationPtr pChild,
        const string& parents_name,
        const string& filename,
        const string& package)
{
    static const string empty;
    RelationPtr pSuper = addClass("class", parents_name, empty, filename, package, false);
    if (pSuper != NULL)
    {
        pChild->addParent(pSuper);
        pSuper->addChild(pChild);
    }
}

// ---------------------------------------------------------------------------
// Release class list children and parents.
void ClassList::release()
{
    ClassList::const_iterator iter;
    for (iter = begin(); iter != end(); iter++)
    {
        RelationPtr crel_ptr = iter->second;
        delete crel_ptr;
    }
    clear();
}
