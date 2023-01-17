#include "ctf.h"
#include <stdlib.h>

/*
 * Initialize the 2D square array to a size of (size*size)
 *
 * @param size, the number of elements of one line in the array, as well as the
 * number of rows. For maximum observability, make size equal a multiple of a
 * cache line length
 */
int **initialize(int size) {
  trace_entry(0x1000);
  int **array = (int **)malloc(sizeof(int[size][size]));
  for (int x = 0; x < size; x++) {
    array[x] = (int *)malloc(sizeof(int[size]));
  }
  trace_exit(0x1000);
  return array;
}

/*
 * Teardown, clean up, free ram
 *
 * @param array, the array to clean
 * @size the number of rows in the array
 */
void teardown(int **array, int size) {
  trace_entry(0x4000);
  free(array);
  trace_exit(0x4000);
}

/*
 * Set the value of an entry in the array.
 */
void set_value(int **array, int x, int y, int value) { array[x][y] = value; }

/*
 * Set the values of the array, first setting X and then Y. This should be
 * faster as it maintains cache locality.
 *
 * @param array, the array to traverse
 * @size the number of rows in the array
 */
void first_x_then_y(int **array, int size) {
  trace_entry(0x2000);
  for (int x = 0; x < size; x++) {
    trace_entry(0x2100);
    for (int y = 0; y < size; y++) {
      set_value(array, x, y, 0xDEADBEEF);
    }
    trace_exit(0x2100);
  }
  trace_exit(0x2000);
}

/*
 * Set the values of the array, first setting X and then Y. This should be
 * slower as it thrashes the cache.
 *
 * @param array, the array to traverse
 * @size the number of rows in the array
 */
void first_y_then_x(int **array, int size) {
  trace_entry(0x3000);
  for (int y = 0; y < size; y++) {
    trace_entry(0x3100);
    for (int x = 0; x < size; x++) {
      set_value(array, x, y, 0xDEADBEEF);
    }
    trace_exit(0x3100);
  }
  trace_exit(0x3000);
}

/*
 * Main function. Calls the two array traversal functions to compare them.
 */
int main() {
  init_trace("trace-bad2");

  const int SIZE = 10000;
  int **array = initialize(SIZE);
  first_x_then_y(array, SIZE);
  first_y_then_x(array, SIZE);
  teardown(array, SIZE);
  flush();
}
