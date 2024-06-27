#include "../lib/asprintf.h"
#include "../lib/cJSON.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char *ReadJSON(const char *filename) {
  FILE *file = fopen(filename, "rb");
  if (file == NULL) {
    perror("File opening failed");
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *content = (char *)malloc(length + 1);
  if (content) {
    fread(content, 1, length, file);
    content[length] = '\0';
  }

  fclose(file);

  return content;
}

void WriteJSON(const char *file_path, const cJSON *json) {
  FILE *file = fopen(file_path, "w");
  if (file == NULL) {
    perror("File opening failed");
    return;
  }

  fputs(cJSON_Print(json), file);
  fclose(file);
}

int Choice(char *message, int min, int max) {
  int choice = 0;
  int invalid_integer = 0;

  do {
    printf("\n%s (%d-%d): ", message, min, max);
    invalid_integer = scanf("%d", &choice) != 1 && scanf("%*[^\n]") == 0;

    if (invalid_integer == 1) {
      printf("Input an integer!\n");
      continue;
    }

    if (choice < min || choice > max) {
      printf("Invalid range!\n");
    }
  } while (choice < min || choice > max || invalid_integer == 1);

  return choice;
}

int YesNo(const char *message) {
  char choice_char;
  int is_other_char = 0;
  int choice = 0;

  do {
    printf("\n%s (y/n): ", message);
    scanf(" %c", &choice_char);

    while (getchar() != '\n')
      ;

    choice_char = tolower(choice_char);

    is_other_char = choice_char != 'y' && choice_char != 'n';

    if (is_other_char == 1) {
      printf("Input with 'y' or 'n'!\n");
    }
  } while (is_other_char);

  if (choice_char == 'y') {
    choice = 1;
  }

  return choice;
}

char *ParseISODateTime(const char *date_string) {
  struct tm tm;
  char formatted_date[20];
  char day[3];
  char formatted_time[11];
  char *result = NULL;

  memset(&tm, 0, sizeof(struct tm));
  memset(day, 0, sizeof(day));
  memset(formatted_time, 0, sizeof(formatted_time));

  int items_parsed = sscanf(
      date_string, "%4d-%2d-%2dT%2d:%2d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min);

  if (items_parsed == 5) {
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;

    snprintf(day, sizeof(day), "%d", tm.tm_mday);

    if (strftime(formatted_date, sizeof(formatted_date), "%b %Y", &tm)) {
      // Format time in 12-hour format
      int hour = tm.tm_hour;
      char am_pm[] = "AM";
      if (hour >= 12) {
        am_pm[0] = 'P';
        if (hour > 12) {
          hour -= 12;
        }
      }
      if (hour == 0) {
        hour = 12;
      }
      snprintf(formatted_time, sizeof(formatted_time), "%02d:%02d %s", hour, tm.tm_min, am_pm);

      // Calculate the total length including the time part
      size_t total_length = strlen(day) + strlen(formatted_date) + strlen(formatted_time) + 4;
      result = malloc(total_length);

      if (result != NULL) {
        snprintf(result, total_length, "%s %s %s", day, formatted_date, formatted_time);
      }
    } else {
      fprintf(stderr, "Failed to format date\n");
    }
  } else {
    fprintf(stderr, "Failed to parse date string\n");
  }

  return result;
}

int IsValidISODate(const char *date_string) {
  struct tm tm;
  memset(&tm, 0, sizeof(struct tm));

  int items_parsed = sscanf(date_string, "%4d-%2d-%2d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);

  if (items_parsed == 3) {
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;

    if (tm.tm_year >= 0 && tm.tm_mon >= 0 && tm.tm_mon < 12 && tm.tm_mday > 0 && tm.tm_mday <= 31) {
      time_t t = mktime(&tm);
      return t != -1;
    }
  }

  return 0;
}

int IsValidISODateTime(const char *date_string) {
  struct tm tm;
  memset(&tm, 0, sizeof(struct tm));

  int items_parsed = sscanf(
      date_string, "%4d-%2d-%2dT%2d:%2d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min);

  if (items_parsed == 5) {
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;

    if (tm.tm_year >= 0 && tm.tm_mon >= 0 && tm.tm_mon < 12 && tm.tm_mday > 0 && tm.tm_mday <= 31 &&
        tm.tm_hour >= 0 && tm.tm_hour < 24 && tm.tm_min >= 0 && tm.tm_min < 60) {
      time_t t = mktime(&tm);
      return t != -1;
    }
  }

  return 0;
}

char *StringInput(const char *message, const int strlength) {
  char *string = (char *)malloc((strlength + 2) * sizeof(char));

  while (1) {
    printf("%s (max %d characters): ", message, strlength);

    char *formatString = NULL;
    asprintf(&formatString, "%%%ds", strlength + 1);

    scanf(formatString, string);

    free(formatString);

    while (getchar() != '\n')
      ;

    if (strlen(string) > strlength) {
      printf("\nMax length exceeded!\n\n");
    } else {
      break;
    }
  }

  return string;
}

cJSON *FindKey(const cJSON *json, char *field, const char *value) {
  cJSON *item;
  cJSON *selected_item = NULL;

  cJSON_ArrayForEach(item, json) {
    cJSON *json_field = cJSON_GetObjectItemCaseSensitive(item, field);

    if (strcmp(json_field->valuestring, value) == 0) {
      selected_item = item;
      break;
    }
  }

  return selected_item;
}

cJSON *FindValue(const cJSON *json, const char *value) {
  cJSON *item;

  cJSON_ArrayForEach(item, json) {
    if (strcmp(item->string, value) == 0) {
      return item;
    }
  }

  return NULL;
}

char *Capitalize(char *str) {
  asprintf(&str, "%c%s", toupper(str[0]), str + 1);
  return str;
}
