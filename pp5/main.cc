

#include <string.h>
#include <stdio.h>
#include "utility.h"
#include "errors.h"
#include "parser.h"


int main(int argc, char *argv[]) {
    ParseCommandLine(argc, argv);

    InitScanner();
    InitParser();
    yyparse();
    return (ReportError::NumErrors() == 0 ? 0 : -1);
}

