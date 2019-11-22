
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int, char**)
{
	fprintf(stderr,"client started\n");

	char in { '\0' };
	while (in != ';') {
		read(0, &in, 1);
	}

	fprintf(stderr,"Tests finished\n");
}
