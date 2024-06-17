#include <unistd.h>

int main(void)
{
	syscall(1, 1, "hello\n", 6);
	return 0;
}