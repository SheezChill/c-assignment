#include "../../include/login.h"
#include "../../include/utils.h"
#include "../../lib/cJSON.h"
#include "../../lib/fort.h"
#include "../../lib/nanoid.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { STAFF, CUSTOMER } UserType;

typedef struct {
  char *type;
  int price;
} RoomType;

// #region global variable definitions
cJSON *selected_user;

char *customers_json_data;
cJSON *customers_json;

char *staffs_json_data;
cJSON *staffs_json;

char *rooms_json_data;
cJSON *rooms_json;

cJSON *item;

ft_table_t *table;
// #endregion

// #region function definitions
int AdministratorMenu();

void CreateUser(const UserType);

void ManageStaffs();
void ManageStaffs_Ask(const int);

void ManageCustomers();
void ManageCustomers_Ask(const int);

void ManageRooms();
// #endregion

int AdministratorMenu() {
  printf("\n--- Administrator Menu ---");
  printf("\n1. Register new staff");
  printf("\n2. Register new customer");
  printf("\n3. Manage staff");
  printf("\n4. Manage customer");
  printf("\n5. Manage hotel rooms");
  printf("\n6. Logout");

  int choice = Choice("Enter choice", 1, 6);

  switch (choice) {
  case 1:
    CreateUser(STAFF);
    break;
  case 2:
    CreateUser(CUSTOMER);
    break;
  case 3:
    ManageStaffs();
    break;
  case 4:
    ManageCustomers();
    break;
  case 5:
    ManageRooms();
    break;
  case 6:
    LoginMenu();
    return 0;
  }

  return 0;
}

void CreateUser(const UserType user_type) {
  char *user_name = StringInput("User name", 64);
  char *user_password = StringInput("User password", 64);

  cJSON *new_user = cJSON_CreateObject();

  cJSON_AddStringToObject(new_user, "id", safe_generate(5));
  cJSON_AddStringToObject(new_user, "name", user_name);
  cJSON_AddStringToObject(new_user, "password", user_password);

  free(user_name);
  free(user_password);

  switch (user_type) {
  case STAFF: {
    char *email = StringInput("Staff email", 64);
    char *phone = StringInput("Staff phone number", 10);
    char *join_date;
    int is_date_string_valid;

    do {
      join_date = StringInput("Staff join date (YYYY-MM-DD)", 10);
      is_date_string_valid = IsValidISODate(join_date) == 0;

      if (is_date_string_valid) {
        printf("Not a valid date string!\n");
        continue;
      }

      break;
    } while (is_date_string_valid);

    cJSON_AddStringToObject(new_user, "email", email);
    cJSON_AddStringToObject(new_user, "phone", phone);
    cJSON_AddStringToObject(new_user, "joined", join_date);

    free(email);
    free(phone);
    free(join_date);

    cJSON_AddItemToArray(staffs_json, new_user);
    WriteJSON("database/staffs.json", staffs_json);
    break;
  }
  case CUSTOMER: {
    char *email = StringInput("Customer email", 64);

    cJSON_AddStringToObject(new_user, "email", email);
    cJSON_AddArrayToObject(new_user, "bookings");
    cJSON_AddArrayToObject(new_user, "reservations");

    free(email);

    cJSON_AddItemToArray(customers_json, new_user);
    WriteJSON("database/customers.json", customers_json);
    break;
  }
  }

  AdministratorMenu();
}

void ManageRooms() {
  table = ft_create_table();

  TableHeader(table, "Index", "Room ID", "Type", "Price", "Booked", "Booked by");

  int rooms_array_size = cJSON_GetArraySize(rooms_json);

  char *room_ids[rooms_array_size];

  int i = 0;
  cJSON_ArrayForEach(item, rooms_json) {
    cJSON *booked_by = cJSON_GetObjectItemCaseSensitive(item, "booked_by");
    char *capitalized_type = Capitalize(cJSON_GetObjectItemCaseSensitive(item, "type")->valuestring);
    room_ids[i] = item->string;
    i++;

    ft_printf_ln(table,
                 "%d|%s|%s|%d|%s|%s",
                 i,
                 item->string,
                 capitalized_type,
                 cJSON_GetObjectItemCaseSensitive(item, "price")->valueint,
                 cJSON_GetObjectItemCaseSensitive(item, "booked")->valueint ? "Yes" : "No",
                 cJSON_IsNull(booked_by) ? "No one" : booked_by->valuestring);

    free(capitalized_type);
  }

  printf("\n%s\n", ft_to_string(table));
  ft_destroy_table(table);

  printf("\n1. Add room");
  printf("\n2. Edit room");
  printf("\n3. Delete room");
  printf("\n4. Back");

  int choice = Choice("Enter choice", 1, 4);

  switch (choice) {
  case 1: {
    char *roomID;
    int exists;

    do {
      roomID = StringInput("Room ID (0 to go back)", 7);
      exists = 0;

      if (strcmp(roomID, "0") == 0) {
        ManageRooms();
        return;
      }

      cJSON_ArrayForEach(item, rooms_json) {
        if (strcmp(item->string, roomID) == 0) {
          printf("This room already exists!\n");
          exists = 1;
          break;
        }
      }
    } while (exists == 1);

    RoomType room_types[4] = {
        {"suite", 250},
        {"deluxe", 200},
        {"double", 150},
        {"single", 100},
    };

    printf("\nWhat is the room type? ");
    for (int i = 0; i < 4; i++) {
      char *capitalized_room_type = Capitalize(room_types[i].type);

      printf("\n%d. %s", i + 1, capitalized_room_type);

      free(capitalized_room_type);
    }

    RoomType selected_room_type = room_types[Choice("Enter choice: ", 1, 4) - 1];

    cJSON *new_room = cJSON_CreateObject();
    cJSON_AddBoolToObject(new_room, "booked", 0);
    cJSON_AddNullToObject(new_room, "booked_by");
    cJSON_AddNumberToObject(new_room, "price", selected_room_type.price);
    cJSON_AddStringToObject(new_room, "type", selected_room_type.type);
    cJSON_AddArrayToObject(new_room, "history");

    cJSON_AddItemToObject(rooms_json, roomID, new_room);
    free(roomID);
    break;
  }
  case 2: {
    int selected_room_id_index = Choice("Enter which room you want to edit", 1, rooms_array_size) - 1;

    cJSON *selected_room = FindValue(rooms_json, room_ids[selected_room_id_index]);
    char *old_room_id = (char *)malloc(strlen(selected_room->string) + 1);

    strcpy(old_room_id, selected_room->string);

    if (cJSON_GetObjectItemCaseSensitive(selected_room, "booked")->valueint == 1) {
      printf("\nYou can't edit this room! It's being booked!\n");
      ManageRooms();
      return;
    }

    printf("\n1. Edit room ID");
    printf("\n2. Edit type");

    int choice = Choice("Enter choice", 1, 2);

    switch (choice) {
    case 1: {
      char *new_room_id;
      int exists;
      do {
        new_room_id = StringInput("Room ID (0 to go back)", 7);
        exists = 0;

        if (strcmp(new_room_id, "0") == 0) {
          ManageRooms();
          return;
        }

        cJSON_ArrayForEach(item, rooms_json) {
          if (strcmp(item->string, new_room_id) == 0) {
            printf("This room already exists!\n");
            exists = 1;
            break;
          }
        }
      } while (exists == 1);

      cJSON *old_children = cJSON_DetachItemFromObject(rooms_json, room_ids[selected_room_id_index]);

      cJSON_AddItemToObject(rooms_json, new_room_id, old_children);
      cJSON *history_array = cJSON_GetObjectItemCaseSensitive(old_children, "history");

      if (cJSON_GetArraySize(history_array) > 0) {
        char *customer_ids[cJSON_GetArraySize(history_array)];
        int i = 0;
        cJSON_ArrayForEach(item, history_array) {
          customer_ids[i] = cJSON_GetObjectItemCaseSensitive(item, "booked_by")->valuestring;
          i++;
        }

        for (i = 0; i < cJSON_GetArraySize(history_array); i++) {
          cJSON *customer_data = FindKey(customers_json, "id", customer_ids[i]);

          cJSON *yes = cJSON_GetObjectItemCaseSensitive(customer_data, "bookings");
          cJSON *booking_containing_room_id = FindKey(yes, "room_id", selected_room->string);

          cJSON_ArrayForEach(item, yes) {
            cJSON *room_id_item = cJSON_GetObjectItemCaseSensitive(item, "room_id");
            if (room_id_item == NULL) {
              fprintf(stderr, "Error: room_id not found in item\n");
              continue;
            }

            if (strcmp(room_id_item->valuestring, old_room_id) == 0) {
              cJSON_SetValuestring(cJSON_GetObjectItemCaseSensitive(item, "room_id"), new_room_id);
            }
          }
        }

        free(old_room_id);

        WriteJSON("database/customers.json", customers_json);
      }

      cJSON_DeleteItemFromObject(rooms_json, room_ids[selected_room_id_index]);

      free(new_room_id);
      break;
    }
    case 2: {
      RoomType room_types[4] = {
          {"suite", 250},
          {"deluxe", 200},
          {"double", 150},
          {"single", 100},
      };

      printf("\nWhat is the room type? ");
      for (int i = 0; i < 4; i++) {
        char *capitalized_room_type = Capitalize(room_types[i].type);

        printf("\n%d. %s", i + 1, capitalized_room_type);

        free(capitalized_room_type);
      }

      RoomType selected_room_type = room_types[Choice("Enter choice: ", 1, 4) - 1];

      cJSON_SetValuestring(cJSON_GetObjectItemCaseSensitive(selected_room, "type"), selected_room_type.type);
      cJSON_SetNumberValue(
          cJSON_GetObjectItemCaseSensitive(selected_room, "price"), selected_room_type.price);
      break;
    }
    }
    break;
  }
  case 3: {
    printf("\nWhich room do you want to delete? ");
    choice = Choice("Enter choice", 1, cJSON_GetArraySize(rooms_json)) - 1;

    cJSON_DeleteItemFromObject(rooms_json, room_ids[choice]);
    break;
  }
  case 4:
    AdministratorMenu();
    return;
  }

  WriteJSON("database/rooms.json", rooms_json);
  ManageRooms();
}

void ManageStaffs_Ask(const int updated) {
  if (updated) {
    printf("\nUpdated info: \n");
  }

  table = ft_create_table();

  char *staff_id = cJSON_GetObjectItemCaseSensitive(selected_user, "id")->valuestring;
  char *staff_name = cJSON_GetObjectItemCaseSensitive(selected_user, "name")->valuestring;
  char *staff_password = cJSON_GetObjectItemCaseSensitive(selected_user, "password")->valuestring;
  char *staff_email = cJSON_GetObjectItemCaseSensitive(selected_user, "email")->valuestring;
  char *staff_phone = cJSON_GetObjectItemCaseSensitive(selected_user, "phone")->valuestring;
  char *staff_join_date = cJSON_GetObjectItemCaseSensitive(selected_user, "joined")->valuestring;

  TableHeader(table, "Id", "Name", "Password", "Email", "Phone", "Join date");
  ft_write_ln(table, staff_id, staff_name, staff_password, staff_email, staff_phone, staff_join_date);

  printf("%s\n", ft_to_string(table));
  ft_destroy_table(table);

  printf("\n1. Change name");
  printf("\n2. Change password");
  printf("\n3. Change email");
  printf("\n4. Change phone");
  printf("\n5. Change join date");
  printf("\n6. Delete staff");
  printf("\n7. Back");

  int choice = Choice("What to change?", 1, 7);

  switch (choice) {
  case 1: {
    char *new_name = StringInput("New name", 64);

    cJSON_SetValuestring(cJSON_GetObjectItemCaseSensitive(selected_user, "name"), new_name);

    free(new_name);

    WriteJSON("database/staffs.json", staffs_json);
    break;
  }
  case 2: {
    char *new_password = StringInput("New password", 10);

    cJSON_SetValuestring(cJSON_GetObjectItemCaseSensitive(selected_user, "password"), new_password);

    free(new_password);

    WriteJSON("database/staffs.json", staffs_json);
    break;
  }
  case 3: {
    char *new_email = StringInput("New email", 10);

    cJSON_SetValuestring(cJSON_GetObjectItemCaseSensitive(selected_user, "email"), new_email);

    free(new_email);

    WriteJSON("database/staffs.json", staffs_json);
    break;
  }
  case 4: {
    char *new_phone = StringInput("New phone", 10);

    cJSON_SetValuestring(cJSON_GetObjectItemCaseSensitive(selected_user, "phone"), new_phone);

    free(new_phone);

    WriteJSON("database/staffs.json", staffs_json);
    break;
  }
  case 5: {
    char *new_join_date;
    int is_date_string_valid;

    do {
      new_join_date = StringInput("New join date (YYYY-MM-DD)", 10);
      is_date_string_valid = IsValidISODate(new_join_date) == 0;

      if (is_date_string_valid) {
        printf("Not a valid date string!\n");
        continue;
      }

      break;
    } while (is_date_string_valid);

    cJSON_SetValuestring(cJSON_GetObjectItemCaseSensitive(selected_user, "joined"), new_join_date);

    free(new_join_date);

    WriteJSON("database/staffs.json", staffs_json);
    break;
  }
  case 6: {
    int choice = YesNo("Are you sure you want to delete this staff?");

    if (choice == 0) {
      printf("\nStaff deletion cancelled.\n");
      ManageStaffs_Ask(0);
      return;
    }

    cJSON *detached_item = cJSON_DetachItemViaPointer(staffs_json, selected_user);
    cJSON_Delete(detached_item);

    WriteJSON("database/staffs.json", staffs_json);
    printf("\nStaff deleted.\n");
    ManageStaffs();
    return;
  }
  case 7:
    ManageStaffs();
    return;
  }

  ManageStaffs_Ask(1);
}

void ManageStaffs() {
  // Show users in table format
  table = ft_create_table();

  int array_size = cJSON_GetArraySize(staffs_json);
  char *ids[array_size];

  int i = 0;
  TableHeader(table, "No.", "Id", "Name", "Password", "Email", "Phone", "Join date");

  cJSON_ArrayForEach(item, staffs_json) {
    char *staff_id = cJSON_GetObjectItemCaseSensitive(item, "id")->valuestring;
    char *staff_name = cJSON_GetObjectItemCaseSensitive(item, "name")->valuestring;
    char *staff_password = cJSON_GetObjectItemCaseSensitive(item, "password")->valuestring;
    char *staff_email = cJSON_GetObjectItemCaseSensitive(item, "email")->valuestring;
    char *staff_phone = cJSON_GetObjectItemCaseSensitive(item, "phone")->valuestring;
    char *staff_join_date = cJSON_GetObjectItemCaseSensitive(item, "joined")->valuestring;

    ids[i] = staff_id;
    i++;

    ft_printf_ln(table,
                 "%d|%s|%s|%s|%s|%s|%s",
                 i,
                 staff_id,
                 staff_name,
                 staff_password,
                 staff_email,
                 staff_phone,
                 staff_join_date);
    ft_add_separator(table);
  }

  printf("%s\n", ft_to_string(table));
  ft_destroy_table(table);

  // Ask for which staff to manage
  int choice = Choice("Select which staff to manage (0 to go back to main menu)", 0, array_size) - 1;
  if (choice == -1) {
    AdministratorMenu();
    return;
  }

  selected_user = FindKey(staffs_json, "id", ids[choice]);

  ManageStaffs_Ask(0);
}

void ManageCustomers_Ask(const int updated) {
  if (updated) {
    printf("\nUpdated info: \n");
  }

  table = ft_create_table();

  char *customer_id = cJSON_GetObjectItemCaseSensitive(selected_user, "id")->valuestring;
  char *customer_name = cJSON_GetObjectItemCaseSensitive(selected_user, "name")->valuestring;
  char *customer_password = cJSON_GetObjectItemCaseSensitive(selected_user, "password")->valuestring;
  char *customer_email = cJSON_GetObjectItemCaseSensitive(selected_user, "email")->valuestring;

  TableHeader(table, "Id", "Name", "Password", "Email");
  ft_write(table, customer_id, customer_name, customer_password, customer_email);

  printf("%s\n", ft_to_string(table));
  ft_destroy_table(table);

  printf("\n1. Change name");
  printf("\n2. Change password");
  printf("\n3. Change email");
  printf("\n4. Delete customer");
  printf("\n5. Back");

  int choice = Choice("What to change?", 1, 5);

  switch (choice) {
  case 1: {
    char *new_name = StringInput("New name", 64);

    cJSON_SetValuestring(cJSON_GetObjectItemCaseSensitive(selected_user, "name"), new_name);

    free(new_name);

    WriteJSON("database/customers.json", customers_json);
    break;
  }
  case 2: {
    char *new_password = StringInput("New password", 10);

    cJSON_SetValuestring(cJSON_GetObjectItemCaseSensitive(selected_user, "password"), new_password);

    free(new_password);

    WriteJSON("database/customers.json", customers_json);
    break;
  }
  case 3: {
    char *new_email = StringInput("New email", 10);

    cJSON_SetValuestring(cJSON_GetObjectItemCaseSensitive(selected_user, "email"), new_email);

    free(new_email);

    WriteJSON("database/customers.json", customers_json);
    break;
  }
  case 4: {
    int choice = YesNo("Are you sure you want to delete this customer?");

    if (choice == 0) {
      printf("\nCustomer deletion cancelled.\n");
      ManageCustomers_Ask(0);
      return;
    }

    cJSON *detached_item = cJSON_DetachItemViaPointer(customers_json, selected_user);
    cJSON_Delete(detached_item);

    WriteJSON("database/customers.json", customers_json);
    printf("\nCustomer deleted.\n");
    ManageCustomers();
    return;
  }
  case 5:
    cJSON_Delete(customers_json);
    free(customers_json_data);

    cJSON_Delete(staffs_json);
    free(staffs_json_data);

    cJSON_Delete(rooms_json);
    free(rooms_json_data);

    ManageCustomers();
    return;
  }

  ManageCustomers_Ask(1);
}

void ManageCustomers() {
  table = ft_create_table();

  int array_size = cJSON_GetArraySize(customers_json);
  char *ids[array_size];

  int i = 0;
  TableHeader(table, "No.", "Id", "Name", "Password", "Email");

  cJSON_ArrayForEach(item, customers_json) {
    char *customer_id = cJSON_GetObjectItemCaseSensitive(item, "id")->valuestring;
    char *customer_name = cJSON_GetObjectItemCaseSensitive(item, "name")->valuestring;
    char *customer_password = cJSON_GetObjectItemCaseSensitive(item, "password")->valuestring;
    char *customer_email = cJSON_GetObjectItemCaseSensitive(item, "email")->valuestring;

    ids[i] = customer_id;
    i++;

    ft_printf_ln(table, "%d|%s|%s|%s|%s", i, customer_id, customer_name, customer_password, customer_email);
    ft_add_separator(table);
  }

  printf("%s\n", ft_to_string(table));
  ft_destroy_table(table);

  // Ask for which staff to manage
  int choice = Choice("Select which customer to manage (0 to go back to main menu)", 0, array_size) - 1;
  if (choice == -1) {
    AdministratorMenu();
    return;
  }

  selected_user = FindKey(customers_json, "id", ids[choice]);

  ManageCustomers_Ask(0);

  cJSON_Delete(selected_user);
}

void InitAdministrator() {
  ft_set_default_border_style(FT_SOLID_ROUND_STYLE);

  customers_json_data = ReadJSON("database/customers.json");
  customers_json = cJSON_Parse(customers_json_data);

  staffs_json_data = ReadJSON("database/staffs.json");
  staffs_json = cJSON_Parse(staffs_json_data);

  rooms_json_data = ReadJSON("database/rooms.json");
  rooms_json = cJSON_Parse(rooms_json_data);

  AdministratorMenu();
}
