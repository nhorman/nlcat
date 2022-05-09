#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <monitors.h>

int main(int argc __unused, char **argv __unused)
{
	setup_monitors();
	start_monitoring();
	pause();
	fprintf(stderr, "tearing down monitors\n");
	teardown_monitors();
}

