#include <fs/fake_socket.h>

void test_stream();
void test_core();
void test_server();

int
main(
	int			aNumArgs,
	char**		aArgs)
{
	test_stream();
	test_core();
	test_server();

	return EXIT_SUCCESS;
}