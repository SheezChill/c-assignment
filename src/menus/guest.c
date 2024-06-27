#include "../../include/login.h"
#include "../../include/utils.h"
#include "../../lib/cJSON.h"
#include "../../lib/fort.h"
#include <stdio.h>

ft_table_t *table;

cJSON *item;

char *rooms_json_data;
cJSON *rooms_json;

void GuestMenu();

void ViewRooms() {
  table = ft_create_table();

  TableHeader(table, "Room ID", "Price", "Type");

  cJSON_ArrayForEach(item, rooms_json) {
    if (cJSON_GetObjectItemCaseSensitive(item, "booked")->valueint == 0) {
      int price = cJSON_GetObjectItemCaseSensitive(item, "price")->valueint;
      char *capitalized_type = Capitalize(cJSON_GetObjectItemCaseSensitive(item, "type")->valuestring);

      ft_printf_ln(table, "%s|%d|%s", item->string, price, capitalized_type);

      free(capitalized_type);
    }
  }

  printf("%s\n", ft_to_string(table));
  ft_destroy_table(table);

  GuestMenu();
}

void GuestMenu() {
  printf("\n--- Guest Menu ---");
  printf("\n1. View available rooms");
  printf("\n2. Back");

  int choice = Choice("Enter choice", 1, 2);

  switch (choice) {
  case 1:
    ViewRooms();
    break;
  case 2:
    cJSON_Delete(rooms_json);
    free(rooms_json_data);

    LoginMenu();
    break;
  }
}

void InitGuest() {
  ft_set_default_border_style(FT_SOLID_ROUND_STYLE);

  rooms_json_data = ReadJSON("database/rooms.json");
  rooms_json = cJSON_Parse(rooms_json_data);

  GuestMenu();
}
