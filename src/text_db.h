#pragma once

struct text_db_impl;
typedef struct text_db_impl *text_db_t;

text_db_t       tdb_open(char const *filename);
int             tdb_next_row(text_db_t db);
int             tdb_fields_num(text_db_t db);
char const *    tdb_field(text_db_t db, int idx);
void            tdb_close(text_db_t);
