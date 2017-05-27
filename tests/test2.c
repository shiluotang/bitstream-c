#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../src/bitstream.h"

#define TEST_ASSERT(CONDITION) \
    do { \
        if (!(CONDITION)) { \
            fprintf(stdout, "%s failed!\n", #CONDITION); \
            goto failure; \
        } \
    } while (0)

int main(int argc, char* *argv) {
    int rc = 0;
    int i = 0, n = 0;

    unsigned char buf[0xff];
    size_t const buflen = sizeof(buf);

    BitOutputStream bos = {0};
    BitInputStream bis = {0};

    BitOutputStreamInitialize(&bos, &buf[0], buflen * 8);
    for (i = 0, n = 6; i < n; ++i) {
        TEST_ASSERT(BS_SUCCEEDED(BitOutputStreamWriteUInt(&bos, 3, 3)));
        TEST_ASSERT(BitOutputStreamGetBitSize(&bos) == (i + 1) * 3);
        TEST_ASSERT(BitOutputStreamGetSize(&bos) == (i + 1) * 3 / 8);
    }

    goto success;
exit:
    return rc;
failure:
    goto cleanup;
success:
    goto cleanup;
cleanup:
    BitInputStreamRelease(&bis);
    BitOutputStreamRelease(&bos);
    goto exit;
}

