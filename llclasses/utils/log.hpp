//-------------------------------------------------------------------------------------------------
//
// File: Log.h
// Author: Dennis Lang
// Desc: Log error and warning messages to console in color.
//
//-------------------------------------------------------------------------------------------------
//
// Author: Dennis Lang - 2019
// https://landenlabs.com
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

#include "colors.hpp"


#define LOG_BAD_FILE 0  // Always log
#define LOG_DUPLCATE 1
#define LOG_IGNORE 2


//  Manage logging message, warnings and errors
class Log {
protected:
    ostream& m_out;
    bool m_active;

public:
    Log(ostream& out) : m_out(out), m_active(true) {}
    Log(ostream& out, bool active) : m_out(out), m_active(active) {
        if (! active) {
            m_out.setstate(std::ios_base::badbit);
        }
    }
    Log(Log&& other) : m_out(other.m_out), m_active(other.m_active) {
        // move constructore
        other.m_active = false;
    }
    ~Log() {
        if (m_active) {
            m_out << "\033[00m " << endl;
        }
        m_out.clear();
    }


    // Output log levels, message with level higher are ignored.
    static const unsigned int OFF_LEVEL = 0;
    static unsigned int W_LEVEL;
    static unsigned int E_LEVEL;

    virtual ostream& out() { return m_out; }
    // static Log none();

    // Example warning message, EOL is automatically added.
    //   Log::warning(LOG_LEVEL).out()  << "Duplicate " << stuff << " more stuff";
    inline static Log warning(unsigned wLevel) {
        if (wLevel > W_LEVEL)
            return Log(cerr, false);    // Ignore log messages

        Log warning(cerr);
        const char* color = (wLevel < 2) ? GREEN : PINK;
        warning.out() << color << " Warning ";
        return warning;
    }


    // Example warning message, EOL is automatically added.
    //   Log::error(LOG_BAD_FILE).out()  << "Duplicate " << stuff << " more stuff";
    inline static Log error(unsigned eLevel) {
        if (eLevel > E_LEVEL)
            return Log(cerr, false);    // Ignore log messages

        Log error(cerr);
        error.out() << RED " Error ";
        return error;
    }
};


