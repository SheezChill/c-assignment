#include "../../include/login.h"
#include "../../include/utils.h"
#include "../../lib/asprintf.h"
#include "../../lib/cJSON.h"
#include "../../lib/fort.h"
#include <stdio.h>

typedef struct {
  cJSON *customers_json;
  cJSON *rooms_json;
  ft_table_t *table;
} StaffContext;

void StaffMenu(StaffContext *staffContext);
void ToggleCheckInOut(StaffContext *staffContext);
void ViewRoomAvailability(StaffContext *staffContext);
void ViewAllRoomBookingHistory(StaffContext *staffContext);
void ViewSpecificRoomBookingHistory(StaffContext *staffContext);

void ViewAllRoomBookingHistory(StaffContext *staffContext) {
  cJSON *item;
  cJSON *rooms_json = staffContext->rooms_json;

  cJSON_ArrayForEach(item, rooms_json) {
    staffContext->table = ft_create_table();

    cJSON *booking_history_array = cJSON_GetObjectItemCaseSensitive(item, "history");

    ft_set_cell_span(staffContext->table, 0, 0, 3);
    ft_set_cell_prop(staffContext->table, 0, FT_ANY_COLUMN, FT_CPROP_TEXT_ALIGN, FT_ALIGNED_CENTER);
    ft_write_ln(staffContext->table, item->string);

    if (cJSON_GetArraySize(booking_history_array) > 0) {
      ft_set_cell_prop(staffContext->table, 1, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
      ft_write_ln(staffContext->table, "Booked by", "Booking start", "Booking end");

      cJSON *history;
      cJSON_ArrayForEach(history, booking_history_array) {
        cJSON *booked_by = cJSON_GetObjectItemCaseSensitive(history, "booked_by");
        cJSON *booking_start = cJSON_GetObjectItemCaseSensitive(history, "booking_start");
        cJSON *booking_end = cJSON_GetObjectItemCaseSensitive(history, "booking_end");

        ft_write_ln(staffContext->table,
                    booked_by->valuestring,
                    ParseISODateTime(booking_start->valuestring),
                    ParseISODateTime(booking_end->valuestring));

        ft_add_separator(staffContext->table);
      }
    } else {
      ft_set_cell_span(staffContext->table, 1, 0, 3);
      ft_set_cell_prop(staffContext->table, 1, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
      ft_write_ln(staffContext->table, "No booking history");
    }

    printf("%s\n", ft_to_string(staffContext->table));
    ft_destroy_table(staffContext->table);
  }
}

void ViewSpecificRoomBookingHistory(StaffContext *staffContext) {
  cJSON *rooms_json = staffContext->rooms_json;

  char *ids[cJSON_GetArraySize(rooms_json)];

  int i = 0;
  cJSON *item;
  cJSON_ArrayForEach(item, rooms_json) {
    ids[i] = item->string;
    i++;

    printf("\n%d. %s", i, item->string);
  }

  int selected_id_index = Choice("Enter choice (0 to go back)", 0, cJSON_GetArraySize(rooms_json)) - 1;

  if (selected_id_index == -1) {
    return;
  }

  char *selected_id = ids[selected_id_index];

  staffContext->table = ft_create_table();
  ft_set_cell_span(staffContext->table, 0, 0, 3);
  ft_set_cell_prop(staffContext->table, 0, FT_ANY_COLUMN, FT_CPROP_TEXT_ALIGN, FT_ALIGNED_CENTER);
  ft_write_ln(staffContext->table, selected_id);

  cJSON *history_array =
      cJSON_GetObjectItemCaseSensitive(cJSON_GetObjectItemCaseSensitive(rooms_json, selected_id), "history");

  if (cJSON_GetArraySize(history_array) > 0) {
    ft_set_cell_prop(staffContext->table, 1, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
    ft_write_ln(staffContext->table, "Booked by", "Booking start", "Booking end");

    cJSON_ArrayForEach(item, history_array) {
      cJSON *booked_by = cJSON_GetObjectItemCaseSensitive(item, "booked_by");
      cJSON *booking_start = cJSON_GetObjectItemCaseSensitive(item, "booking_start");
      cJSON *booking_end = cJSON_GetObjectItemCaseSensitive(item, "booking_end");

      ft_write_ln(
          staffContext->table, booked_by->valuestring, booking_start->valuestring, booking_end->valuestring);
    }
  } else {
    ft_set_cell_span(staffContext->table, 1, 0, 3);
    ft_set_cell_prop(staffContext->table, 1, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
    ft_write_ln(staffContext->table, "No booking history");
  }

  printf("%s\n", ft_to_string(staffContext->table));
  ft_destroy_table(staffContext->table);
}

void ToggleCheckInOut(StaffContext *staffContext) {
  cJSON *customers_json = staffContext->customers_json;

  char *customer_ids[cJSON_GetArraySize(customers_json)];

  while (1) {
    int i = 0;
    cJSON *item;
    cJSON_ArrayForEach(item, customers_json) {
      customer_ids[i] = cJSON_GetObjectItemCaseSensitive(item, "id")->valuestring;
      i++;

      char *buffer;
      asprintf(&buffer,
               "[%s] %s",
               cJSON_GetObjectItemCaseSensitive(item, "id")->valuestring,
               cJSON_GetObjectItemCaseSensitive(item, "name")->valuestring);
      printf("\n%d. %s", i, buffer);
    }

    int selected_id_index = Choice("Enter choice (0 to go back)", 0, cJSON_GetArraySize(customers_json)) - 1;

    if (selected_id_index == -1) {
      StaffMenu(staffContext);
      break;
    }

    cJSON *bookings_array =
        cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(customers_json, selected_id_index), "bookings");

    while (1) {
      staffContext->table = ft_create_table();
      TableHeader(
          staffContext->table, "No.", "Room Id", "Booking start", "Booking end", "Checked in", "Checked out");

      int j = 1;
      cJSON_ArrayForEach(item, bookings_array) {
        ft_printf_ln(staffContext->table,
                     "%d|%s|%s|%s|%s|%s",
                     j,
                     cJSON_GetObjectItemCaseSensitive(item, "room_id")->valuestring,
                     ParseISODateTime(cJSON_GetObjectItemCaseSensitive(item, "booking_start")->valuestring),
                     ParseISODateTime(cJSON_GetObjectItemCaseSensitive(item, "booking_end")->valuestring),
                     cJSON_GetObjectItemCaseSensitive(item, "checked_in")->valueint ? "Yes" : "No",
                     cJSON_GetObjectItemCaseSensitive(item, "checked_out")->valueint ? "Yes" : "No");
        j++;
      }

      printf("%s\n", ft_to_string(staffContext->table));
      ft_destroy_table(staffContext->table);

      int selected_booking_index =
          Choice("Booking to manage (0 to go back)", 0, cJSON_GetArraySize(bookings_array)) - 1;
      if (selected_booking_index == -1) {
        break;
      }
      cJSON *selected_booking = cJSON_GetArrayItem(bookings_array, selected_booking_index);

      int exit = 0;
      while (exit == 0) {
        staffContext->table = ft_create_table();

        TableHeader(
            staffContext->table, "Room Id", "Booking start", "Booking end", "Checked in", "Checked out");
        ft_write_ln(
            staffContext->table,
            cJSON_GetObjectItemCaseSensitive(selected_booking, "room_id")->valuestring,
            ParseISODateTime(
                cJSON_GetObjectItemCaseSensitive(selected_booking, "booking_start")->valuestring),
            ParseISODateTime(cJSON_GetObjectItemCaseSensitive(selected_booking, "booking_end")->valuestring),
            cJSON_GetObjectItemCaseSensitive(selected_booking, "checked_in")->valueint ? "Yes" : "No",
            cJSON_GetObjectItemCaseSensitive(selected_booking, "checked_out")->valueint ? "Yes" : "No");

        printf("%s\n", ft_to_string(staffContext->table));
        ft_destroy_table(staffContext->table);

        printf("\n1. Toggle check in");
        printf("\n2. Toggle check out");
        printf("\n3. Back");

        int choice = Choice("Enter choice", 1, 3);
        switch (choice) {
        case 1: {
          if (cJSON_GetObjectItemCaseSensitive(selected_booking, "checked_out")->valueint == 1) {
            printf("\nThis guest has already checked out!\n");
            continue;
          }

          cJSON *checked_in = cJSON_GetObjectItemCaseSensitive(selected_booking, "checked_in");
          cJSON_SetBoolValue(checked_in, checked_in->valueint ? 0 : 1);
          checked_in->valueint = checked_in->valueint ? 0 : 1;
          WriteJSON("database/customers.json", customers_json);
          break;
        }
        case 2: {
          if (cJSON_GetObjectItemCaseSensitive(selected_booking, "checked_in")->valueint == 0) {
            printf("\nThis guest is not checked in yet!\n");
            continue;
          }

          cJSON *checked_out = cJSON_GetObjectItemCaseSensitive(selected_booking, "checked_out");
          cJSON_SetBoolValue(checked_out, checked_out->valueint ? 0 : 1);
          checked_out->valueint = checked_out->valueint ? 0 : 1;

          cJSON *new_room_history = cJSON_CreateObject();
          cJSON_AddStringToObject(
              new_room_history,
              "booked_by",
              cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(customers_json, selected_id_index), "id")
                  ->valuestring);
          cJSON_AddStringToObject(
              new_room_history,
              "booking_start",
              cJSON_GetObjectItemCaseSensitive(selected_booking, "booking_start")->valuestring);
          cJSON_AddStringToObject(
              new_room_history,
              "booking_end",
              cJSON_GetObjectItemCaseSensitive(selected_booking, "booking_end")->valuestring);

          cJSON *room_history_array = cJSON_GetObjectItemCaseSensitive(
              cJSON_GetObjectItemCaseSensitive(
                  staffContext->rooms_json,
                  cJSON_GetObjectItemCaseSensitive(selected_booking, "room_id")->valuestring),
              "history");

          if (checked_out->valueint == 1) {
            cJSON_AddItemToArray(room_history_array, new_room_history);

            cJSON *selected_room = cJSON_GetObjectItemCaseSensitive(
                staffContext->rooms_json,
                cJSON_GetObjectItemCaseSensitive(selected_booking, "room_id")->valuestring);
            cJSON_SetBoolValue(cJSON_GetObjectItemCaseSensitive(selected_room, "booked"), 0);
            cJSON_ReplaceItemInObjectCaseSensitive(selected_room, "booked_by", cJSON_CreateNull());

            WriteJSON("database/rooms.json", staffContext->rooms_json);
          } else {
            cJSON_ArrayForEach(item, room_history_array) {
              if (cJSON_Compare(item, new_room_history, 0) == 1) {
                cJSON *detached_item = cJSON_DetachItemViaPointer(room_history_array, item);
                cJSON_Delete(detached_item);
                break;
              }
            }

            cJSON *selected_room = cJSON_GetObjectItemCaseSensitive(
                staffContext->rooms_json,
                cJSON_GetObjectItemCaseSensitive(selected_booking, "room_id")->valuestring);
            cJSON_SetBoolValue(cJSON_GetObjectItemCaseSensitive(selected_room, "booked"), 1);

            cJSON_ReplaceItemInObjectCaseSensitive(
                selected_room,
                "booked_by",
                cJSON_CreateString(cJSON_GetObjectItemCaseSensitive(
                                       cJSON_GetArrayItem(customers_json, selected_id_index), "id")
                                       ->valuestring));

            WriteJSON("database/rooms.json", staffContext->rooms_json);
          }

          WriteJSON("database/customers.json", customers_json);
          break;
        }
        case 3:
          exit = 1;
          break;
        }
      }
    }
  }
}

void ViewRoomAvailability(StaffContext *staffContext) {
  staffContext->table = ft_create_table();

  TableHeader(staffContext->table, "Room ID", "Available");

  cJSON *item;
  cJSON *rooms_json = staffContext->rooms_json;
  cJSON_ArrayForEach(item, rooms_json) {
    ft_write_ln(staffContext->table,
                item->string,
                cJSON_GetObjectItemCaseSensitive(item, "booked")->valueint ? "No" : "Yes");
  }

  printf("%s\n", ft_to_string(staffContext->table));
  ft_destroy_table(staffContext->table);

  StaffMenu(staffContext);
}

void StaffMenu(StaffContext *staffContext) {
  printf("\n--- Staff Menu ---");
  printf("\n1. Check-in and check-out guests");
  printf("\n2. View room availability");
  printf("\n3. View room booking history");
  printf("\n4. Logout");

  int choice = Choice("Enter choice", 1, 4);

  switch (choice) {
  case 1:
    ToggleCheckInOut(staffContext);
    break;
  case 2:
    ViewRoomAvailability(staffContext);
    break;
  case 3: {
    while (1) {
      printf("\n1. View all booking history");
      printf("\n2. View specific room's history");
      printf("\n3. Back");

      int choice = Choice("Enter choice", 1, 3);

      switch (choice) {
      case 1:
        ViewAllRoomBookingHistory(staffContext);
        break;
      case 2:
        ViewSpecificRoomBookingHistory(staffContext);
        break;
      case 3:
        StaffMenu(staffContext);
        return;
      }
    }

    break;
  }
  case 4:
    cJSON_Delete(staffContext->customers_json);
    cJSON_Delete(staffContext->rooms_json);

    LoginMenu();
    return;
  }
}

void InitStaff() {
  ft_set_default_border_style(FT_SOLID_ROUND_STYLE);

  StaffContext staffContext;
  char *customers_json_data = ReadJSON("database/customers.json");
  staffContext.customers_json = cJSON_Parse(customers_json_data);
  free(customers_json_data);

  char *rooms_json_data = ReadJSON("database/rooms.json");
  staffContext.rooms_json = cJSON_Parse(rooms_json_data);
  free(rooms_json_data);

  StaffMenu(&staffContext);
}
