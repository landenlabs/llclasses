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

// 4291 - No matching operator delete found
#pragma warning( disable : 4291 )

#include <stdio.h>
#include <ctype.h>
#include <fstream>
#include <iostream>
#include <iomanip>

#include <vector>
#include <map>
#include <algorithm>
#include <regex>

#include "presenter.hpp"
#include "SwapStream.hpp"
#include "utils.hpp"


string dtree_css = "dtree.css";
string dtree_js = "dtree.js";
static char HTML_HEAD[] =
    "<head> \n"
    "<meta name=\"keywords\" content=\"Java,class,hierarchy,tree,diagram\"> \n"
    "<meta name=\"description\" content=\"Java class hierachy\">\n"
    "<meta name=\"author\" content=\"Dennis Lang\">\n"
    "\n"
    "<!-- Mobile -->\n"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, maximum-scale=1\">\n"
    "\n"
    "<link rel=\"icon\" type=\"image/ico\" href=\"favicon.ico\" > \n"
    "<link rel=\"shortcut icon\" href=\"/favicon.ico\" >\n"
    "\n"
    "<meta http-equiv=\"Content-type\" content=\"text/html;charset=UTF-8\" >\n"
    "<META http-equiv=\"Content-Style-Type\" content=\"text/css\">\n"
    "\n"
    "<!-- Google analytics tracking -->\n"
    "<script type=\"text/javascript\" src=\"~/scripts.js\"></script>\n"
    "\n"
    "<style> \n"
    "    body { \n"
    "    background: url(\"bg.jpg\");\n"
    "    }\n"
    "</style>\n";

// -------------------------------------------------------------------------------------------------
static void outputHtmlPrefix1() {
    cout <<
    "<!DOCTYPE html>\n"
    "<html lang=\"en\">\n";
}

// -------------------------------------------------------------------------------------------------
void PublishHtml::outputHtmlMetaHeader2(const char* auxStyle) const {
    if  (htmlHead.empty()) {
        cout << HTML_HEAD;
        outputHtmlTitle3();
        cout <<
        "    <link rel=StyleSheet href=" << dtree_css << " type=text/css /> \n"
        "    <script type=text/javascript src=" << dtree_js << " ></script>  \n";
        cout << ((auxStyle != NULL) ? auxStyle : "");
        cout <<
        "</head> \n";
    } else {
        if (auxStyle != NULL) {
            size_t pos = htmlHead.rfind("</head>");
            if (pos != string::npos) {
                htmlHead.insert(pos, auxStyle);
            }
        }
        cout << htmlHead;
    }
}

// -------------------------------------------------------------------------------------------------
void PublishHtml::outputHtmlTitle3() const {
    const string& title1 = getIt(presenter.titles, 0, "Class List");
    const string& title2 = getIt(presenter.titles, 1, "");
    cout << "    <title>" << title1 << " " << title2 << "</title> \n";
}

// -------------------------------------------------------------------------------------------------
//   Access css images
//   <img class="img0">
//   0 = vertical line
//   1 = L shape
//   2 = Side T
//   n = empty
static const char* outputEmbeddedImages =
    "<style>\n"
    "img.img0 {\n"
    "content: url(data:image/gif;base64,iVBORw0KGgoAAAANSUhEUgAAABQAAAAUCAYAAACNiR0NAAAAk0lEQVR4nL3UsQqDMBSF4XuTTDdj19L3f67StWOg0lzjosMpmDYe6Vn+QfkgItHWWpOd3a4XKaVIzlnuj+fea7DQezhNLygNHtkXUD9Kg+PrgiklKA/GCKXB6g6lQV8hPws8si6oIUB5UBVKg/PsUBqs7wqlwe0i6lxIY+Dp3zDGAKXB7aQDJ/7zj21moqpiZj+DC+HEMjOdUS71AAAAAElFTkSuQmCC);\n"
    "width: 32px;\n"
    "height: 32px;\n"
    "}\n"
    "img.img1 {\n"
    "content: url(data:image/gif;base64,iVBORw0KGgoAAAANSUhEUgAAABQAAAAUCAYAAACNiR0NAAAA0ElEQVR4nO3TwQqCQBAG4H/US6ipmxUE0QNI7x9ETxEGlZfoWNrBhTTM6eBJyEU0gqD/ModZPmZhhpiZ0ZDFfAwpU1iWjdP50vSsFk3VzLJ7rfYGu+S7IIFqtT9IFcRgRNGxP6gZegVyic161R80dB1EADPhGsetQFLt4WzqIH/kABEs04KUqRIb+RM16IsBFO23+fjaGKqmECMkSQxmhmmasO0hHNeD5woUzwKH/Q6plOCyrKYjTf3lMNziliTwhEAQLFtNqAS75Mdv+Q+2ygvsm1AVZxgGvgAAAABJRU5ErkJggg==);\n"
    "width: 32px;\n"
    "height: 32px;\n"
    "}\n"
    "img.img2 {\n"
    "content: url(data:image/gif;base64,iVBORw0KGgoAAAANSUhEUgAAABQAAAAUCAYAAACNiR0NAAAAr0lEQVR4nNWUQQvCMAyFX5eKg3oT8SL+/98lDrwIXnooJIuXqVTWdmtB8EF5lIYvKWljVFWR0Pm0h/cezjlcrvdUWKQudxhCiLwZWKM/BxqYyJuBCo28GVgjuySImXE87KadQmTE3PMlojyQOgKD39CSRCR/Zbv55LPWTotgzHyTihUKCwCg3/YYbo9ihUChKaJj5M3AGuWBr06mB9Ja4Jc3AyuIha+3FvdrYM20eQJd5D4GgV/8lwAAAABJRU5ErkJggg==);\n"
    "width: 32px;\n"
    "height: 32px;\n"
    "}\n"
    "img.imgN {\n"
    "content: url(data:image/gif;base64,iVBORw0KGgoAAAANSUhEUgAAABQAAAAUAQMAAAC3R49OAAAAA1BMVEX///+nxBvIAAAAAXRSTlMAQObYZgAAAAFiS0dEAIgFHUgAAAAMSURBVBjTY2CgLgAAAFAAAZ7SdgsAAAAASUVORK5CYII=);\n"
    "width: 32px;\n"
    "height: 32px;\n"
    "}\n"
    "</style>\n";


// -------------------------------------------------------------------------------------------------
static const char* outputHtmlTableStyle =
    "<style type = 'text/css'>\n"
    "#gradient-style\n"
    "{\n"
    "    font-family: 'Lucida Sans Unicode', 'Lucida Grande', Sans-Serif;\n"
    "    font-size: 12px;\n"
    "    margin: 45px;\n"
    "    width: 480px;\n"
    "    text-align: left;\n"
    "    border-collapse: collapse;\n"
    "}\n"
    "#gradient-style th\n"
    "{\n"
    "    font-size: 13px;\n"
    "    font-weight: normal;\n"
    "    padding: 8px;\n"
    "    background: #b9c9fe url('img/gradhead.png') repeat-x;\n"
    "    border-top: 2px solid #d3ddff;\n"
    "    border-bottom: 1px solid #fff;\n"
    "    color: #039;\n"
    "}\n"
    "#gradient-style td\n"
    "{\n"
    "    padding: 8px;\n"
    "    border-bottom: 1px solid #fff;\n"
    "    color: #669;\n"
    "    border-top: 1px solid #fff;\n"
    "    background: #e8edff url('img/gradback.png') repeat-x;\n"
    "}\n"
    "#gradient-style tfoot tr td\n"
    "{\n"
    "    background: #e8edff;\n"
    "    font-size: 12px;\n"
    "    color: #99c;\n"
    "}\n"
    "#gradient-style tbody tr:hover td\n"
    "{\n"
    "   background: #d0dafd url('img/gradhover.png') repeat-x;\n"
    "   color: #339;\n"
    "}\n"
    "</style>\n"

    "<link rel='stylesheet' type='text/css' href='http://cdn.datatables.net/1.10.16/css/jquery.dataTables.min.css'> \n"
    "<script type='text/javascript' language='javascript' src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script> \n"
    "<script type='text/javascript' charset='utf8' src='http://cdn.datatables.net/1.10.16/js/jquery.dataTables.min.js'></script> \n"
    "\n"
    "<script type='text/javascript' class='init'> \n"
    "\n"
    "$(document).ready(function() { \n"
    "  $('#gradient-style').DataTable({ \n"
    "    'scrollY':        '600px', \n"
    "    'scrollCollapse' : false, \n"
    "    'paging' : false \n"
    "  }); \n"
    "}); \n"
    "</script> \n";


// ------------------------------------------------------------------------------------------------
static void outputHtmlTableList(const ClassList& clist, Presenter& presenter) {
    time_t nowTime;
    time(&nowTime);

    cout <<
    "<table id='gradient-style' summary='display' ellspacing='0' width='100%'  >\n"
    "<thead>\n"
    "<tr>\n"
    "<th scope='col'>Package</th> \n";

    if (presenter.showInterfaces)
        cout << "<th scope='col'>Type</th> \n";

    cout <<
//    "<th scope='col'>FullName</th> \n"
    "<th scope='col'>Name</th> \n"
    "<th scope='col'>Modifiers</th> \n"
    "<th scope='col'>Filename</th> \n"
    "</tr>\n"
    "</thead>\n"
    "<tfoot>\n"
    "<tr>\n"
    "<td colspan='5'><a href=\"http://landenlabs.com\"> LanDen Labs - Dennis Lang </a></td><td>"
        << dateTimeToString(now(), "%d-%h-%Y")
        << "</td> \n"
    "</tr> \n"
    "</tfoot> \n"
    "<tbody>\n";

    ClassList::const_iterator iter;
    RelationPtr crel_ptr;
    string basepath;

    for (iter = clist.begin(); iter != clist.end(); iter++) {
        crel_ptr = iter->second;
        basepath = equalSubStr(basepath, crel_ptr->filename);
    }

    size_t pathOffset = basepath.rfind(DIR_SLASH_CHR) +1;
    for (iter = clist.begin(); iter != clist.end(); iter++) {
        crel_ptr = iter->second;

        if (presenter.canShow(crel_ptr)) {
            string name = crel_ptr->name;
            if (name.length() > 20)
                replaceAll(name, ".", ".<br>");

            cout << "<tr>"
                << " <td>" << crel_ptr->package;

            if (presenter.showInterfaces)
                cout << " <td>" << crel_ptr->type;

            cout
            //         << " <td>" << crel_ptr->name    // fullClassName
                    << " <td>" << name
                        << " <td>" << crel_ptr->modifier
                        << " <td>" << (crel_ptr->filename.c_str() + pathOffset)
                        << endl;
        }
    }

    cout  <<
    "</tbody>\n"
    "</table>\n";

}

// -------------------------------------------------------------------------------------------------
size_t PublishHtml::displayChildren(
    unsigned parentNum,
    size_t width,
    const RelationPtr parentPtr,
    const string& basepath) const {
    size_t nodeCnt = 0;
    RelationPtr child_ptr;

    const RelationList& children =  parentPtr->children;
    if (! children.empty()) {
        for (RelationList::const_iterator iter = children.begin(); iter != children.end(); iter++) {
            child_ptr = *iter;

            if (child_ptr != NULL) {
                string chilNname = child_ptr->name;
                if ((child_ptr->definition && presenter.canShow(child_ptr))
                    || presenter.canShowChildren(child_ptr)) {
                    presenter.hasShown(child_ptr);

                    cout << "d.add(" << presenter.sNodeNum;
                    cout << "," << parentNum << ",'" << replaceAll(chilNname, "<", "&lt;");
                    if (! child_ptr->modifier.empty() && child_ptr->modifier != "public")
                        cout << " (" << child_ptr->modifier << ")";
                    if (child_ptr->type != "class")
                        cout << " [" << child_ptr->type << "]";
                    if (child_ptr->definition) {
                        if (basepath.empty()) {
                            cout << "','" << child_ptr->filename;
                        } else {
                            cout << "',p+'" << replaceAll(child_ptr->filename, basepath, "");
                        }
                    } else {
                        cout << " { Imported }";
                        if (basepath.empty()) {
                            cout << "','" << chilNname;
                        } else {
                            cout << "',i+'" << chilNname;
                        }
                    }
                    cout << "');\n";
                }
                nodeCnt++;
                nodeCnt += displayChildren(presenter.sNodeNum++, width, child_ptr, basepath);
            }
        }
    }
    return nodeCnt;
}


// -------------------------------------------------------------------------------------------------
void PublishHtml::present() const {

    const string& title1 = getIt(presenter.titles, 0, "Class List");
    string dataStr = dateTimeToString(now(), "%d-%h-%Y");
    const string& title2 = getIt(presenter.titles, 1, dataStr);
    const string& title3 = getIt(presenter.titles, 2, "title1");

    presenter.applyReplacements(htmlHead);
    presenter.applyReplacements(bodyBegin);
    presenter.applyReplacements(bodyEnd);

    /*
    string basepath;
    ClassList::const_iterator iter;
    for (iter = clist.begin(); iter != clist.end(); iter++)
    {
        RelationPtr crel_ptr = iter->second;
        // fileWidth = max(fileWidth, crel_ptr->filename.length());
        // nameWidth = max(nameWidth, crel_ptr->name.length());
        // modWidth = max(modWidth, crel_ptr->modifier.length());
        basepath = equalSubStr(basepath, crel_ptr->filename);
    }
     */

    if (presenter.tabularList) {
        outputHtmlPrefix1();
        outputHtmlMetaHeader2(outputHtmlTableStyle);

        // outputHtmlTitle3();
        cout <<
        "<body id='top'> \n"
        "<a href=javascript:window.history.back();> Back </a> \n"
        "<p> \n"
        " \n"
        "<p><a href=\"http://landenlabs.com\"> LanDen Labs - Dennis Lang </a>\n"
        "  Created on:" << dateTimeToString(now(), "%d-%h-%Y ")
            << "<p>\n"
        "<h2>Tabular List of " << title1 << "</h2>"
        "<body>\n";
        outputHtmlTableList(clist, presenter);
        cout <<
        "</body> \n"
        "</html> \n"
        "\n";
    } else if (presenter.cset == Presenter::JAVA_CHAR) {
        outputHtmlPrefix1();
        outputHtmlMetaHeader2(NULL);

        if (bodyBegin.empty()) {
            cout <<
            "<body id='top'> \n"
            "<a href=javascript:window.history.back();> Back </a> \n"
            "<p> \n"
            " \n"
            "<h1> " << title1 << " </h1> \n"
            " \n"
            "<h2>" << title2 << "</h2> \n"
            "<div class=dtree style='margin:20px; font-size:110%; box-shadow: 10px 10px 5px #888888; padding:10px; display:inline-block; background:#f0f0f0;'> \n"
            "<p> \n"
            "<span style='border:2px ridge #404040 ; border-radius:20px; padding:5px; background:#e0e0e0;'> \n"
            "<a href=javascript:d.openAll();> <font style='font-size:150%'>open all</font></a> \n"
            "</span> \n"
            "&nbsp; \n"
            "<span style='border:2px ridge #404040 ; border-radius:20px; padding:5px; background:#e0e0e0;'> \n"
            "<a href=javascript:d.closeAll();> <font style='font-size:150%'>close all</font></a> \n"
            "</span> \n"
            "</p> \n"
            "<script type=text/javascript> \n"
            "<!-- \n"
            "d = new dTree('d'); \n"
            "d.add(0, -1, '" << title3 << "'); \n";
        } else {
            cout << bodyBegin;
        }

        displayDependencies();

        cout <<
        "        document.write(d); \n"
        "       d.openAll();\n"
        "        //--> \n"
        "    </script> \n";

        if (bodyEnd.empty()) {
            cout <<
            "</div>  \n"
            "<p><a href=\"http://landenlabs.com\"> LanDen Labs - Dennis Lang </a>\n"
            "  Created on:" << dateTimeToString(now(), "%d-%h-%Y ")
                << "\n"
            "</body> \n"
            "</html> \n"
            "\n";
        } else {
            cout << bodyEnd;
        }
    } else {
        PublishText::present();
    }
}

// -------------------------------------------------------------------------------------------------
void PublishHtml::displayDependencies() const {
    RelationPtr crel_ptr;

    // fputs(doc_classes[cset], stdout);
    switch (presenter.cset) {
    case Presenter::HTML_CHAR:
        fputs(
            "<html>\n"
            "<head>\n"
            "<style>\n"
            ".table-fixed thead th {\n"
            "position: sticky;\n"
            "position: -webkit-sticky;\n"
            "top: 0;\n"
            "z-index: 999;\n"
            "background-color: #ccc;\n"
            "color: #fff;\n"
            "}\n"
            "</style>\n"
            "</head>\n"
            "<body>\n"
            "<table class='table-fixed'>\n"
            "<thead>\n"
            "<tr>\n"
            "<th>File Path</th>\n"
            "<th>Scope</th>\n"
            "<th>Class</th>\n"
            "</tr>\n"
            "</thead>\n"
            "<tbody>\n"
            , stdout);
        break;
    }

    ClassList::const_iterator iter;

    size_t fileWidth = 14;
    size_t nameWidth = 14;
    size_t modWidth  = 6;   // public
    string basepath;

    for (iter = clist.begin(); iter != clist.end(); iter++) {
        crel_ptr = iter->second;
        // cprel_ptr= crel_ptr->parents().relations;

        fileWidth = max(fileWidth, crel_ptr->filename.length());
        nameWidth = max(nameWidth, crel_ptr->name.length());
        modWidth = max(modWidth, crel_ptr->modifier.length());
        basepath = equalSubStr(basepath, crel_ptr->filename);
    }

    SwapStream swapStream(cout);

    if (basepath.length() > 10) {
        if (presenter.cset == Presenter::JAVA_CHAR) {
            cout << "\np='" <<  basepath << "';"; // p=base path of file
            cout << "\ni='" <<  basepath << "';\n"; // i=imported (not part of this file)
        }
    } else {
        basepath.clear();
    }
    fileWidth -= basepath.length();

    for (iter = clist.begin(); iter != clist.end(); iter++) {
        crel_ptr = iter->second;
        // if (crel_ptr->definition == false)
        //    continue;

        if (crel_ptr->isSuper() && presenter.canShowChildren(crel_ptr)) {
            presenter.hasShown(crel_ptr);

            // Have super class - now display subclasses.
            if (presenter.cset == Presenter::JAVA_CHAR) {
                cout << "d.add(" << presenter.sNodeNum;
                string name = crel_ptr->name;
                cout << "," << 0 << ",'" << replaceAll(name, "<", "&lt;");
                if (! crel_ptr->modifier.empty() && crel_ptr->modifier != "public")
                    cout << " (" << crel_ptr->modifier << ")";
                if (crel_ptr->type != "class")
                    cout << " [" << crel_ptr->type << "]";

                if (crel_ptr->definition) {
                    if (basepath.empty()) {
                        cout  << "','" << crel_ptr->filename;
                    } else {
                        cout  << "',p+'" << replaceAll(crel_ptr->filename, basepath, "");
                    }
                } else {
                    cout << " { Imported }";
                    if (basepath.empty()) {
                        cout  << "','" << name;
                    } else {
                        cout  << "',i+'" << name;
                    }
                }
                cout << "');\n";
                displayChildren(presenter.sNodeNum++, fileWidth, crel_ptr, basepath);
            } else {
                string filename = crel_ptr->filename;
                replaceAll(filename, basepath, "");
                fputs("<tr><td>", stdout); // fputs(doc_classesBLine[presenter.cset], stdout);
                printf("%s<td> %s <td> <b>%s</b> ",
                    filename.c_str(),
                    crel_ptr->modifier.c_str(),
                    crel_ptr->name.c_str()
                );
                presenter.displayInterfaces(crel_ptr);
                fputs("</tr>\n", stdout);   // fputs(doc_classesELine[presenter.cset], stdout);
                Indent indent;
                PublishText::displayChildren(indent, fileWidth, basepath.length(), modWidth, crel_ptr);
            }
        }
    }

    // fputs(doc_end[cset], stdout);
    switch (presenter.cset) {
    case Presenter::HTML_CHAR:
        fputs("</tbody></table>\n", stdout);
        fprintf(stdout, "<p><a href=\"http://landenlabs.com\"> Generatedy by llclasses - Dennis Lang </a>");
        fprintf(stdout, " on %s\n",  dateTimeToString(now(), "%Y-%m-%d %H:%M:%S").c_str());
        fputs("</body>\n</html>\n", stdout);
        break;
    }
}
