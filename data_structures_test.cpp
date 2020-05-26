#include "file_data_structures.h"


int main() {

	ContiguousAllocation CA(8);
	LinkedAllocation LA(8);

	CA.CreateFile(1, 32);
	LA.CreateFile(1, 32);

	assert(CA.Slice(0,100) == LA.Slice(0, 100));

	assert(CA.Access(1, 30) == 3);
	assert(LA.Access(1, 30) == 3);


	CA.Extend(1, 2);
	LA.Extend(1, 2);

	assert(CA.Slice(0,100) == LA.Slice(0, 100));



	CA.CreateFile(2, 32);
	LA.CreateFile(2, 32);

	assert(CA.Slice(0,100) == LA.Slice(0, 100));


	CA.Shrink(1, 2);
	LA.Shrink(1, 2);

	assert(CA.Slice(0,100) == LA.Slice(0, 100));	


	CA.CreateFile(3, 64);
	LA.CreateFile(3, 64);


	LA.Extend(3,32748);

	LA.CreateFile(5, 32);

	LA.Shrink(1, 2);
	LA.Shrink(2, 2);

	LA.CreateFile(6, 32);

	debug(LA.Slice(1,100), LA.available_space);

	cout << "Tests Successful\n";
}