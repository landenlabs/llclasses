//-------------------------------------------------------------------------------------------------
//
// File: directory.cpp
// Author: Dennis Lang
// Desc: Get files from directories
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

#include "directoryfiles.hpp"

#include <iostream>

#ifdef HAVE_WIN
    #include <windows.h>
#endif

#include <stdio.h>
#include <errno.h>

#ifdef WIN32

const string ANY("\\*");

//-------------------------------------------------------------------------------------------------
// Return true if attribute is a Directory
inline static bool isDir(DWORD attr) {
    return (attr != -1) && ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0);
}

//-------------------------------------------------------------------------------------------------
// Return 'clean' full path, remove extra slahes.
static string& GetFullPath(string& fname) {
    char fullPath[MAX_PATH];
    DWORD len1 = GetFullPathName(fname.c_str(), ARRAYSIZE(fullPath), fullPath, NULL);
    fname = fullPath;
    return fname;
}

//-------------------------------------------------------------------------------------------------

DirectoryFiles::DirectoryFiles(const string& dirName) :
    my_dir_hnd(INVALID_HANDLE_VALUE),
    my_dirName(dirName) {
}

//-------------------------------------------------------------------------------------------------

DirectoryFiles::~DirectoryFiles() {
    if (my_dir_hnd != INVALID_HANDLE_VALUE)
        FindClose(my_dir_hnd);
}

//-------------------------------------------------------------------------------------------------

void DirectoryFiles::close() {
    if (my_dir_hnd != INVALID_HANDLE_VALUE) {
        FindClose(my_dir_hnd);
        my_dir_hnd = INVALID_HANDLE_VALUE;
    }
}

//-------------------------------------------------------------------------------------------------

bool DirectoryFiles::begin() {
    close();

    string dir = my_dirName;
    if (dir.empty())
        dir = ".\\";    // Default to current directory

    DWORD attr = GetFileAttributes(dir.c_str());
    if (isDir(attr)) {
        dir += ANY;
    } else { // if (attr != INVALID_FILE_ATTRIBUTES)
        GetFullPath(my_dirName);
        // Peel off one subdir from reference name.
        size_t pos = my_dirName.find_last_of(":/\\");
        if (pos != string::npos)
            my_dirName.resize(pos);
    }

    my_dir_hnd = FindFirstFile(dir.c_str(), &my_dirent);
    bool is_more = (my_dir_hnd != INVALID_HANDLE_VALUE);

    while (is_more
        && (isDir(my_dirent.dwFileAttributes)
    && strspn(my_dirent.cFileName, ".") == strlen(my_dirent.cFileName) )) {
        is_more = (FindNextFile(my_dir_hnd, &my_dirent) != 0);
    }

    return is_more;
}

//-------------------------------------------------------------------------------------------------

bool DirectoryFiles::more() {
    if (my_dir_hnd == INVALID_HANDLE_VALUE)
        return begin();

    bool is_more = false;
    if (my_dir_hnd != INVALID_HANDLE_VALUE) {
        // Determine if there any more files
        //   skip any dot-directories.
        do {
            is_more = (FindNextFile(my_dir_hnd, &my_dirent) != 0);
        } while (is_more
            && (isDir(my_dirent.dwFileAttributes)
        && strspn(my_dirent.cFileName, ".") == strlen(my_dirent.cFileName)));

    }

    return is_more;
}

//-------------------------------------------------------------------------------------------------

bool DirectoryFiles::is_directory() const {
    return (my_dir_hnd != INVALID_HANDLE_VALUE && isDir(my_dirent.dwFileAttributes));
}

//-------------------------------------------------------------------------------------------------

const char* DirectoryFiles::name() const {
    return (my_dir_hnd != INVALID_HANDLE_VALUE) ?
        my_dirent.cFileName : NULL;
}

//-------------------------------------------------------------------------------------------------

string& DirectoryFiles::fullName(string& fname) const {
    fname = my_dirName + DIR_SLASH_STR + name();
    return GetFullPath(fname);
}


#else

// #include <dirent.h>
#include <unistd.h>
#include <stdlib.h>

//-------------------------------------------------------------------------------------------------

bool DirectoryFiles::isFile(const string& filepath) {
    struct stat filestat;
    if (stat( filepath.c_str(), &filestat ))
        return false;
    return (S_ISREG( filestat.st_mode ));
}

//-------------------------------------------------------------------------------------------------

DirectoryFiles::DirectoryFiles(const string& dirName) :
    my_pDir(NULL),
    my_pDirEnt(NULL) {
    realpath(dirName.c_str(), my_fullname);
    my_baseDir = my_fullname;
    my_pDir = opendir(my_baseDir.c_str());
    my_is_more = (my_pDir != NULL);
    my_is_file = isFile(dirName);
}

//-------------------------------------------------------------------------------------------------

DirectoryFiles::~DirectoryFiles() {
    if (my_pDir != NULL)
        closedir(my_pDir);
}

//-------------------------------------------------------------------------------------------------

bool DirectoryFiles::more() {
    if (my_is_file) {
        my_is_file = false;
        return true;
    }

    if (my_is_more) {
        my_pDirEnt = readdir(my_pDir);
        my_is_more = my_pDirEnt != NULL;
        if (my_is_more) {
            if (my_pDirEnt->d_type == DT_DIR) {
                while (my_is_more &&
                (my_pDirEnt->d_name[0] == '.' && ! isalnum(my_pDirEnt->d_name[1]))) {
                    more();
                }
            }
        }
    }

    return my_is_more;
}

//-------------------------------------------------------------------------------------------------

bool DirectoryFiles::is_directory() const {
    return my_pDirEnt != NULL && my_pDirEnt->d_type == DT_DIR;
}

//-------------------------------------------------------------------------------------------------

string& DirectoryFiles::fullName(string& fname) const {
    if (my_pDirEnt != NULL) {
        fname = my_baseDir + DIR_SLASH_STR + my_pDirEnt->d_name;
        // return realpath(fname.c_str(), my_fullname);
        return fname;
        // return GetFullPath(fname);
    }

    fname = my_baseDir;
    return fname;;
}

#endif
