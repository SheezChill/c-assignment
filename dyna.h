#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <stdlib.h>

typedef struct {
  void **data;
  size_t used;
  size_t capacity;
} DynamicArray;

void DynamicArray_New(DynamicArray *a, size_t initialCapacity);
void DynamicArray_Push(DynamicArray *a, void *element);
void DynamicArray_Free(DynamicArray *a);
void DynamicArray_DeleteIndex(DynamicArray *a, size_t index);
void *DynamicArray_GetIndex(DynamicArray *a, size_t index);

#endif // DYNAMIC_ARRAY_H
