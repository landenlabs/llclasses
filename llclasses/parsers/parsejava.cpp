//-------------------------------------------------------------------------------------------------
//
//  parse-java      3-Feb-2019        Dennis Lang
//
//  Parse Java files and generate class names and class dependence tree.
//
//  Created by Dennis Lang on 3-Feb-2019
//  Copyright © 2019 Dennis Lang. All rights reserved.
//-------------------------------------------------------------------------------------------------
//
// Author: Dennis Lang - 2019
// http://landenlabs.com/
//
// This file is part of llclass project.
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
#include "Log.hpp"

static string emptyStr;
// eoc=end-of-code semicolon or { or }
enum State { none, quote1, quote2, paren1, paren2, comment, eoc };

//-------------------------------------------------------------------------------------------------
static unsigned findEnd(string &line, string& part, unsigned idx, State& state)
{
    const char Q1 = '\'';
    const char Q2 = '"';
    const char AT = '@';
    const char P1 = '(';
    const char P2 = ')';
    const char C1 = '/';
    const char C2B = '*';    // comment block
    const char C2L = '/';    // comment line
    const char ESC = '\\';

    unsigned eol = (unsigned)part.length();
    State prevState = none;
    
    switch (state) {
        default:
        case eoc:
        case none:
            for (; idx < part.length()-1; idx++) {
                char chr1 = part[idx];
                if (chr1 == Q1) {
                    state = quote1;
                    idx = findEnd(line, part, idx+1, state);
                } else if (chr1 == Q2) {
                    state = quote2;
                    idx = findEnd(line, part, idx+1, state);
                } else if (chr1 == AT && part.find(P1, idx+1) != string::npos) {
                    // @meta({foo,bar})
                    state = paren1;
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
            
        case paren1: // Nested Paren ?
            for (; idx < part.length()-1; idx++) {
                char chr1 = part[idx];
                if (chr1 == P1) {
                    state = paren2;
                    return findEnd(line, part, idx+1, state);
                } else if (chr1 == ESC && part[idx+1] == P1) {
                    idx++;
                }
            }
            if (part[idx] == P1)
                state = paren2;
            break;
            
        case paren2: // Nested Paren ?
            for (; idx < part.length(); idx++) {
                char chr1 = part[idx];
                if (chr1 == P2) {
                    state = none;
                    return idx;
                } else if (chr1 == ESC && part[idx+1] == P2) {
                    idx++;
                }
            }
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
static istream& getJavaLine(istream& in, string& line, string& buffer)
{
    State state = none;

    line.clear();
    while (buffer.length()>0 || getline(in, buffer).good())
    {
        unsigned idx;
        for (idx = 0; idx < buffer.length() && state != eoc; idx++) {
            idx = findEnd(line, buffer, idx, state);
        }
        
#if 1
        if (buffer.size() > 1 && buffer[0] == '@')
            line.append(buffer);    // Keep Meta strings @foo({blah, blah})
#endif
        
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
int ParseJava::parseCode(const string& filename, ClassList& clist, const Presenter& presenter) {
    
    if (presenter.parseImports != 0)
    {
        return parseJavaImports(filename, clist, presenter);
    }
    else
    {
        return parseJavaClasses(filename, clist, presenter);
    }
}

//-------------------------------------------------------------------------------------------------
// Parse java source code and extract class definitions
int ParseJava::parseJavaClasses(const string& filename, ClassList& clist, const Presenter& presenter)
{
    // Java class definition syntax.
    //
    // [@MetaDescriptions]
    // [modifiers] class ClassName
    //             [<types [,types]...>]
    //             [extends classDeclaraion]
    //             [implements interface [,interface]]
    //             {classBody};
    // where modifiers are:
    //   Annotation
    //   public
    //   protected
    //   private
    //   static
    //   abstract
    //   final
    //   native
    //   synchronized
    //   transient
    //   volatile
    //   strictfp
    
    // Examples:
    //
    // private static final class CacheKey extends Master.Inner1 implements Cloneable.Inner2 {
    // private static final class LoaderReference extends WeakReference<ClassLoader>
    // private static final class CacheKey implements Cloneable {
    // final class EntryIterator extends PrivateEntryIterator<Map.Entry<K,V>> {
    // public final class Formatter implements Closeable, Flushable {
    // static final class Entry<K,V> implements Map.Entry<K,V>
    // public abstract class Calendar implements Serializable, Cloneable, Comparable<Calendar> {
    // public abstract class AbstractSet<E> extends AbstractCollection<E> implements Set<E> {
    // static class MutablePair<F, S> {
    // public abstract class UnselectableArrayAdapter<E> extends ArrayAdapter<E> {
    // public class Garage<X extends Vehicle> {
    // class BST<X extends Comparable<X>> {
    // int totalFuel(List<? extends Vehicle> list) {
    // private static abstract class EmptySpliterator<T, S extends Spliterator<T>, C> {
    //
    // @SuppressWarnings({"unused", "WeakerAccess", "FieldCanBeLocal"})
    // public abstract class WxNotify {
    //
    
    string line;
    string buffer;
    static regex javePackageRE("package ([A-Za-z0-9_.]+);");
    static regex javeClassRE("([^ ]*? |)([Aa-z][Aa-z ]*|)(class|interface) ([A-Za-z0-9_]+)( *<.*?>+|)( extends [A-Za-z0-9_.<>, ]+?|)( implements [A-Za-z0-9_.<>, ]+?|) *[{]");
    smatch match;
    string package = "none";
    ifstream in(filename);

    classNames.clear();
    
    while (getJavaLine(in, line, buffer)) {
        replaceAll(line, ", ", ",");
        if (regex_match(line, match, javePackageRE, regex_constants::match_default)) {
            package = match[1];
        } else  if (regex_match(line, match, javeClassRE, regex_constants::match_default)) {
            
            string outer = join(classNames, ".", ".");
            string meta = match[1];
            string modifiers =  match[2];           // "public" ... "abstract", etc
            string classOrInterface = match[3];     // "class" or "interface"
            string name = outer + string(match[4]);
            string generic = match[5];              //
            string extends = match[6];              // extends alpha
            string implements = match[7];           // implementa iname1, iname2
   
            if (meta.find("@") != 0)
            {
                modifiers = meta + " " + modifiers;
                meta.clear();
            }

            // Add class definition
            RelationPtr crel_ptr =
                clist.addClass(classOrInterface, name, trim(modifiers), filename, package, true);
            if (crel_ptr != NULL) {
                crel_ptr->meta = meta;
                crel_ptr->generic = generic;
                
                // Add all parent (extend) classes
                if (extends.length() > 0) {
                    extends = trim(replaceAll(extends, " extends ", emptyStr));
                    Split extendList(extends, ",", FindSplit);
                    for (string extendItem : extendList) {
                        extendItem = trim(removeTemplate(extendItem));
                        clist.addParent(crel_ptr, extendItem, filename, package);
                    }
                }
                
                // Add all interfaces
                if (implements.length() > 0) {
                    implements = trim(replaceAll(implements, " implements ", emptyStr));
                    Split implementsList(implements, ",", FindSplit);
                    for (string implementsItem : implementsList) {
                        implementsItem = trim(removeTemplate(implementsItem));
                        RelationPtr cImpl =
                            clist.addClass("interface", implementsItem, "", filename, package, false);
                        crel_ptr->addInterface(cImpl);
                    }
                }

                classNames.push_back(name); // Used for nested (inner classes)
                
                if (presenter.logLevel >= LOG_ADDING_CLASSES) {
                    cout << line << endl;
                    cout << "  Modifiers       =" << modifiers << endl;
                    cout << "  classOrInterface=" << classOrInterface << endl;
                    cout << "  name            =" << name << endl;
                    cout << "  generic         =" << generic << endl;
                    cout << "  extends         =" << extends << endl;
                    cout << "  implements      =" << implements << endl;
                }
            }
        } else {
            
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
        }
    }

    classNames.clear(); // Should be empty already.
    
    return 0;
}

//--------------------------------------------------------------------------------------------------
// Remove trailing parts, return true if empty.
static bool removeTailParts(string& inOutStr, int parts, const char* find=".")
{
    if (parts > 0) {
        size_t dotPos = inOutStr.length();
        while (parts-- > 0 && (dotPos = inOutStr.rfind(find, dotPos-1)) != string::npos);
        if (dotPos < inOutStr.length())
            inOutStr.erase(dotPos);
        return (dotPos == string::npos) || inOutStr.empty();
    }
    return false;
}
//--------------------------------------------------------------------------------------------------
// Keep head parts, always return false
static bool keepHeadarts(string& inOutStr, int parts, const char* find=".")
{
    if (parts > 0) {
        size_t dotPos = -1;
        while (parts-- > 0 && (dotPos = inOutStr.find(find, dotPos+1)) != string::npos);
        if (dotPos < inOutStr.length())
            inOutStr.erase(dotPos);
    }
    return false;
}
//--------------------------------------------------------------------------------------------------
// Keep head parts, return true if empty.
static bool removeParts(string& inOutStr, int parts, const char* find=".")
{
    if (parts < 0) {
        return removeTailParts(inOutStr, -parts, find);
    }
    return keepHeadarts(inOutStr, parts, find);
}

//--------------------------------------------------------------------------------------------------
// Parse java source code and extract imported packages
int ParseJava::parseJavaImports(const string& filename, ClassList& clist, const Presenter& presenter)
{
    
    // import static com.google.common.base.Preconditions.checkNotNull;
    // import static com.google.common.collect.ImmutableList.toImmutableList;
    // import static com.google.common.collect.ImmutableSet.toImmutableSet;
    //
    // import com.google.auto.common.MoreTypes;
    // import com.google.auto.value.AutoValue;
    // import com.google.common.base.Equivalence;
    // import com.google.common.base.Joiner;
    
    string line;
    string buffer;
    static regex javePackageRE("package ([A-Za-z0-9_.]+);");
    static regex javeImportRE("import (static |)([A-Za-z][A-Za-z0-9_.*]*);");
    smatch match;
    string package = "none";
    ifstream in(filename);
    RelationPtr crel_ptr = NULL;
    
    while (getJavaLine(in, line, buffer)) {
        replaceAll(line, ", ", ",");
        if (regex_match(line, match, javePackageRE, regex_constants::match_default))
        {
            assert(crel_ptr == NULL);
            package = match[1];
            if (removeParts(package, presenter.parseImports)) {
                Log::warning(LOG_IGNORE).out() << "Missing valid package in file " << filename;
                return 0; // no package
            }
            crel_ptr =
                clist.addClass("class", package, "", filename, package, true);
        }
        else if (regex_match(line, match, javeImportRE, regex_constants::match_default))
        {
            if (crel_ptr == NULL) {
                Log::warning(LOG_IGNORE).out() << "Missing valid package in class file " << filename;
                return 0;   // No valid package
            }
            
            string modifiers = match[1];            // static
            string name = match[2];

            if (removeParts(name, presenter.parseImports)) {
                continue;
            }
            
            replaceAll(name, ".*", "");
            if (false) {
                clist.addParent(crel_ptr, name, filename, package);
            } else {
                bool flip = false;
                if (flip) {
                    RelationPtr child_ptr =
                    clist.addClass("class", package, modifiers, filename, name, false);
                    // child_ptr->addParent(crel_ptr);
                    // crel_ptr->addChild(child_ptr);
                    if (child_ptr != NULL) {
                        clist.addParent(child_ptr, name, filename, package);
                    }
                } else {
                    RelationPtr child_ptr =
                         clist.addClass("class", name, modifiers, filename, name, false);
                    // child_ptr->addParent(crel_ptr);
                    // crel_ptr->addChild(child_ptr);
                    if (child_ptr != NULL) {
                        clist.addParent(child_ptr, package, filename, package);
                    }
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


