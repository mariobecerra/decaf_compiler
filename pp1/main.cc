/* File: main.cc
 * -------------
 * This file defines the main() routine for the program and not much else.
 * You should not need to modify this file.
 */
 
#include <string.h>
#include <stdio.h>
#include "utility.h"
#include "errors.h"
#include "scanner.h"
#include "location.h"

/* Function: PrintOneToken()
 * Usage: PrintOneToken(T_Double, "3.5", val, loc);
 * -----------------------------------------------
 * We supply this function to print information about the tokens returned
 * by the lexer as part of pp1.  Do not modifiy it.
 */
static void PrintOneToken(TokenType token, const char *text, YYSTYPE value,
                          yyltype loc)
{
  char buffer[] = {'\'', token, '\'', '\0'};
  const char *name = token >= T_Void ? gTokenNames[token - T_Void] : buffer;
  
  printf("%-12s line %d cols %d-%d is %s ", text,
	   loc.first_line, loc.first_column, loc.last_column, name);
  
  switch(token) {
    case T_IntConstant:     
      printf("(value = %d)\n", value.integerConstant); break;
    case T_DoubleConstant:   
      printf("(value = %g)\n", value.doubleConstant); break;
    case T_StringConstant:  
      printf("(value = %s)\n", value.stringConstant); break;
    case T_BoolConstant:    
      printf("(value = %s)\n", value.boolConstant ? "true" : "false"); break;
    case T_Identifier:
	if (strcmp(text, value.identifier)) {
	  printf("(truncated to %s)\n", value.identifier);
	  break;
	}
    default:
      printf("\n"); break;
  }
}


/* Function: main()
 * ----------------
 * Entry point to the entire program.  We parse the command line and turn
 * on any debugging flags requested by the user when invoking the program.
 * The UNIX popen command is used to first invoke the preprocessor to
 * filter the input and we feed that result into the scanner as input.
 * (This is somewhat unusual -- ordinarily lex would just read directly
 * from the usual stdin, without the calls to popen/yyrestart)
 * InitScanner() is used to set up the scanner.
 * Once everything is set up, we loop, calling yylex() to get each token
 * and print out its info. We continue until all input has been scanned.
 */
int main(int argc, char *argv[])
{
    ParseCommandLine(argc, argv);
    FILE *filtered = popen("./dpp", "r"); // start up the preprocessor
    yyrestart(filtered); // tell lex to read from output of preprocessor
  
    InitScanner();
    TokenType token;
    while ((token = (TokenType)yylex()) != 0) 
        PrintOneToken(token, yytext, yylval, yylloc);
    pclose(filtered);
    return (ReportError::NumErrors() == 0? 0 : -1);
}

