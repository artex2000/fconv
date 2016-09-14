#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_SYMBOLS 20

typedef enum {
    String,
    Kerning,
    XScale,
    YScale,
    Feedrate,
    Zbound,
    Fontname,
    Origin,
    Undefined
} token_t;

typedef enum {
    alpha,
    number,
    token
} strtype_t;


static FILE *fp;
static char str[256];
static int line_num = 0;

static int conf_ok = 0;
static int has_string = 0;
static int has_kerning = 0;
static int has_xscale = 0;
static int has_yscale = 0;
static int has_feedrate = 0;
static int has_zbound = 0;
static int has_fontname = 0;
static int has_origin = 0;

static char default_string[] = "Test";
static char conf_string[MAX_SYMBOLS];

static double default_kern[] = {0.2, 0.2, 0.2, 0};
static double conf_kern[MAX_SYMBOLS];

static double default_xscale = 0;
static double conf_xscale;

static double default_yscale = 0;
static double conf_yscale;

static int default_feedrate = 100;
static int conf_feedrate;

static double default_hang = -2.5;
static double default_drill = -4;
static double conf_hang;
static double conf_drill;

static char default_fontname[] = "Gunplay 12";
static char conf_fontname[256];

static double default_xorigin = 0;
static double default_yorigin = 0;
static double conf_xorigin;
static double conf_yorigin;

char * get_string (void)
{
    return (conf_ok && has_string) ? conf_string : default_string;
}

double * get_kern (void)
{
    return (conf_ok && has_kerning) ? conf_kern : default_kern;
}

double get_xscale (void)
{
    return (conf_ok && has_xscale) ? conf_xscale : default_xscale;
}

double get_yscale (void)
{
    return (conf_ok && has_yscale) ? conf_yscale : default_yscale;
}

int get_feedrate (void)
{
    return (conf_ok && has_feedrate) ? conf_feedrate : default_feedrate;
}

double get_hang (void)
{
    return (conf_ok && has_zbound) ? conf_hang : default_hang;
}

double get_drill (void)
{
    return (conf_ok && has_zbound) ? conf_drill : default_drill;
}

char * get_fontname (void)
{
    return (conf_ok && has_fontname) ? conf_fontname : default_fontname;
}

double get_xorigin (void)
{
    return (conf_ok && has_origin) ? conf_xorigin : default_xorigin;
}

double get_yorigin (void)
{
    return (conf_ok && has_origin) ? conf_yorigin : default_yorigin;
}

static token_t get_token (char *s)
{
    if (!strcmp (s, "[String]"))
        return String;
    else if (!strcmp (s, "[Kerning]"))
        return Kerning;
    else if (!strcmp (s, "[XScale]"))
        return XScale;
    else if (!strcmp (s, "[YScale]"))
        return YScale;
    else if (!strcmp (s, "[Feedrate]"))
        return Feedrate;
    else if (!strcmp (s, "[Zbound]"))
        return Zbound;
    else if (!strcmp (s, "[Fontname]"))
        return Fontname;
    else if (!strcmp (s, "[Origin]"))
        return Origin;
    else
        return Undefined;
}

static int get_conf_line (strtype_t type, char **s)
{
    char *p = str;
    int sz;

    while (fgets (p, 255, fp) != NULL) {
        line_num++;
        if (*p == '#') {      //skip comment line
            p = str;
            continue;
        }
        while (*p == ' ' || *p == '\t') //skip white space
            p++;
        if (*p == '\n') { //skip empty string
            p = str;
            continue;
        }
        if (type == token && *p != '[')
            return 1;
        if (type == alpha && !isalpha (*p))
            return 1;
        if (type == number && !isdigit (*p) && *p != '-')
            return 1;
        sz = strlen (p);
        if (p[sz - 1] == '\n')
            p[sz - 1] = 0;
        *s = p;
        return 0;
    }
    *s = NULL;
    return 0;
}

static int set_string (void)
{
    int r;
    char *s;

    r = get_conf_line (alpha, &s);
    if (r) {
        printf ("line %d: Syntax error, printable string is expected\n", line_num);
        return 1;
    }

    if (s == NULL) {
        printf ("Unexpected end of file\n");
        return 1;
    }

    has_string = 1;
    s[MAX_SYMBOLS - 1] = 0;
    strcpy (conf_string, s);
    return 0;
}

static int set_fontname (void)
{
    int r;
    char *s;

    r = get_conf_line (alpha, &s);
    if (r) {
        printf ("line %d: Syntax error, font name is expected\n", line_num);
        return 1;
    }

    if (s == NULL) {
        printf ("Unexpected end of file\n");
        return 1;
    }

    has_fontname = 1;
    s[255] = 0;
    strcpy (conf_fontname, s);
    return 0;
}

static int set_scale (int x)
{
    int r;
    char *s;
    double v;

    r = get_conf_line (number, &s);
    if (r) {
        printf ("line %d: Syntax error, number is expected\n", line_num);
        return 1;
    }

    if (s == NULL) {
        printf ("Unexpected end of file\n");
        return 1;
    }

    v = atof (s);
    if (v == 0) {
        printf ("line %d: Invalid value\n", line_num);
        return 1;
    }

    if (x) {
        has_xscale = 1;
        conf_xscale = v;
    } else {
        has_yscale = 1;
        conf_yscale = v;
    }

    return 0;
}

static int set_feedrate (void)
{
    int r, v;
    char *s;

    r = get_conf_line (number, &s);
    if (r) {
        printf ("line %d: Syntax error, number is expected\n", line_num);
        return 1;
    }

    if (s == NULL) {
        printf ("Unexpected end of file\n");
        return 1;
    }

    v = atoi (s);
    if (v == 0) {
        printf ("line %d: Invalid value\n", line_num);
        return 1;
    }

    has_feedrate = 1;
    conf_feedrate = v;

    return 0;
}

static int set_zbound (void)
{
    int r;
    char *s;
    double v;

    r = get_conf_line (number, &s);
    if (r) {
        printf ("line %d: Syntax error, number is expected\n", line_num);
        return 1;
    }

    if (s == NULL) {
        printf ("Unexpected end of file\n");
        return 1;
    }

    v = atof (s);
    if (v == 0) {
        printf ("line %d: Invalid value\n", line_num);
        return 1;
    }

    conf_hang = v;

    r = get_conf_line (number, &s);
    if (r) {
        printf ("line %d: Syntax error, number is expected\n", line_num);
        return 1;
    }

    if (s == NULL) {
        printf ("Unexpected end of file\n");
        return 1;
    }

    v = atof (s);
    if (v == 0) {
        printf ("line %d: Invalid value\n", line_num);
        return 1;
    }

    has_zbound = 1;
    conf_drill = v;

    return 0;
}

static int set_origin (void)
{
    int r;
    char *s;
    double v;

    r = get_conf_line (number, &s);
    if (r) {
        printf ("line %d: Syntax error, number is expected\n", line_num);
        return 1;
    }

    if (s == NULL) {
        printf ("Unexpected end of file\n");
        return 1;
    }

    v = atof (s);
    if (v == 0) {
        printf ("line %d: Invalid value\n", line_num);
        return 1;
    }

    conf_xorigin = v;

    r = get_conf_line (number, &s);
    if (r) {
        printf ("line %d: Syntax error, number is expected\n", line_num);
        return 1;
    }

    if (s == NULL) {
        printf ("Unexpected end of file\n");
        return 1;
    }

    v = atof (s);
    if (v == 0) {
        printf ("line %d: Invalid value\n", line_num);
        return 1;
    }

    has_origin = 1;
    conf_yorigin = v;

    return 0;
}

static int set_kerning (void)
{
    int i, n;
    char *s;
    int r;

    if (!has_string) {
        printf ("Can't use kerning without defined string\n");
        return 0;
    }

    n = strlen (conf_string);
    for (i = 0; i < n; i++) {
        r = get_conf_line (number, &s);
        if (r) {
            printf ("line %d: Syntax error, number is expected\n", line_num);
            return 1;
        }

        if (s == NULL) {
            printf ("Unexpected end of file\n");
            return 1;
        }

        conf_kern[i] = atof (s);
    }

    has_kerning = 1;
    return 0;
}

int get_conf (char *fname)
{
    int r;
    char *s;
    token_t t;

    fp = fopen (fname, "r");
    if (fp == NULL) {
        perror ("open file");
        return 1;
    }

    do {
        r = get_conf_line (token, &s);
        if (r) {
            printf ("line %d: Syntax error, token is expected\n", line_num);
            break;
        }

        if (s == NULL)  //end-of-file is reached
            break;

        t = get_token (s);
        if (t == Undefined) {
            printf ("Unknown token %s\n", s);
            return 1;
        }

        switch (t) {
            case String:
                r = set_string ();
                break;
            case Kerning:
                r = set_kerning ();
                break;
            case XScale:
                r = set_scale (1);
                break;
            case YScale:
                r = set_scale (0);
                break;
            case Feedrate:
                r = set_feedrate ();
                break;
            case Zbound:
                r = set_zbound ();
                break;
            case Fontname:
                r = set_fontname ();
                break;
            case Origin:
                r = set_origin ();
                break;
            default:
                break;
        }
    } while (!r);
    if (r)
        return 1;
    else {
        conf_ok = 1;
        return 0;
    }
}

