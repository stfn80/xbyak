#include <stdio.h>
#include <string.h>
#include "xbyak/xbyak.h"
#define NUM_OF_ARRAY(x) (sizeof(x) / sizeof(x[0]))

using namespace Xbyak;

struct TestJmp : public CodeGenerator {
	void putNop(int n)
	{
		for (int i = 0; i < n; i++) {
			nop();
		}
	}
/*
     4                                  X0:
     5 00000004 EBFE                    jmp short X0
     6
     7                                  X1:
     8 00000006 <res 00000001>          dummyX1 resb 1
     9 00000007 EBFD                    jmp short X1
    10
    11                                  X126:
    12 00000009 <res 0000007E>          dummyX126 resb 126
    13 00000087 EB80                    jmp short X126
    14
    15                                  X127:
    16 00000089 <res 0000007F>          dummyX127 resb 127
    17 00000108 E97CFFFFFF              jmp near X127
    18
    19 0000010D EB00                    jmp short Y0
    20                                  Y0:
    21
    22 0000010F EB01                    jmp short Y1
    23 00000111 <res 00000001>          dummyY1 resb 1
    24                                  Y1:
    25
    26 00000112 EB7F                    jmp short Y127
    27 00000114 <res 0000007F>          dummyY127 resb 127
    28                                  Y127:
    29
    30 00000193 E980000000              jmp near Y128
    31 00000198 <res 00000080>          dummyY128 resb 128
    32                                  Y128:
*/
	TestJmp(int offset, bool isBack, bool isShort)
	{
		char buf[32];
		static int count = 0;
		if (isBack) {
			sprintf(buf, "L(\"X%d\");\n", count);
			L(buf);
			putNop(offset);
			jmp(buf);
		} else {
			sprintf(buf, "L(\"Y%d\");\n", count);
			if (isShort) {
				jmp(buf);
			} else {
				jmp(buf, T_NEAR);
			}
			putNop(offset);
			L(buf);
		}
		count++;
	}
};

int main()
{
	static const struct Tbl {
		int offset;
		bool isBack;
		bool isShort;
		const char *result;
	} tbl[] = {
		{ 0, true, true, "EBFE" },
		{ 1, true, true, "EBFD" },
		{ 126, true, true, "EB80" },
		{ 127, true, false, "E97CFFFFFF" },
		{ 0, false, true, "EB00" },
		{ 1, false, true, "EB01" },
		{ 127, false, true, "EB7F" },
		{ 128, false, false, "E980000000" },
	};
	for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
		const Tbl *p = &tbl[i];
		TestJmp jmp(p->offset, p->isBack, p->isShort);
		const uint8 *q = (const uint8*)jmp.getCode();
		char buf[32];
		if (p->isBack) q += p->offset; /* skip nop */
		for (size_t j = 0; j < jmp.getSize() - p->offset; j++) {
			sprintf(&buf[j * 2], "%02X", q[j]);
		}
		if (strcmp(buf, p->result) != 0) {
			printf("error %d assume:%s, err=%s\n", i, p->result, buf);
		} else {
			printf("ok %d\n", i);
		}
	}
}
