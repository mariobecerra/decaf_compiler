/*
 * file:  dpp.l
 * ------------
 * Lex inupt file to generate the scanner for the preprocessor.
 * If you decide to use lex for the preprocesor, put your rules
 * here, otherwise the file can remain empty.
 */

%{
#include <map>
#include <string>
#include "errors.h"

using namespace std;

std::map<std::string, std::string> mapOfMacros;

int line_num = 1;

%}

ONELINE_COMMENT \/\/.*
MULTILINE_COMMENT \/\*([^*]|[\n]|(\*+([^*/]|[\n])))*\*+\/
NAME [A-Z]+
MACRO_DECLARATION "#define"\ {NAME}\ .*
MACRO_USAGE \#{NAME}

%%

\n {line_num++;
	string texto = strdup(yytext);
	printf("%s", texto.c_str());
}

{ONELINE_COMMENT} { }

{MULTILINE_COMMENT} { }

{MACRO_DECLARATION} { 
	
	string name_replacement, name, replacement, texto;
	int find_space;

	texto = strdup(yytext);

	name_replacement = texto.substr(8); // NAME replacement
	find_space = name_replacement.find(" "); // ubica dónde está el primer espacio en name_replacement
	name = name_replacement.substr(0, find_space);
	replacement = name_replacement.substr(find_space);
	mapOfMacros.insert(make_pair(name, replacement)); // mapOfMacros[name]=replacement;

}

{MACRO_USAGE} {
	string texto, name, replacement;
	texto = strdup(yytext);
	
	name = texto.substr(1);
	if(mapOfMacros.find(name) == mapOfMacros.end()){
		ReportError::InvalidDirective(line_num); 
	} else {
		replacement = mapOfMacros.find(name)->second;
		printf("%s", replacement.c_str());
	}
}

. {
	string texto = strdup(yytext);
	printf("%s", texto.c_str());
}

%%
