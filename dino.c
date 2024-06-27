#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  void **data;
  size_t used;
  size_t capacity;
} DynamicArray;

void DynamicArray_New(DynamicArray *a, size_t initialCapacity) {
  a->data = malloc(initialCapacity * sizeof(void *));
  a->used = 0;
  a->capacity = initialCapacity;
}

void DynamicArray_Push(DynamicArray *a, void *element) {
  if (a->used == a->capacity) {
    a->capacity *= 2;
    a->data = realloc(a->data, a->capacity * sizeof(void *));
  }
  a->data[a->used++] = element;
}

void DynamicArray_Free(DynamicArray *a) {
  free(a->data);
  a->data = NULL;
  a->used = 0;
  a->capacity = 0;
}

void DynamicArray_DeleteIndex(DynamicArray *a, size_t index) {
  if (index < a->used) {
    for (size_t i = index; i < a->used - 1; i++) {
      a->data[i] = a->data[i + 1];
    }
    a->used--;
  }
}

void *DynamicArray_GetIndex(DynamicArray *a, size_t index) {
  if (index < a->used) {
    return a->data[index];
  }
  return NULL;
}

typedef struct {
  char room_id[10];
  char booking_date[20];
} Booking;

typedef struct HashNode {
  Booking booking;
  struct HashNode *next;
} HashNode;

typedef struct {
  HashNode **table;
  size_t size;
} HashTable;

unsigned int hash(char *str, size_t size) {
  unsigned int hash = 0;
  while (*str) {
    hash = (hash << 5) + *str++;
  }
  return hash % size;
}

HashTable *HashTable_Create(size_t size) {
  HashTable *hashTable = malloc(sizeof(HashTable));
  hashTable->table = calloc(size, sizeof(HashNode *));
  hashTable->size = size;
  return hashTable;
}

void HashTable_Insert(HashTable *hashTable, Booking booking) {
  unsigned int index = hash(booking.room_id, hashTable->size);
  HashNode *newNode = malloc(sizeof(HashNode));
  newNode->booking = booking;
  newNode->next = hashTable->table[index];
  hashTable->table[index] = newNode;
}

Booking *HashTable_Search(HashTable *hashTable, char *room_id) {
  unsigned int index = hash(room_id, hashTable->size);
  HashNode *node = hashTable->table[index];
  while (node) {
    if (strcmp(node->booking.room_id, room_id) == 0) {
      return &node->booking;
    }
    node = node->next;
  }
  return NULL;
}

void HashTable_Free(HashTable *hashTable) {
  for (size_t i = 0; i < hashTable->size; i++) {
    HashNode *node = hashTable->table[i];
    while (node) {
      HashNode *temp = node;
      node = node->next;
      free(temp);
    }
  }
  free(hashTable->table);
  free(hashTable);
}

int main() {
  DynamicArray dynArray;
  DynamicArray_New(&dynArray, 2);

  HashTable *hashTable = HashTable_Create(10);

  Booking bookings[] = {{"A-12", "2024-06-10"}, {"B-34", "2024-06-11"}};

  for (int i = 0; i < 2; i++) {
    HashTable_Insert(hashTable, bookings[i]);
  }

  for (int i = 0; i < 10; i++) {
    HashNode *node = hashTable->table[i];
    while (node) {
      DynamicArray_Push(&dynArray, &node->booking);
      node = node->next;
    }
  }

  // Display the data in the dynamic array
  for (size_t i = 0; i < dynArray.used; i++) {
    Booking *booking = (Booking *)DynamicArray_GetIndex(&dynArray, i);
    if (booking) {
      printf("Room ID: %s, Booking Date: %s\n", booking->room_id,
             booking->booking_date);
    }
  }

  DynamicArray_Free(&dynArray);
  HashTable_Free(hashTable);
  return 0;
}
