# llclasses
OSX / Linux / DOS  Object Oriented Code parser

Parse C++ or Java files and output class hierarchy relationship. 

  [![Build status](https://travis-ci.org/landenlabs/llclasses.svg?branch=master)](https://travis-ci.org/landenlabs/llclasses)
  [![Known Vulnerabilities](https://snyk.io/test/github/landenlabs/llclasses/badge.svg)](https://snyk.io/test/github/landenlabs/llclasses)

LLClasses supports parsing both **Java** and **C++**. The code is structured with a set of parsers and publishers.  The frontend handles the runtime switches and iterates over directory files
passing them to the **parsers** and **publishers**. 

For further details visit home website which shows how llclasses can parse object oriented files.
[https://landenlabs.com/code/llclasses/index.html]


## Source code files:
<pre>
.
├── ClassRel.cpp
├── ClassRel.h
├── Makefile
├── Parser.cpp
├── Parser.h
├── Presenter.cpp
├── Presenter.h
├── build.csh
├── llclasses.cpp
├── llclasses.h
├── parsers
│   ├── ParseCode.h
│   ├── ParseCpp.cpp
│   ├── ParseCpp.h
│   ├── ParseJava.cpp
│   ├── ParseJava.cpp.bak
│   ├── ParseJava.h
│   └── ParseJava.h.bak
├── publishers
│   ├── PublishHtml.cpp
│   ├── PublishText.cpp
│   ├── PublishText.cpp.bak
│   └── PublishViz.cpp
└── utils
    ├── Colors.h
    ├── DirectoryFiles.cpp
    ├── DirectoryFiles.h
    ├── Log.cpp
    ├── Log.h
    ├── Publish.h
    ├── SwapStream.h
    └── utils.h

3 directories, 29 files
</pre>
 
##  Help screen

<pre>
llclasses (v2.1)

Des: Generate class dependence tree (May  9 2019)
By:  LanDen Labs  www.landenlabs.com
Use: llclasses [-+ntgxshjz] header_files...

Switches (*=default)(-=off, +=on):
  n  ; Show alphabetic class name list
* t  ; Show class dependency tree

Output format (single choice):
* g  ; Use graphics for tree connections
  x  ; Use (+|-) for tree connections
  s  ; Use spaces for tree connections
  h  ; Html tree connections (needs images 0.png, 1.png, 2.png, n.png)
  j  ; Java tree connections (needs dtree www.destroydrop.com/javascript/tree/)
  z  ; GraphViz (see https://graphviz.gitlab.io/)

  With -j to customize java 1=header, 2=body begin, 3=body end
     Specify file which contains html text to insert in output
  -1=head.html -2=bodybegin.html -3=bodyend.html
     Optional replacement words applied to html sections
  -0=x1,foo1 -0=x2,foo2 -0=x3,foo3

Modifiers:
  Z               ; Split GraphViz by tree, use with -O
  N =nodesPerFile ; Split by nodes per file, use with -O
  O =outpath      ; Save output in file
  T =tabular      ; Tabular html
  V =filePattern  ; Ignore file path pattern, ex *Test*
  v =classPattern ; Ignore class pattern, ex [Tt]est[0-9]
  A =allClasses   ; Include Protected and Private classes
  M =level        ; Parse imports instead of Classes
                       level +n=keep left n levels, -n=remove last n levels
  I =interfaces   ; Include Interfaces in report
  i =classPattern ; Include class pattern, opposite of -v
  L =Title        ; Set optional title
  _V_l =logLevel     ; Log level (0=default), -l=2 show extra logging

Examples (assumes java source code in directory src):
  llclasses -t +n  src\*.java  ; *.java prevent recursion
  llclasses -x  src > llclasses.txt
  llclasses -h  dir1 dir2 foo*.java  > llclasses.html
  llclasses -h -I -T "-v=*Test*" src > javaTable.html
  llclasses -j  src > llclassesWithJs.html
  llclasses -j -1=head.html -2=bodybegin.html -3=bodyend.html src > jtrees.html

  -V is case sensitive
  llclasses -z -Z -O=.\viz\ -V=*Test* -V=*Exception* src >directgraph.dot
  llclasses -z -N=10 -O=.\viz\ -V=*Test* -V=*Exception* src >directgraph.dot
</pre>

![screen shot of help text on OSX console](https://raw.githubusercontent.com/landenlabs/llclasses/master/llclasses-help.png)

## Links

Details on website
[https://landenlabs.com/code/llclasses/index.html]

Previous unsupported legacy program which only parsed Java
[https://landenlabs.com/code/javatree/index.html]

Example diagrams of Android classes generated using llclasses
[https://landenlabs.com/android/classtree/classtree.html]

Home website
[https://landenlabs.com](https://landenlabs.com)

## Sample reports

[HTML report](https://raw.githubusercontent.com/landenlabs/llclasses/master/screenshots/report-html.png)

[Simple list](https://raw.githubusercontent.com/landenlabs/llclasses/master/screenshots/report-list.png)

[Tabular report](https://raw.githubusercontent.com/landenlabs/llclasses/master/screenshots/report-tabular.jpg)

[Text report](https://raw.githubusercontent.com/landenlabs/llclasses/master/screenshots/report-text.png)

[Html tree report](https://raw.githubusercontent.com/landenlabs/llclasses/master/screenshots/report-tree.png)

## License

```  
Copyright 2020 Dennis Lang  
  
Licensed under the Apache License, Version 2.0 (the "License");  
you may not use this file except in compliance with the License.  
You may obtain a copy of the License at  
  
 http://www.apache.org/licenses/LICENSE-2.0  
Unless required by applicable law or agreed to in writing, software  
distributed under the License is distributed on an "AS IS" BASIS,  
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
See the License for the specific language governing permissions and  
limitations under the License.  
```  