#include "coder.h"
#include "command.h"
#include <string.h>

int main(int argc, char *argv[])
{

	if (strcmp("encode", argv[1]) == 0) {
		encode_file(argv[2], argv[3]);
	} else if (strcmp("decode", argv[1]) == 0) {
		decode_file(argv[2], argv[3]);
	} else {
		printf("Invalid first argument\n");
	}

	return 0;
}
