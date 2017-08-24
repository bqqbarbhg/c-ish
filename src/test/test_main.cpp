#include "test.h"

int main(int argc, char **argv)
{
	bool good = run_tests();
	return good ? 0 : 1;
}
