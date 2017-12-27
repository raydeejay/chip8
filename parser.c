#include <string.h>

#include "parser.h"

/*******************
 * PARSER
 ******************/

char *skipWhitespace(const char *line) {
    char *s = (char *) line;

    // skip until the next non-whitespace
    while(*s && (*s == ' ' || *s == '\t' || *s == '\n')) s++;

    // if there's a comment, skip until the end of the line
    if (*s == ';') while(*s) s++;

    return s;
}

char *readToken(const char *line) {
    char *s = (char *) line;
    while(*s && *s != ' ' && *s != '\t' && *s != ';' && *s != '\n')
        s++;
    return strndup(line, s - line);
}
