

/* Genode includes */
#include <util/string.h>

/* libc includes */
#include <stdio.h>
#include <unistd.h>


int main(int argc, char *argv[])
{
	enum {
		DEFAULT_DELAY_S  =  30,
		US_PER_S         =  1'000'000,
	};

	char   const file_name[]          { "/results/test_detail.xml" };
	char   const file_content[]       { R"(<test_result>
	<result name="A" success="yes"/>
	<result name="B" success="no"/>
</test_result>
)" };

	size_t delay_s { DEFAULT_DELAY_S };
	if (argc > 1) {
		Genode::ascii_to_unsigned(argv[1], delay_s, 10) * US_PER_S;
	}
	size_t delay_us { delay_s * US_PER_S };

	printf("wait %lu seconds before writing result file\n",delay_s);
	usleep(delay_us);

	printf("write result file '%s' now\n",file_name);
	FILE *f = fopen(file_name, "w+");
	if (!f) {
		printf("failed to open result file '%s' to write\n",file_name);
		return -1;
	}

	fprintf(f, file_content);
	fclose(f);

	printf("result file '%s' written\n",file_name);

	return 0;
}
