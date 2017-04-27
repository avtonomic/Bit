#include "coder.h"
#include "command.h"
#include <inttypes.h>
#include <stdio.h>

int encode(uint32_t code_point, CodeUnits *code_units)
{
	int code_128 = 128, code_63 = 63;

	if (code_point <= 0x7F) {
		code_units->length = 1;
		code_units->code[0] = code_point;
		return 0;
	} else if (code_point <= 0x7FF) {
		code_units->length = 2;
	} else if (code_point <= 0xFFFF) {
		code_units->length = 3;
	} else if (code_point <= 0x1FFFFF){
		code_units->length = 4;
	} else {
		return -1;
	}

	code_units->code[0] = code_128;
	for (size_t i = 1; i < code_units->length; i++) {
		code_units->code[0] = code_units->code[0] | (code_128 >> i);
	}
	code_units->code[0] = code_units->code[0] | (code_point >> 6 * (code_units->length - 1));

	for (size_t i = 1; i < code_units->length; i++) {
		code_units->code[i] = code_128 | ((code_point >> (6 * (code_units->length - i - 1))) & code_63);
	}
	print_bit(code_units);

	return 0;
}

uint32_t decode(const CodeUnits *code_units)
{
	uint32_t code_point = 0;

	if (code_units->length == 1) {
		code_point = code_units->code[0];
	} else if (code_units->length == 2) {
		code_point = code_units->code[0] & 0x3f;
	} else if (code_units->length == 3) {
		code_point = code_units->code[0] & 0x1f;
	} else if (code_units->length == 4) {
		code_point = code_units->code[0] & 0x0f;
	} else {
		return -1;
	}

	for (size_t i = 1; i < code_units->length; i++) {
		code_point = (code_point << 6) | (code_units->code[i] & 0x7f);
	}

	return code_point;
}

int read_next_code_unit(FILE *in, CodeUnits *code_units)
{
	while (!feof(in)) {
		fread(&code_units->code[0], 1, 1, in);
		if (code_units->code[0] <= 0x7f) {
			code_units->length = 1;
		} else if (code_units->code[0] <= 0xdf) {
			code_units->length = 2;
		} else if (code_units->code[0] <= 0xef) {
			code_units->length = 3;
		} else if (code_units->code[0] <= 0xf7) {
			code_units->length = 4;
		} else {
			continue;
		}

		for (size_t i = 1; i < code_units->length; i++) {
			fread(&code_units->code[i], 1, 1, in);

			if ((code_units->code[i] & 0xc0) != 0x80) {
				fseek(in, -1, SEEK_CUR);
				continue;
			}
			if (i != code_units->length - 1) {
				if (feof(in) != 0) {
					return -1;
				}
			}
		}

		return 0;
	}

	return -1;
}

int write_code_unit(FILE *out, const CodeUnits *code_units)
{
	fwrite(code_units->code, code_units->length, sizeof(code_units->code), out);

	return 0;
}

void print_bit(const CodeUnits *code_units)
{
	int code_128 = 128;
	int code_bit = 0;

	for (size_t i = 0;i < code_units->length;++i) {
		for (size_t j = 0;j < 8;++j) {
			code_bit = code_128 & code_units->code[i];
			if (code_bit != 0) {
				printf("%d", 1);
			} else printf("%d", 0);
			code_128 = code_128 >> 1;
		}
		printf(" ");
		code_128 = 128;
	}
	printf("\n");
}