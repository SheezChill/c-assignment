#include "cJSON.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  printf("Welcome to the Hotel Management System");
  printf("\n1. Administrator");
  printf("\n2. Hotel Staff");
  printf("\n3. Customer");
  printf("\n4. Guest");
  printf("\n5. Exit");

  int role = Choice("Enter your role", 1, 5);

  char *filename;

  switch (role) {
  case 1:
    filename = "./database/administrators.json";
    break;
  case 2:
    filename = "./database/staffs.json";
    break;
  case 3:
    filename = "./database/users.json";
    break;
  }

  char name[100];
  char password[100];

  printf("\nName: ");
  scanf("%99s", name);

  printf("Password: ");
  scanf("%99s", password);

  char *json_data = ReadJSON(filename);
  if (json_data == NULL) {
    fprintf(stderr, "Failed to read file: %s\n", filename);
    return 1;
  }

  cJSON *json = cJSON_Parse(json_data);

  cJSON *item = NULL;
  cJSON_ArrayForEach(item, json) {
    cJSON *jsonName = cJSON_GetObjectItemCaseSensitive(item, "name");
    cJSON *jsonPassword = cJSON_GetObjectItemCaseSensitive(item, "password");

    if (jsonName != NULL && jsonPassword != NULL) {
      if (strcmp(jsonName->valuestring, name) == 0 &&
          strcmp(jsonPassword->valuestring, password) == 0) {
        printf("Login successful!\n");
      }
    }
  }

  cJSON_Delete(json);
  free(json_data);
  return 0;
}
