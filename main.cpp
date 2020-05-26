#include "file_data_structures.h"


int main() {

  LinkedAllocation LA(1024);
  ContiguousAllocation CA(1024);

  LA.CreateFile(3, 10000);
  CA.CreateFile(3, 10000);

  LA.PrintSlice(0, 20);
  puts("");
  CA.PrintSlice(0, 20);
  puts("");

  LA.Shrink(3, 3);
  CA.Shrink(3, 3);

  LA.PrintSlice(0, 20);
  puts("");
  CA.PrintSlice(0, 20);
  puts("");

  LA.Extend(3, 1);
  CA.Extend(3, 1);

  LA.PrintSlice(0, 20);
  puts("");
  CA.PrintSlice(0, 20);
  puts("");

}
