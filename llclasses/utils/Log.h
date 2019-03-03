//-------------------------------------------------------------------------------------------------
//
// File: Log.h
// Author: Dennis Lang
// Desc: Log error and warning messages.
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

#include <iostream>
using namespace std;


//  Manage logging message, warnings and errors
class Log
{
    ostream& m_out;
    
public:
    Log(ostream& out) : m_out(out) {}
    ~Log()
    {
        m_out << "\033[00m " << endl;
    }
    
    ostream& out() { return m_out; }
    
    // Example warning message, EOL is automatically added.
    //   Log::warning().out()  << "Duplicate " << stuff << " more stuff";
    inline static Log warning()
    {
        Log warning(cerr);
        warning.out() << "033[01;32m Warning ";
        return warning;
    }
    
    // Example warning message, EOL is automatically added.
    //   Log::error().out()  << "Duplicate " << stuff << " more stuff";
    inline static Log error()
    {
        Log error(cerr);
        error.out() << "\033[01;31m Error ";
        return error;
    }
};
