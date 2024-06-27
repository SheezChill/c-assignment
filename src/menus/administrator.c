#include "../../include/login.h"
#include "../../include/utils.h"
#include "../../lib/asprintf.h"
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

typedef struct {
  char *field_name;
  int max_value_length;
} Field;

typedef struct {
  cJSON *customers_json;
  cJSON *staffs_json;
  cJSON *rooms_json;
  cJSON *selected_user;
  cJSON *item;
  ft_table_t *table;
} AdminContext;

int AdministratorMenu(AdminContext *ctx);

void CreateUser(AdminContext *ctx, const UserType user_type);

void ManageStaffs(AdminContext *ctx);

void ManageCustomers(AdminContext *ctx);

void ManageRooms(AdminContext *ctx);

int AdministratorMenu(AdminContext *ctx) {
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
    CreateUser(ctx, STAFF);
    break;
  case 2:
    CreateUser(ctx, CUSTOMER);
    break;
  case 3:
    ManageStaffs(ctx);
    break;
  case 4:
    ManageCustomers(ctx);
    break;
  case 5:
    ManageRooms(ctx);
    break;
  case 6:
    return 0;
  }

  return 0;
}

void CreateUser(AdminContext *ctx, const UserType user_type) {
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

    cJSON_AddItemToArray(ctx->staffs_json, new_user);
    WriteJSON("database/staffs.json", ctx->staffs_json);
    break;
  }
  case CUSTOMER: {
    char *email = StringInput("Customer email", 64);

    cJSON_AddStringToObject(new_user, "email", email);
    cJSON_AddArrayToObject(new_user, "bookings");
    cJSON_AddArrayToObject(new_user, "reservations");

    free(email);

    cJSON_AddItemToArray(ctx->customers_json, new_user);
    WriteJSON("database/customers.json", ctx->customers_json);
    break;
  }
  }

  AdministratorMenu(ctx);
}

void ChangeUserField(AdminContext *ctx, Field field, UserType user_type) {
  char *buffer;
  asprintf(&buffer, "New %s", field.field_name);

  char *new_value = StringInput(buffer, 64);

  cJSON_SetValuestring(cJSON_GetObjectItemCaseSensitive(ctx->selected_user, field.field_name), new_value);

  free(new_value);
  free(buffer);

  switch (user_type) {
  case STAFF:
    WriteJSON("database/staffs.json", ctx->staffs_json);
    break;
  case CUSTOMER:
    WriteJSON("database/customers.json", ctx->customers_json);
    break;
  }
}

void ManageRooms_Edit(AdminContext *ctx, char *room_ids[]) {
  int rooms_array_size = cJSON_GetArraySize(ctx->rooms_json);

  int selected_room_id_index = Choice("Enter which room you want to edit", 1, rooms_array_size) - 1;
  cJSON *selected_room = FindValue(ctx->rooms_json, room_ids[selected_room_id_index]);

  if (cJSON_GetObjectItemCaseSensitive(selected_room, "booked")->valueint == 1) {
    printf("\nYou can't edit this room! It's being booked!\n");

    ManageRooms(ctx);
    return;
  }

  printf("\n1. Edit room ID");
  printf("\n2. Edit type");
  printf("\n3. Back");

  int choice = Choice("Enter choice", 1, 3);

  switch (choice) {
  case 1: {
    char *new_room_id;

    char *old_room_id = (char *)malloc(strlen(selected_room->string) + 1);
    strcpy(old_room_id, selected_room->string);

    int exists = 0;
    do {
      exists = 0;

      new_room_id = StringInput("Room ID (0 to go back)", 7);

      if (strcmp(new_room_id, "0") == 0) {
        ManageRooms_Edit(ctx, room_ids);
        return;
      }

      cJSON_ArrayForEach(ctx->item, ctx->rooms_json) {
        if (strcmp(ctx->item->string, new_room_id) == 0) {
          printf("This room already exists!\n");

          exists = 1;
          break;
        }
      }
    } while (exists == 1);

    cJSON *room_object = cJSON_DetachItemFromObject(ctx->rooms_json, room_ids[selected_room_id_index]);

    cJSON_AddItemToObject(ctx->rooms_json, new_room_id, room_object);
    cJSON *history_array = cJSON_GetObjectItemCaseSensitive(room_object, "history");

    // Change all instances of the *old* room id to the *new* room id in customers.json
    if (cJSON_GetArraySize(history_array) > 0) {
      char *customer_ids[cJSON_GetArraySize(history_array)];

      int i = 0;
      cJSON_ArrayForEach(ctx->item, history_array) {
        customer_ids[i] = cJSON_GetObjectItemCaseSensitive(ctx->item, "booked_by")->valuestring;
        i++;
      }

      for (i = 0; i < cJSON_GetArraySize(history_array); i++) {
        cJSON *customer_data = FindKey(ctx->customers_json, "id", customer_ids[i]);

        cJSON *bookings = cJSON_GetObjectItemCaseSensitive(customer_data, "bookings");
        cJSON_ArrayForEach(ctx->item, bookings) {
          cJSON *booking_room_id = cJSON_GetObjectItemCaseSensitive(ctx->item, "room_id");

          if (strcmp(booking_room_id->valuestring, old_room_id) == 0) {
            cJSON_SetValuestring(cJSON_GetObjectItemCaseSensitive(ctx->item, "room_id"), new_room_id);
          }
        }
      }

      WriteJSON("database/customers.json", ctx->customers_json);
    }

    cJSON_DeleteItemFromObject(ctx->rooms_json, room_ids[selected_room_id_index]);

    free(new_room_id);
    free(old_room_id);
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
    cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(selected_room, "price"), selected_room_type.price);
    break;
  }
  case 3:
    ManageRooms(ctx);
    return;
  }
}

void ManageRooms_Add(AdminContext *ctx) {
  char *roomID;
  int exists;

  do {
    roomID = StringInput("Room ID (0 to go back)", 7);
    exists = 0;

    if (strcmp(roomID, "0") == 0) {
      ManageRooms(ctx);
      return;
    }

    cJSON_ArrayForEach(ctx->item, ctx->rooms_json) {
      if (strcmp(ctx->item->string, roomID) == 0) {
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

  cJSON_AddItemToObject(ctx->rooms_json, roomID, new_room);

  free(roomID);
}

void ManageRooms(AdminContext *ctx) {
  ft_table_t *table = ctx->table;
  table = ft_create_table();

  TableHeader(table, "Index", "Room ID", "Type", "Price", "Booked", "Booked by");

  int rooms_array_size = cJSON_GetArraySize(ctx->rooms_json);
  char *room_ids[rooms_array_size];

  int i = 0;
  cJSON_ArrayForEach(ctx->item, ctx->rooms_json) {
    cJSON *booked_by = cJSON_GetObjectItemCaseSensitive(ctx->item, "booked_by");
    char *capitalized_type = Capitalize(cJSON_GetObjectItemCaseSensitive(ctx->item, "type")->valuestring);

    room_ids[i] = ctx->item->string;
    i++;

    ft_printf_ln(table,
                 "%d|%s|%s|%d|%s|%s",
                 i,
                 ctx->item->string,
                 capitalized_type,
                 cJSON_GetObjectItemCaseSensitive(ctx->item, "price")->valueint,
                 cJSON_GetObjectItemCaseSensitive(ctx->item, "booked")->valueint ? "Yes" : "No",
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
    ManageRooms_Add(ctx);
    break;
  }
  case 2: {
    ManageRooms_Edit(ctx, room_ids);
    break;
  }
  case 3: {
    choice = Choice("Room to delete", 1, cJSON_GetArraySize(ctx->rooms_json)) - 1;

    int confirmation = YesNo("Are you sure you want to delete this room?");

    if (confirmation == 0) {
      printf("\nRoom deletion cancelled.\n");
      ManageRooms(ctx);
      return;
    }

    cJSON_DeleteItemFromObject(ctx->rooms_json, room_ids[choice]);

    printf("\nRoom deleted.\n");
    break;
  }
  case 4:
    AdministratorMenu(ctx);
    return;
  }

  WriteJSON("database/rooms.json", ctx->rooms_json);

  ManageRooms(ctx);
}

void ManageStaffs_Edit(AdminContext *ctx, const int updated) {
  if (updated) {
    printf("\nUpdated info: \n");
  }

  ft_table_t *table = ctx->table;
  table = ft_create_table();

  char *staff_id = cJSON_GetObjectItemCaseSensitive(ctx->selected_user, "id")->valuestring;
  char *staff_name = cJSON_GetObjectItemCaseSensitive(ctx->selected_user, "name")->valuestring;
  char *staff_password = cJSON_GetObjectItemCaseSensitive(ctx->selected_user, "password")->valuestring;
  char *staff_email = cJSON_GetObjectItemCaseSensitive(ctx->selected_user, "email")->valuestring;
  char *staff_phone = cJSON_GetObjectItemCaseSensitive(ctx->selected_user, "phone")->valuestring;
  char *staff_join_date = cJSON_GetObjectItemCaseSensitive(ctx->selected_user, "joined")->valuestring;

  TableHeader(table, "Id", "Name", "Password", "Email", "Phone", "Join date");
  ft_write_ln(table, staff_id, staff_name, staff_password, staff_email, staff_phone, staff_join_date);

  printf("%s\n", ft_to_string(table));
  ft_destroy_table(table);

  Field fields[] = {
      {"name", 64},
      {"password", 64},
      {"email", 64},
      {"phone", 10},
  };

  for (int i = 0; i < 4; i++) {
    printf("\n%d. Change %s", i + 1, fields[i].field_name);
  }
  printf("\n5. Change join date");
  printf("\n6. Delete staff");
  printf("\n7. Back");

  int choice = Choice("What to change?", 1, 7);

  switch (choice) {
  case 5: {
    char *new_join_date;

    int is_date_string_valid = 0;
    do {
      new_join_date = StringInput("New join date (YYYY-MM-DD)", 10);
      is_date_string_valid = IsValidISODate(new_join_date) == 0;

      if (is_date_string_valid) {
        printf("Not a valid date string!\n");
        continue;
      }

      break;
    } while (is_date_string_valid == 1);

    cJSON_SetValuestring(cJSON_GetObjectItemCaseSensitive(ctx->selected_user, "joined"), new_join_date);

    free(new_join_date);

    WriteJSON("database/staffs.json", ctx->staffs_json);
    break;
  }
  case 6: {
    int choice = YesNo("Are you sure you want to delete this staff?");

    if (choice == 0) {
      printf("\nStaff deletion cancelled.\n");
      ManageStaffs_Edit(ctx, 0);
      return;
    }

    cJSON *detached_item = cJSON_DetachItemViaPointer(ctx->staffs_json, ctx->selected_user);
    cJSON_Delete(detached_item);

    WriteJSON("database/staffs.json", ctx->staffs_json);

    printf("\nStaff deleted.\n");
    ManageStaffs(ctx);
    return;
  }
  case 7:
    ManageStaffs(ctx);
    return;
  default:
    ChangeUserField(ctx, fields[choice - 1], STAFF);
    break;
  }

  ManageStaffs_Edit(ctx, 1);
}

void ManageStaffs(AdminContext *ctx) {
  ft_table_t *table = ctx->table;
  table = ft_create_table();

  TableHeader(table, "No.", "Id", "Name", "Password", "Email", "Phone", "Join date");

  int array_size = cJSON_GetArraySize(ctx->staffs_json);
  char *ids[array_size];

  int i = 0;
  cJSON_ArrayForEach(ctx->item, ctx->staffs_json) {
    char *staff_id = cJSON_GetObjectItemCaseSensitive(ctx->item, "id")->valuestring;
    char *staff_name = cJSON_GetObjectItemCaseSensitive(ctx->item, "name")->valuestring;
    char *staff_password = cJSON_GetObjectItemCaseSensitive(ctx->item, "password")->valuestring;
    char *staff_email = cJSON_GetObjectItemCaseSensitive(ctx->item, "email")->valuestring;
    char *staff_phone = cJSON_GetObjectItemCaseSensitive(ctx->item, "phone")->valuestring;
    char *staff_join_date = cJSON_GetObjectItemCaseSensitive(ctx->item, "joined")->valuestring;

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

  int choice = Choice("Select which staff to manage (0 to go back to main menu)", 0, array_size) - 1;
  if (choice == -1) {
    AdministratorMenu(ctx);
    return;
  }

  ctx->selected_user = FindKey(ctx->staffs_json, "id", ids[choice]);

  ManageStaffs_Edit(ctx, 0);
}

void ManageCustomers_Edit(AdminContext *ctx, const int updated) {
  if (updated) {
    printf("\nUpdated info: \n");
  }

  ft_table_t *table = ctx->table;
  table = ft_create_table();

  char *customer_id = cJSON_GetObjectItemCaseSensitive(ctx->selected_user, "id")->valuestring;
  char *customer_name = cJSON_GetObjectItemCaseSensitive(ctx->selected_user, "name")->valuestring;
  char *customer_password = cJSON_GetObjectItemCaseSensitive(ctx->selected_user, "password")->valuestring;
  char *customer_email = cJSON_GetObjectItemCaseSensitive(ctx->selected_user, "email")->valuestring;

  TableHeader(table, "Id", "Name", "Password", "Email");
  ft_write_ln(table, customer_id, customer_name, customer_password, customer_email);

  printf("%s\n", ft_to_string(table));
  ft_destroy_table(table);

  Field fields[] = {{"name", 64}, {"password", 64}, {"email", 64}};

  for (int i = 0; i < 3; i++) {
    printf("\n%d. Change %s", i + 1, fields[i].field_name);
  }
  printf("\n4. Delete customer");
  printf("\n5. Back");

  int choice = Choice("What to change?", 1, 5);

  switch (choice) {
  case 4: {
    int choice = YesNo("Are you sure you want to delete this customer?");

    if (choice == 0) {
      printf("\nCustomer deletion cancelled.\n");
      ManageCustomers_Edit(ctx, 0);
      return;
    }

    cJSON *detached_item = cJSON_DetachItemViaPointer(ctx->customers_json, ctx->selected_user);
    cJSON_Delete(detached_item);

    WriteJSON("database/customers.json", ctx->customers_json);

    printf("\nCustomer deleted.\n");
    ManageCustomers(ctx);
    return;
  }
  case 5:
    ManageCustomers(ctx);
    return;
  default:
    ChangeUserField(ctx, fields[choice - 1], CUSTOMER);
    break;
  }

  ManageCustomers_Edit(ctx, 1);
}

void ManageCustomers(AdminContext *ctx) {
  ft_table_t *table = ctx->table;
  table = ft_create_table();

  TableHeader(table, "No.", "Id", "Name", "Password", "Email");

  int array_size = cJSON_GetArraySize(ctx->customers_json);
  char *ids[array_size];

  int i = 0;
  cJSON_ArrayForEach(ctx->item, ctx->customers_json) {
    char *customer_id = cJSON_GetObjectItemCaseSensitive(ctx->item, "id")->valuestring;
    char *customer_name = cJSON_GetObjectItemCaseSensitive(ctx->item, "name")->valuestring;
    char *customer_password = cJSON_GetObjectItemCaseSensitive(ctx->item, "password")->valuestring;
    char *customer_email = cJSON_GetObjectItemCaseSensitive(ctx->item, "email")->valuestring;

    ids[i] = customer_id;
    i++;

    ft_printf_ln(table, "%d|%s|%s|%s|%s", i, customer_id, customer_name, customer_password, customer_email);
    ft_add_separator(table);
  }

  printf("%s\n", ft_to_string(table));
  ft_destroy_table(table);

  // Ask for which customer to manage
  int choice = Choice("Select which customer to manage (0 to go back to main menu)", 0, array_size) - 1;
  if (choice == -1) {
    AdministratorMenu(ctx);
    return;
  }

  ctx->selected_user = FindKey(ctx->customers_json, "id", ids[choice]);

  ManageCustomers_Edit(ctx, 0);
}

void InitAdministrator() {
  ft_set_default_border_style(FT_SOLID_ROUND_STYLE);

  AdminContext ctx;

  char *customers_json_data = ReadJSON("database/customers.json");
  ctx.customers_json = cJSON_Parse(customers_json_data);
  free(customers_json_data);

  char *staffs_json_data = ReadJSON("database/staffs.json");
  ctx.staffs_json = cJSON_Parse(staffs_json_data);
  free(staffs_json_data);

  char *rooms_json_data = ReadJSON("database/rooms.json");
  ctx.rooms_json = cJSON_Parse(rooms_json_data);
  free(rooms_json_data);

  AdministratorMenu(&ctx);

  cJSON_Delete(ctx.customers_json);
  cJSON_Delete(ctx.staffs_json);
  cJSON_Delete(ctx.rooms_json);

  LoginMenu();
}
