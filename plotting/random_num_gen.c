#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_AMOUNT 1000
#define MIN 0
#define MAX 10000000

int main() {
  srand(time(NULL));

  FILE* out = fopen("random_nums.txt", "w");

  for (int i = 0; i < NUM_AMOUNT; i++) {
    int num = (rand() % (MAX - MIN + 1)) + MIN;
    fprintf(out, "%d\n", num);
  }

  fclose(out);

  return 0;
}