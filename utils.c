#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// string trimming code from
// https://rosettacode.org/wiki/Strip_whitespace_from_a_string/Top_and_tail#C

char *rtrim(const char *s)
{
    while( isspace(*s) || !isprint(*s) ) ++s;
    return strdup(s);
}

char *ltrim(const char *s)
{
    char *r = strdup(s);
    if (r != NULL)
    {
        char *fr = r + strlen(s) - 1;
        while( (isspace(*fr) || !isprint(*fr) || *fr == 0) && fr >= r) --fr;
        *++fr = 0;
    }
    return r;
}

char *trim(const char *s)
{
    char *r = rtrim(s);
    char *f = ltrim(r);
    free(r);
    return f;
}
