

#ifndef YYLTYPE


typedef struct yyltype
{
    int timestamp;                 
    int first_line, first_column;
    int last_line, last_column;
    char *text;                    
} yyltype;

#define YYLTYPE yyltype


extern struct yyltype yylloc;


inline yyltype Join(yyltype first, yyltype last) {
    yyltype combined;
    combined.first_column = first.first_column;
    combined.first_line = first.first_line;
    combined.last_column = last.last_column;
    combined.last_line = last.last_line;
    return combined;
}


inline yyltype Join(yyltype *firstPtr, yyltype *lastPtr) {
    return Join(*firstPtr, *lastPtr);
}

#endif

