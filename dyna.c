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
