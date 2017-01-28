/* File: errors.cc
 * ---------------
 * Implementation for error-reporting class.
 */

#include "errors.h"
#include <iostream>
#include <sstream>
#include <stdarg.h>
#include <stdio.h>
using namespace std;



int ReportError::numErrors = 0;

 
void ReportError::OutputError(yyltype *loc, string msg) {
    numErrors++;
    fflush(stdout); // make sure any buffered text has been output
    if (loc) {
        cerr << endl << "*** Error line " << loc->first_line << "." << endl;
    } else
        cerr << endl << "*** Error." << endl;
    cerr << "*** " << msg << endl << endl;
}


void ReportError::Formatted(yyltype *loc, const char *format, ...) {
    va_list args;
    char errbuf[2048];
    
    va_start(args, format);
    vsprintf(errbuf,format, args);
    va_end(args);
    OutputError(loc, errbuf);
}

void ReportError::UntermComment() {
    OutputError(NULL, "Input ends with unterminated comment");
}

void ReportError::InvalidDirective(int linenum) {
    yyltype ll = {0, linenum, 0, 0};
    OutputError(&ll, "Invalid # directive");
}

void ReportError::LongIdentifier(yyltype *loc, const char *ident) {
    stringstream s;
    s << "Identifier too long: \"" << ident << "\"" << '\0';
    OutputError(loc, s.str());
}

void ReportError::UntermString(yyltype *loc, const char *str) {
    stringstream s;
    s << "Unterminated string constant: " << str << '\0';
    OutputError(loc, s.str());
}

void ReportError::UnrecogChar(yyltype *loc, char ch) {
    stringstream s;
    s << "Unrecognized char: '" << ch << "'" << '\0';
    OutputError(loc, s.str());
}
