#include <fs/fake_socket.h>

void test_stream();
void test_core();

int
main(
	int			aNumArgs,
	char**		aArgs)
{
	test_stream();
	test_core();

	return EXIT_SUCCESS;
}