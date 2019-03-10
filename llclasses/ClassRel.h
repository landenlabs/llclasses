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

#pragma once
#include "llclasses.h"

#include <set>
#include <map>

// Debug
#include <iostream>

// -------------------------------------------------------------------------------------------------
class ClassRelations;
class ClassLinkage;

typedef ClassRelations*  RelationPtr;
typedef set<RelationPtr> RelationList;

// -------------------------------------------------------------------------------------------------
// Manage class relations - parents, children, interfaces, etc.
// @see ClassList
class ClassRelations
{
  public:
    string          type;           // class or interface
    string          name;           // Class name  outer.inner[.inner]
    string          modifier;       // Modifiers public, static, abstract
    string          filename;       // File declared in.
    string          package;        // Java package declared or referenced in.
    string          generic;        // Java optional generic modifier
    string          meta;           // Java meta tags, @api, @deprecated, etc.
    bool            definition;     // false if declaration (use but not defined)
    
    mutable unsigned flags = 0;      // Used to prevent circular presentation.
    
    RelationList    interfaces;     // List of interfaces
    RelationList    parents;        // List of super classes (base derived from)
    RelationList    children;       // List of children (derived from us)
    

    ClassRelations(
           const string& type,
           const string& className,
           const string& modifier,  // public private static final abstract
           const string& filename,
           const string& package,
           bool definition) :
      type(type), name(className), modifier(modifier),
      filename(filename), package(package),
      definition(definition)
        { }
    
    
    void  addInterface(const RelationPtr relPtr)
        { addRelationTo(interfaces, relPtr); }
    void  addParent(const RelationPtr relPtr)
        { addRelationTo(parents, relPtr); }
    void  addChild(const RelationPtr relPtr)
        { addRelationTo(children, relPtr); }
        
    RelationPtr findInterface(const RelationPtr relPtr)
        { return find(interfaces, relPtr); }
    RelationPtr findParent(const RelationPtr relPtr)
        { return find(parents, relPtr); }
    RelationPtr findChild(const RelationPtr relPtr)
        { return find(children, relPtr); }
    
    bool isCircular() {
        return find(parents, this) != NULL;
    }
    bool isSuper() {
        return parents.empty();
    }
    
  protected:    
    void  addRelationTo(RelationList&, const RelationPtr);

    RelationPtr find(RelationList&, const RelationPtr);
};

// -------------------------------------------------------------------------------------------------
// Manager collection of Class Relation objects
class ClassList : public multimap<string, RelationPtr>
{
private:
    using multimap::find;
    
public:
    // Create and add ClassRelations to list (if not duplicate), key is className
    RelationPtr addClass(
         const string& type,            // class or interface
         const string& className,
         const string& classModifier,   // public, abstract, static, ...
         const string& filename,
         const string& package,
         bool definition);              // false if declaration (use but not definition)
    
    void addParent(
        RelationPtr pChild,
        const string& parentsName,
        const string& filename,
        const string& package);
    
    void release();
    
    PatternList ignoreClassPatterns;        // -v <class_regex_pattern> [-v <>]...
    PatternList includeClassPatterns;       // -i <class_regex_pattern> [-i <>]...
};

