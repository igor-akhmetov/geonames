#pragma once

/* Database of administration codes. */

typedef struct admin_names_t {
    char const *admin1_name;
    char const *admin2_name1;
    char const *admin2_name2;
} admin_names_t;

void load_admin_codes(char const *admin1_filename, char const *admin2_filename);
void get_admin_names(int country_idx, char const *code1, char const *code2, admin_names_t *names);
