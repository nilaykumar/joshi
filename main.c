#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

int get_code_point_length(unsigned char leader_byte) {
  if (leader_byte >> 7 == 0x0)
    return 1;
  if (leader_byte >> 5 == 0x6)
    return 2;
  if (leader_byte >> 4 == 0xe)
    return 3;
  if (leader_byte >> 3 == 0x1e)
    return 4;
  return 0;
}

bool is_valid_continuation_bytes(unsigned char *cont, int cont_length) {
  for (int i = 0; i < cont_length; i++) {
    if(cont[i] >> 6 != 0x2)
      return false;
  }
  return true;
}

bool is_valid_utf8_char(unsigned char *code_point) {
  int code_point_length = get_code_point_length((code_point[0]));
  if (!code_point_length)
    return false;
  return is_valid_continuation_bytes(code_point + sizeof(unsigned char), code_point_length - 1);
}

// TODO if we exit failure at a point after opening the file, should we close it
// before exiting?
int main(int argc, char *argv[]) {
  if (argc <= 1) {
    printf("Not enough arguments supplied!\n");
    return EXIT_FAILURE;
  }

  char *datapath = argv[1];
  printf("Loading example sentences from %s\n", datapath);

  FILE *datafile = fopen(datapath, "r");
  if (datafile == NULL) {
    printf("Could not open %s\n", datapath);
    return EXIT_FAILURE;
  }

  // start by reading the utf-8 bom
  unsigned char data[3];
  size_t readc = fread(&data, sizeof(unsigned char), 3, datafile);
  if (readc != 3 || data[0] != 0xef || data[1] != 0xbb || data[2] != 0xbf) {
    printf("Could not read utf-8 byte-order-mark\n");
    return EXIT_FAILURE;
  }

  int bytec = 0;
  int extra = 0;
  int lines = 0;
  while (lines < 3) {
    unsigned char first_byte;
    readc = fread(&first_byte, sizeof(unsigned char), 1, datafile);
    if (readc == 0)
      break;
    bytec = get_code_point_length(first_byte);
    if (!bytec) {
      printf("Error decoding utf-8");
      return EXIT_FAILURE;
    }

    extra = bytec - 1;
    // if it's a one-byte code point, we're done
    if (extra == 0) {
      if (first_byte == '\t')
        printf(" / ");
      else
        printf("%c", first_byte);
      if (first_byte == '\n')
        lines++;
      continue;
    }

    unsigned char *data = (unsigned char *)malloc(sizeof(unsigned char) * extra);
    readc = fread(data, sizeof(unsigned char), extra, datafile);
    // ensure that we can read the number of bytes implied by the leader byte
    if (readc != extra) {
      printf("Error decoding utf-8");
      return EXIT_FAILURE;
    }
    // verify that the read bytes are continuation bytes
    if (!is_valid_continuation_bytes(data, extra)) {
      printf("Error decoding utf-8");
      return EXIT_FAILURE;
    }
    printf("%c%.*s", first_byte, extra, data);

    free(data);
  }

  fclose(datafile);

  return EXIT_SUCCESS;
}

