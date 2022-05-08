#include <stdlib.h>
#include <stdio.h>
#include <monitors.h>

int main(int argc, char **argv)
{
	setup_monitors();
	start_monitoring();
	pause();
	fprintf(stderr, "tearing down monitors\n");
	teardown_monitors();
}

