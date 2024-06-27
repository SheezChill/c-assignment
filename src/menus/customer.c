#include "../../include/login.h"
#include "../../include/utils.h"
#include "../../lib/cJSON.h"
#include "../../lib/fort.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  ft_table_t *table;
  cJSON *item;
  char *customers_json_data;
  cJSON *customers_json;
  char *rooms_json_data;
  cJSON *rooms_json;
  char *customer_id;
  cJSON *current_customer_json;
} CustomerContext;

void CustomerMenu(CustomerContext *ctx);

void MakeReservation(CustomerContext *ctx);

void CancelReservation(CustomerContext *ctx);

void ViewCurrentBookings(CustomerContext *ctx);

void ViewBookingHistory(CustomerContext *ctx);

void MakeReservation(CustomerContext *ctx) {
  int array_size = cJSON_GetArraySize(ctx->rooms_json);

  ctx->table = ft_create_table();

  TableHeader(ctx->table, "No", "Room ID", "Price", "Type");

  // TODO: replace w/ a dynamic array
  char *ids[array_size];

  int i = 0;
  cJSON_ArrayForEach(ctx->item, ctx->rooms_json) {
    if (cJSON_GetObjectItemCaseSensitive(ctx->item, "booked")->valueint == 0) {
      ids[i] = ctx->item->string;
      i++;

      int price = cJSON_GetObjectItemCaseSensitive(ctx->item, "price")->valueint;
      char *capitalized_type = Capitalize(cJSON_GetObjectItemCaseSensitive(ctx->item, "type")->valuestring);

      ft_printf_ln(ctx->table, "%d|%s|%d|%s", i, ctx->item->string, price, capitalized_type);

      free(capitalized_type);
    }
  }

  if (i == 0) {
    printf("\nNo rooms available, please come back soon.\n");
    CustomerMenu(ctx);
    return;
  }

  printf("%s\n", ft_to_string(ctx->table));
  ft_destroy_table(ctx->table);

  printf("\n1. Reserve a room");
  printf("\n2. Back");

  int choice = Choice("Enter choice", 1, 2);

  switch (choice) {
  case 1: {
    int selected_room_index = Choice("Enter which room you want to reserve", 1, i) - 1;

    char *booking_start;

    int is_date_string_valid = 0;
    do {
      is_date_string_valid = 0;

      booking_start = StringInput("Enter booking start date and time (YYYY-MM-DDTHH:mm)", 16);
      is_date_string_valid = IsValidISODateTime(booking_start) == 0;

      if (is_date_string_valid) {
        printf("Not a valid date string!\n");
        continue;
      }

      break;
    } while (is_date_string_valid == 0);

    char *booking_end;

    is_date_string_valid = 0;
    do {
      is_date_string_valid = 0;

      booking_end = StringInput("Enter booking end date and time (YYYY-MM-DDTHH:mm)", 16);
      is_date_string_valid = IsValidISODateTime(booking_end) == 0;

      if (is_date_string_valid) {
        printf("Not a valid date string!\n");
        continue;
      }

      break;
    } while (is_date_string_valid == 0);

    cJSON *selected_room = FindValue(ctx->rooms_json, ids[selected_room_index]);

    cJSON *booked_field = cJSON_GetObjectItemCaseSensitive(selected_room, "booked");
    cJSON_SetBoolValue(booked_field, 1);

    cJSON_ReplaceItemInObjectCaseSensitive(selected_room, "booked_by", cJSON_CreateString(ctx->customer_id));

    cJSON *new_booking = cJSON_CreateObject();
    cJSON_AddStringToObject(new_booking, "room_id", ids[selected_room_index]);
    cJSON_AddStringToObject(new_booking, "booking_start", booking_start);
    cJSON_AddStringToObject(new_booking, "booking_end", booking_end);
    cJSON_AddBoolToObject(new_booking, "checked_in", 0);
    cJSON_AddBoolToObject(new_booking, "checked_out", 0);

    cJSON *bookings_array = cJSON_GetObjectItemCaseSensitive(ctx->current_customer_json, "bookings");
    cJSON_AddItemToArray(bookings_array, new_booking);

    WriteJSON("database/rooms.json", ctx->rooms_json);
    WriteJSON("database/customers.json", ctx->customers_json);

    int another = YesNo("Make another reservation?");

    if (another == 1)
      MakeReservation(ctx);
    break;
  }
  case 2:
    break;
  }

  CustomerMenu(ctx);
}

void CancelReservation(CustomerContext *ctx) {
  ctx->table = ft_create_table();

  TableHeader(ctx->table, "No", "Room ID", "Price", "Type");

  cJSON *customer_bookings = cJSON_GetObjectItemCaseSensitive(ctx->current_customer_json, "bookings");

  int booking_indices[cJSON_GetArraySize(customer_bookings)];

  int i = 0;
  int j = 0;
  cJSON_ArrayForEach(ctx->item, customer_bookings) {
    int checked_out = cJSON_GetObjectItemCaseSensitive(ctx->item, "checked_out")->valueint;

    if (checked_out == 0) {
      booking_indices[j] = i;

      char *room_id = cJSON_GetObjectItemCaseSensitive(ctx->item, "room_id")->valuestring;
      cJSON *room_data = FindValue(ctx->rooms_json, room_id);

      int price = cJSON_GetObjectItemCaseSensitive(room_data, "price")->valueint;
      char *capitalized_type = Capitalize(cJSON_GetObjectItemCaseSensitive(room_data, "type")->valuestring);

      ft_printf_ln(ctx->table, "%d|%s|%d|%s", j + 1, room_id, price, capitalized_type);
      free(capitalized_type);

      j++;
    }

    i++;
  }

  if (j == 0) {
    printf("\nYou have no reservation!\n");

    CustomerMenu(ctx);
    return;
  }

  printf("%s\n", ft_to_string(ctx->table));
  ft_destroy_table(ctx->table);

  int choice = Choice("Enter which booking to cancel (0 to go back)", 0, j) - 1;
  int selected_booking_index = booking_indices[choice];

  if (choice == -1) {
    CustomerMenu(ctx);
    return;
  }

  cJSON *selected_booking = cJSON_GetArrayItem(customer_bookings, selected_booking_index);

  cJSON *selected_booking_room =
      FindValue(ctx->rooms_json, cJSON_GetObjectItemCaseSensitive(selected_booking, "room_id")->valuestring);

  cJSON *selected_booking_room_booked_status =
      cJSON_GetObjectItemCaseSensitive(selected_booking_room, "booked");
  cJSON_SetBoolValue(selected_booking_room_booked_status, 0);
  selected_booking_room_booked_status->valueint = 0;

  cJSON_ReplaceItemInObjectCaseSensitive(selected_booking_room, "booked_by", cJSON_CreateNull());

  cJSON_DeleteItemFromArray(customer_bookings, selected_booking_index);

  WriteJSON("database/rooms.json", ctx->rooms_json);
  WriteJSON("database/customers.json", ctx->customers_json);
  printf("\nReservation cancelled!\n");

  CustomerMenu(ctx);
}

void ViewCurrentBookings(CustomerContext *ctx) {
  ctx->table = ft_create_table();

  TableHeader(ctx->table, "No", "Room ID", "Price", "Type");

  cJSON *customer_bookings = cJSON_GetObjectItemCaseSensitive(ctx->current_customer_json, "bookings");

  int i = 1;
  cJSON_ArrayForEach(ctx->item, customer_bookings) {
    int checked_out = cJSON_GetObjectItemCaseSensitive(ctx->item, "checked_out")->valueint;

    if (checked_out == 0) {
      char *room_id = cJSON_GetObjectItemCaseSensitive(ctx->item, "room_id")->valuestring;
      cJSON *room_data = FindValue(ctx->rooms_json, room_id);

      int price = cJSON_GetObjectItemCaseSensitive(room_data, "price")->valueint;
      char *capitalized_type = Capitalize(cJSON_GetObjectItemCaseSensitive(room_data, "type")->valuestring);

      ft_printf_ln(ctx->table, "%d|%s|%d|%s", i, room_id, price, capitalized_type);
      free(capitalized_type);
      i++;
    }
  }

  if (i == 1) {
    printf("\nYou have no booking!\n");

    ft_destroy_table(ctx->table);

    CustomerMenu(ctx);
    return;
  }

  printf("%s\n", ft_to_string(ctx->table));
  ft_destroy_table(ctx->table);

  CustomerMenu(ctx);
}

void ViewBookingHistory(CustomerContext *ctx) {
  ctx->table = ft_create_table();

  TableHeader(ctx->table, "No", "Room ID", "Price", "Type");

  cJSON *customer_bookings = cJSON_GetObjectItemCaseSensitive(ctx->current_customer_json, "bookings");

  int i = 1;
  cJSON_ArrayForEach(ctx->item, customer_bookings) {
    int checked_out = cJSON_GetObjectItemCaseSensitive(ctx->item, "checked_out")->valueint;

    if (checked_out == 1) {
      char *room_id = cJSON_GetObjectItemCaseSensitive(ctx->item, "room_id")->valuestring;
      cJSON *room_data = FindValue(ctx->rooms_json, room_id);

      int price = cJSON_GetObjectItemCaseSensitive(room_data, "price")->valueint;
      char *capitalized_type = Capitalize(cJSON_GetObjectItemCaseSensitive(room_data, "type")->valuestring);

      ft_printf_ln(ctx->table, "%d|%s|%d|%s", i, room_id, price, capitalized_type);
      free(capitalized_type);

      i++;
    }
  }

  if (i == 1) {
    printf("\nYou have no booking history!\n");

    ft_destroy_table(ctx->table);

    CustomerMenu(ctx);
    return;
  }

  printf("%s\n", ft_to_string(ctx->table));
  ft_destroy_table(ctx->table);

  CustomerMenu(ctx);
}

void CustomerMenu(CustomerContext *ctx) {
  printf("\n--- Customer Menu ---");
  printf("\n1. View rooms and make a reservation");
  printf("\n2. Cancel reservation");
  printf("\n3. View current bookings");
  printf("\n4. View booking history");
  printf("\n5. Logout");

  int choice = Choice("Enter choice ", 1, 5);

  switch (choice) {
  case 1:
    MakeReservation(ctx);
    break;
  case 2:
    CancelReservation(ctx);
    break;
  case 3:
    ViewCurrentBookings(ctx);
    break;
  case 4:
    ViewBookingHistory(ctx);
    break;
  case 5:
    cJSON_Delete(ctx->customers_json);
    cJSON_Delete(ctx->rooms_json);

    LoginMenu();
    return;
  }

  return;
}

void InitCustomer(char *id) {
  ft_set_default_border_style(FT_SOLID_ROUND_STYLE);

  CustomerContext ctx;
  ctx.customer_id = id;

  ctx.customers_json_data = ReadJSON("database/customers.json");
  ctx.customers_json = cJSON_Parse(ctx.customers_json_data);
  free(ctx.customers_json_data);

  ctx.rooms_json_data = ReadJSON("database/rooms.json");
  ctx.rooms_json = cJSON_Parse(ctx.rooms_json_data);
  free(ctx.rooms_json_data);

  ctx.current_customer_json = FindKey(ctx.customers_json, "id", ctx.customer_id);

  CustomerMenu(&ctx);
}
