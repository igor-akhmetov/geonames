#include "standard.h"
#include "log.h"
#include "text_db.h"
#include "util.h"

#define MAX_LINE_LEN 16384  /* maximum line length */
#define MAX_FIELDS_NUM 16   /* maximum number of fields in a single line */

typedef struct text_db_impl {
    FILE        *f;                      /* file to read data from */
    char        *filename;               /* filename to report in error messages */
    char        buf[MAX_LINE_LEN];       /* buffer to hold the current text line */
    int         nfields;                 /* number of fields in the current line */
    char        *fields[MAX_FIELDS_NUM]; /* pointers to start of the fields */
} text_db_impl;

text_db_t tdb_open(char const *filename) {
    text_db_t db = xcalloc(sizeof(text_db_impl));
    db->filename = xstrdup(filename);
    if (!(db->f = fopen(filename, "r")))
        cerror("can't open %s", filename);
    return db;
}

int tdb_next_row(text_db_t db) {
    char *line;

    assert(db);
    assert(db->f);

    while ((line = fgets(db->buf, sizeof db->buf, db->f))) {
        char *field = line;
        int first = strspn(line, " \t");

        /* Check if the current line is a comment. */
        if (first == strlen(line) || line[first] == '#')
            continue;

        /* Strip end-of-line characters. */
        strtrim(line);

        /* Iterate over fields of the current line. */
        for (db->nfields = 0; db->nfields < MAX_FIELDS_NUM;) {
            char * next_field = field;
            for (; *next_field && *next_field != '\t'; ++next_field);

            db->fields[db->nfields] = field;
            ++db->nfields;
            if (!*next_field)
                break;

            *next_field = 0;
            field = next_field + 1;
        }

        return 1;
    }

    return 0;
}

int tdb_fields_num(text_db_t db) {
    assert(db);
    return db->nfields;
}

char const * tdb_field(text_db_t db, int idx) {
    assert(idx < tdb_fields_num(db));
    return db->fields[idx];
}

void tdb_close(text_db_t db) {
    assert(db);
    assert(db->f);

    if (ferror(db->f))
        cerror("can't read %s", db->filename);

    if (fclose(db->f) == EOF)
        cerror("can't close %s", db->filename);

    free(db->filename);
    free(db);
}
