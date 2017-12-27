/**
 * @file parser.c
 * @author Sergi Reyner <sergi.reyner@gmailcom>
 * @date 27/12/2017
 * @brief Source code parser
 *
 * Here typically goes a more extensive explanation of what the header
 * defines. Doxygens tags are words preceeded by either a backslash @\
 * or by an at symbol @@.
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 */

#include <string.h>

#include "parser.h"

/**
 * @brief Read a token
 *
 * return a troken from the source
 * @param line the source
 * @return a newly allocated string containing the token
 */
char *readToken(const char *line) {
    char *s = (char *) line;
    while(*s && *s != ' ' && *s != '\t' && *s != ';' && *s != '\n')
        s++;
    return strndup(line, s - line);
}

/**
 * @brief Skip whitespace
 *
 * skip whitespace from the source
 * @param line the source
 * @return a pointer to the next non-whitespace character, or the end of the string
 */
char *skipWhitespace(const char *line) {
    char *s = (char *) line;
    while(*s && (*s == ' ' || *s == '\t' || *s == '\n'))
        s++;
    if (*s == ';') /* if there's a comment, skip until the end of the line */
        while(*s)
            s++;
    return s;
}
