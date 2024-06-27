#include <stdio.h>
#include <string.h>
#include <time.h>

// Example time string: "2024-06-11T14:30"

int main() {
  const char *iso_date_time = "2024-06-11T14:30";
  struct tm tm;
  char formatted_date[20];

  // Initialize the tm structure
  memset(&tm, 0, sizeof(struct tm));

  // Parse the date-time string using sscanf
  if (sscanf(iso_date_time, "%4d-%2d-%2dT%2d:%2d", &tm.tm_year, &tm.tm_mon,
             &tm.tm_mday, &tm.tm_hour, &tm.tm_min) == 5) {
    // Adjust the year and month to match struct tm expectations
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;

    // Successfully parsed the date-time
    if (strftime(formatted_date, sizeof(formatted_date), "%d %b %Y", &tm)) {
      printf("Formatted date: %s\n", formatted_date);
    } else {
      fprintf(stderr, "Failed to format date\n");
      return 1;
    }
  } else {
    // Failed to parse the date-time
    fprintf(stderr, "Failed to parse date-time\n");
    return 1;
  }

  return 0;
}
