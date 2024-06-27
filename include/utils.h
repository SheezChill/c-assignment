#include "../lib/cJSON.h"

#define TableHeader(table, ...)                                                                              \
  ft_set_cell_prop(table, FT_ANY_ROW, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);                      \
  ft_write_ln(table, __VA_ARGS__)

extern int Choice(const char *message, const int min, const int max);

extern char *ReadJSON(const char *filename);
extern void WriteJSON(const char *file_path, const cJSON *json);

extern char *ParseISODateTime(const char *date_string);
extern int IsValidISODate(const char *date_string);
extern int IsValidISODateTime(const char *date_string);

// 0 is false, 1 is true
extern int YesNo(const char *message);

extern char *StringInput(const char *, const int);
extern cJSON *FindKey(const cJSON *json, char *field, const char *value);
extern cJSON *FindValue(const cJSON *json, const char *value);
extern char *Capitalize(char *str);
