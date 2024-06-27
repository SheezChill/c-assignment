#ifndef __NANOID_H__
#define __NANOID_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <bcrypt.h>
#include <windows.h>
#pragma comment(lib, "bcrypt.lib")
#endif

// default alphabet list
// updated to nanoid 2.0
static char alphs[] = {'-', '_', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e',
                       'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                       'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
                       'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '\0'};

// custom alphabet pool and length
char *custom(char alphs[], int size);

// custom length
char *generate(int size);

// default character pool and 21 in length
char *simple();

// safe implementation of custom() which uses /dev/urandom or RtlGenRandom
char *safe_custom(char alphs[], int size);

// safe implementation of generate()
char *safe_generate(int size);

// safe implementation of simple()
char *safe_simple();

char *custom(char alphs[], int size) {
  int alph_size = strlen(alphs) - 1;
  char *id = (char *)malloc(sizeof(char) * size);

  for (int i = 0; i < size; i++) {
    int random_num;
    do {
      random_num = rand();
    } while (random_num >= (RAND_MAX - RAND_MAX % alph_size));

    random_num %= alph_size;
    id[i] = alphs[random_num];
  }

  return id;
}

char *generate(int size) {
  return custom(alphs, size);
}

char *simple() {
  return generate(21);
}

#ifdef _WIN32
int generate_random_bytes(unsigned char *buffer, size_t size) {
  return BCryptGenRandom(NULL, buffer, size, BCRYPT_USE_SYSTEM_PREFERRED_RNG) == STATUS_SUCCESS;
}
#else
int generate_random_bytes(unsigned char *buffer, size_t size) {
  FILE *urandom = fopen("/dev/urandom", "rb");
  if (urandom == NULL)
    return 0;

  fread(buffer, size, 1, urandom);
  fclose(urandom);

  return 1;
}
#endif

char *safe_custom(char alphs[], int size) {
  char *id = (char *)malloc(size);
  unsigned char *buffer = (unsigned char *)malloc(size);

  if (!generate_random_bytes(buffer, size)) {
    free(buffer);
    return NULL;
  }

  int alph_len = strlen(alphs);

  for (int i = 0; i < size; ++i) {
    id[i] = alphs[buffer[i] % alph_len];
  }

  free(buffer);
  return id;
}

char *safe_generate(int size) {
  return safe_custom(alphs, size);
}

char *safe_simple() {
  return safe_generate(21);
}

#endif // __NANOID_H__
