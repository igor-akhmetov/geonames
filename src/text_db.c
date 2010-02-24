#include "standard.h"
#include "log.h"
#include "text_db.h"
#include "util.h"

#define MAX_LINE_LEN 16384
#define MAX_FIELDS_NUM 16

typedef struct text_db_impl {
    FILE        *f;
    char        *filename;
    char        buf[MAX_LINE_LEN];
    int         nfields;
    char        *fields[MAX_FIELDS_NUM];
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
        
        if (first == strlen(line) || line[first] == '#')
            continue;        

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

void tdb_close(text_db_t db)
{
    assert(db);
    assert(db->f);
    
    if (ferror(db->f))
        cerror("can't read %s", db->filename);

    if (fclose(db->f) == EOF)
        cerror("can't close %s", db->filename);

    free(db->filename);
    free(db);
}
