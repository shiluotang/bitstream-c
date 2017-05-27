#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../src/bitstream.h"

static int printBinary(BitInputStream *const bis) {
    int rc = 0;
    ReadResult r;
    size_t bpos = BitInputStreamGetBitPosition(bis);

    while (BS_SUCCEEDED(r = BitInputStreamReadBit(bis))) {
        if (printf("%u", (unsigned int) r._M_value.uint) < 1)
            goto failure;
        ++rc;
    }
    goto success;
exit:
    return rc;
failure:
    goto cleanup;
success:
    goto cleanup;
cleanup:
    BitInputStreamSeekBits(bis, bpos, SEEK_SET);
    goto exit;
}

#define TEST_ASSERT(CONDITION) \
    do { \
        if (!(CONDITION)) { \
            fprintf(stdout, "%s failed!\n", #CONDITION); \
            goto failure; \
        } \
    } while (0)

typedef int (*perf_runner)(void*);

int perf_test(char const* name, perf_runner runner, size_t n, void *arg) {
    int rc = 0;
    size_t i = 0;
    time_t t1, t2;
    time(&t1);
    for (i = 0; i < n; ++i)
        rc = runner(arg);
    time(&t2);
    fprintf(stdout, "Name = %s, N = %lu, Total = %g s, Average = %g s, QPS = %g\n",
            name, (unsigned long) n, difftime(t2, t1), difftime(t2, t1) / n,
            n / difftime(t2, t1));
    return rc;
}

int test_output_perf(void *arg) {
    int rc;
    BitOutputStream *const bos = (BitOutputStream*) arg;
    size_t bpos = BitOutputStreamGetBitSize(bos);

    if (!BS_SUCCEEDED(BitOutputStreamWriteInt(bos, 5, 1)))
        goto failure;
    if (!BS_SUCCEEDED(BitOutputStreamWriteInt(bos, 5, -1)))
        goto failure;
    if (!BS_SUCCEEDED(BitOutputStreamWriteSInt(bos, 5, 2)))
        goto failure;
    if (!BS_SUCCEEDED(BitOutputStreamWriteSInt(bos, 5, -2)))
        goto failure;
    if (!BS_SUCCEEDED(BitOutputStreamWriteUInt(bos, 5, 3)))
        goto failure;
    if (!BS_SUCCEEDED(BitOutputStreamWriteUInt(bos, 5, -3)))
        goto failure;
    TEST_ASSERT(BitOutputStreamGetBitSize(bos) == 5 * 6);
    goto success;
exit:
    return rc;
failure:
    rc = EXIT_FAILURE;
    goto cleanup;
success:
    rc = EXIT_SUCCESS;
    goto cleanup;
cleanup:
    BitOutputStreamSeekBits(bos, bpos, SEEK_SET);
    goto exit;
}

int test_input_perf(void *arg) {
    int rc;
    ReadResult r;
    BitInputStream *const bis = (BitInputStream*) arg;
    size_t bpos = BitInputStreamGetBitPosition(bis);

    r._M_status = BS_SUCCESS;
    if (!BS_SUCCEEDED(r = BitInputStreamReadInt(bis, 5)))
        goto failure;
    TEST_ASSERT(r._M_value.sint == 1);
    if (!BS_SUCCEEDED(r = BitInputStreamReadInt(bis, 5)))
        goto failure;
    TEST_ASSERT(r._M_value.sint == -1);
    if (!BS_SUCCEEDED(r = BitInputStreamReadSInt(bis, 5)))
        goto failure;
    TEST_ASSERT(r._M_value.sint == 2);
    if (!BS_SUCCEEDED(r = BitInputStreamReadSInt(bis, 5)))
        goto failure;
    TEST_ASSERT(r._M_value.sint == -2);
    if (!BS_SUCCEEDED(r = BitInputStreamReadUInt(bis, 5)))
        goto failure;
    TEST_ASSERT(r._M_value.uint == 3);
    if (!BS_SUCCEEDED(r = BitInputStreamReadUInt(bis, 5)))
        goto failure;
    TEST_ASSERT(r._M_value.uint == (((uint64_t) -3) & 0x1f));
    goto success;
exit:
    return rc;
failure:
    rc = EXIT_FAILURE;
    goto cleanup;
success:
    rc = EXIT_SUCCESS;
    goto cleanup;
cleanup:
    BitInputStreamSeekBits(bis, bpos, SEEK_SET);
    goto exit;
}

#define PERF_TEST(RUNNER, N, PARAM) perf_test(#RUNNER, &RUNNER, N, PARAM)

int main(int argc, char* *argv) {
    int rc = 0;
    size_t const N = 0x1ffffff;

    unsigned char buf[0xff];
    size_t const buflen = sizeof(buf);

    BitOutputStream bos = {0};
    BitInputStream bis = {0};

    BitOutputStreamInitialize(&bos, &buf[0], buflen * 8);

    if ((rc = PERF_TEST(test_output_perf, N, &bos)) == EXIT_FAILURE)
        goto failure;
    BitInputStreamInitialize(&bis, &buf[0], (5 * 6 + 8 - 1) / 8 * 8);
    printf("binary: ");
    printBinary(&bis);
    printf("\n");
    if ((rc = PERF_TEST(test_input_perf, N, &bis)) == EXIT_FAILURE)
        goto failure;
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
