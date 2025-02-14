#include<stdlib.h>
#include<stdbool.h>
#include<stdio.h>
#include<sys/types.h>
#include <wchar.h>

// TODO if we exit failure at a point after opening the file, should we close it before exiting?
int main(int argc, char *argv[]) {
  if(argc <= 1) {
    printf("Not enough arguments supplied!\n");
    return EXIT_FAILURE;
  }

  char *datapath = argv[1];
  printf("Loading example sentences from %s\n", datapath);

  FILE* datafile = fopen(datapath, "r");
  if(datafile == NULL) {
    printf("Could not open %s\n", datapath);
    return EXIT_FAILURE;
  }

  // TODO what's the difference between the behavior of char and unsigned char here?
  // TODO what happens when you downcast from char to unsigned char? can we math it out?
  // TODO note that this is not a string, right? as it is not null-terminated!
  // start by reading the utf-8 bom
  unsigned char data[3];
  size_t readc = fread(&data, sizeof(unsigned char), 3, datafile);
  if (readc != 3 || data[0] != 0xef || data[1] != 0xbb || data[2] != 0xbf) {
    printf("Could not read utf-8 byte-order-mark\n");
    return EXIT_FAILURE;
  }

  // TODO check for unicode validity
  // TODO why is there a gap in between 1 and 2 byte
  int bytec = 0;
  int extra = 0;
  int lines = 0;
  while (lines < 10) {
    unsigned char first_byte;
    readc = fread(&first_byte, sizeof(unsigned char), 1, datafile);
    if (readc == 0)
      break;
    if (first_byte >= 0b11111000) {
      printf("Error decoding utf-8");
      return EXIT_FAILURE;
    }
    else if (first_byte >= 0b11110000)
      bytec = 4;
    else if (first_byte >= 0b11100000)
      bytec = 3;
    else if (first_byte >= 0b11000000)
      bytec = 2;
    else if (first_byte >= 0b10000000) {
      printf("Error decoding utf-8");
      return EXIT_FAILURE;
    }
    else
      bytec = 1;
    //printf("Read first character: %c / 0x%02x / %d-byte\n", first_byte, first_byte, bytec);

    extra = bytec - 1;
    // if it's a one-byte code point, we're done
    if (extra == 0) {
      if (first_byte == '\t')
        printf(" / ");
      else
        printf("%c", first_byte);
      if (first_byte == '\n') {
        lines++;
        printf("\n");
      }
      continue;
    }

    char *data = (char *) malloc(sizeof(char) * extra);
    readc = fread(data, sizeof(char), extra, datafile);
    if (readc != extra) {
      printf("Error decoding utf-8");
      return EXIT_FAILURE;
    }
    //printf("Found %d byte code point. Reading %d more characters: %c%.*s\n", bytec, extra, first_byte, extra, data);
    printf("%c%.*s", first_byte, extra, data);
  }

  fclose(datafile);

  return EXIT_SUCCESS;
}
