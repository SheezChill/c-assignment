#include "../../include/login.h"
#include "../../include/utils.h"
#include "../../lib/cJSON.h"
#include "../../lib/fort.h"
#include <stdio.h>
#include <string.h>

// Function prototypes
void GuestMenu(cJSON *rooms_json, char *rooms_json_data);
void ViewRooms(cJSON *rooms_json);

// Function to display available rooms
void ViewRooms(cJSON *rooms_json) {
  // Create a table for displaying room information
  ft_table_t *table = ft_create_table();
  TableHeader(table, "Room ID", "Price", "Type");

  cJSON *item;

  // Iterate through each room in the JSON array
  cJSON_ArrayForEach(item, rooms_json) {
    // Check if the room is not booked
    if (cJSON_GetObjectItemCaseSensitive(item, "booked")->valueint == 0) {
      int price = cJSON_GetObjectItemCaseSensitive(item, "price")->valueint;
      char *capitalized_type = Capitalize(cJSON_GetObjectItemCaseSensitive(item, "type")->valuestring);

      // Add room information to the table
      ft_printf_ln(table, "%s|%d|%s", item->string, price, capitalized_type);

      free(capitalized_type);
    }
  }

  // Print the table
  printf("\n%s", ft_to_string(table));
  ft_destroy_table(table);

  // Return to the guest menu
  GuestMenu(rooms_json, NULL);
}

void SearchRooms(cJSON *rooms_json) {
  char *room_types[4] = {"suite", "deluxe", "double", "single"};

  printf("\nSelect the room type do you want to search for:");
  for (int i = 0; i < 4; i++) {
    char *capitalized_room_type = Capitalize(room_types[i]);
    printf("\n%d. %s", i + 1, capitalized_room_type);
    free(capitalized_room_type);
  }

  int choice = Choice("Enter choice", 1, 4) - 1;

  // Create a table for displaying room information
  ft_table_t *table = ft_create_table();
  TableHeader(table, "Room ID", "Price", "Type");

  // Iterate through each room in the JSON array
  cJSON *item;
  int room_result_num = 0;
  cJSON_ArrayForEach(item, rooms_json) {
    // Check if the room is not booked
    if (cJSON_GetObjectItemCaseSensitive(item, "booked")->valueint == 0 &&
        strcmp(cJSON_GetObjectItemCaseSensitive(item, "type")->valuestring, room_types[choice]) == 0) {
      room_result_num++;

      int price = cJSON_GetObjectItemCaseSensitive(item, "price")->valueint;
      char *capitalized_type = Capitalize(cJSON_GetObjectItemCaseSensitive(item, "type")->valuestring);

      // Add room information to the table
      ft_printf_ln(table, "%s|%d|%s", item->string, price, capitalized_type);

      free(capitalized_type);
    }
  }

  if (room_result_num == 0) {
    printf("No room with this type is available, try again later.\n");
    ft_destroy_table(table);
    GuestMenu(rooms_json, NULL);
    return;
  }

  // Print the table
  printf("\n%s", ft_to_string(table));
  ft_destroy_table(table);

  // Return to the guest menu
  GuestMenu(rooms_json, NULL);
}

// Function to display and handle guest menu options
void GuestMenu(cJSON *rooms_json, char *rooms_json_data) {
  printf("\n--- Guest Menu ---");
  printf("\n1. View available rooms");
  printf("\n2. Search for room type");
  printf("\n3. Back");

  // Get user choice
  int choice = Choice("Enter choice", 1, 3);

  switch (choice) {
  case 1:
    ViewRooms(rooms_json);
    break;
  case 2:
    SearchRooms(rooms_json);
    break;
  case 3:
    // Clean up and return to login menu
    cJSON_Delete(rooms_json);
    if (rooms_json_data)
      free(rooms_json_data);

    LoginMenu();
    break;
  }
}

// Function to initialize the guest interface
void InitGuest() {
  // Set the default border style for tables
  ft_set_default_border_style(FT_SOLID_ROUND_STYLE);

  // Read room data from JSON file
  char *rooms_json_data = ReadJSON("database/rooms.json");
  cJSON *rooms_json = cJSON_Parse(rooms_json_data);

  // Display the guest menu
  GuestMenu(rooms_json, rooms_json_data);
}
