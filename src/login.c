#include "../include/administrator.h"
#include "../include/customer.h"
#include "../include/guest.h"
#include "../include/staff.h"
#include "../include/utils.h"
#include "../lib/cJSON.h"
#include <stdio.h>
#include <string.h>

typedef enum { ADMIN, STAFF, CUSTOMER } Role;

char *Login(Role role) {
  char *db_data;
  cJSON *db_json;

  switch (role) {
  case ADMIN:
    db_data = ReadJSON("database/administrators.json");
    db_json = cJSON_Parse(db_data);
    break;
  case STAFF:
    db_data = ReadJSON("database/staffs.json");
    db_json = cJSON_Parse(db_data);
    break;
  case CUSTOMER:
    db_data = ReadJSON("database/customers.json");
    db_json = cJSON_Parse(db_data);
    break;
  }

  char *name = StringInput("Enter name ", 64);
  char *password = StringInput("Enter password ", 64);

  cJSON *item;
  cJSON_ArrayForEach(item, db_json) {
    char *user_name = cJSON_GetObjectItemCaseSensitive(item, "name")->valuestring;
    char *user_password = cJSON_GetObjectItemCaseSensitive(item, "password")->valuestring;
    cJSON *id = cJSON_GetObjectItemCaseSensitive(item, "id");

    if (strcmp(name, user_name) == 0 && strcmp(password, user_password) == 0) {
      if (id != NULL) {
        return id->valuestring;
      } else {
        return "";
      }
    }
  }

  return NULL;
}

void LoginUser(Role role) {
  char *id = Login(role);

  if (id == NULL) {
    printf("\nInvalid credentials!");
  } else {
    switch (role) {
    case ADMIN:
      InitAdministrator();
      break;
    case STAFF:
      InitStaff();
      break;
    case CUSTOMER:
      InitCustomer(id);
      break;
    }
  }
}

void LoginMenu() {
  printf("\n1. Administrator");
  printf("\n2. Staff");
  printf("\n3. Customer");
  printf("\n4. Continue as guest");
  printf("\n5. Exit");

  int choice = Choice("Enter role", 1, 5);

  Role roles[] = {ADMIN, STAFF, CUSTOMER};

  switch (choice) {
  case 4: {
    InitGuest();
    break;
  }
  case 5:
    break;
  default:
    LoginUser(roles[choice - 1]);
    break;
  }
}

int main() {
  LoginMenu();
}
