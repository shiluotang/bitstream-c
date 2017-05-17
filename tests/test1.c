#include <stdlib.h>
#include <stdio.h>

#include "../src/bitstream.h"

void printBOS(BitOutputStream const *bos) {
    ReadResult r;
    BitInputStream bis = {0};

    if (!BitInputStreamInitialize(&bis, BitOutputStreamGetBuffer(bos), BitOutputStreamGetSize(bos) * 8))
        goto cleanup;
    while (BS_SUCCEEDED(r = BitInputStreamReadBit(&bis)))
        printf("%d", (int) r._M_value.uint);
    printf("\n");

exit:
    return;
cleanup:
    BitInputStreamRelease(&bis);
    goto exit;
}

int main(int argc, char* *argv) {
    BitOutputStream bos = {0};

    if (!BitOutputStreamInitialize(&bos, NULL, 0))
        goto cleanup;
    if (!BS_SUCCEEDED(BitOutputStreamWriteUInt(&bos, 64, 3)))
        goto cleanup;
    if (!BS_SUCCEEDED(BitOutputStreamWriteUInt(&bos, 1, 1)))
        goto cleanup;
    if (!BS_SUCCEEDED(BitOutputStreamWriteUInt(&bos, 1, 1)))
        goto cleanup;
    printBOS(&bos);
exit:
    return EXIT_SUCCESS;
cleanup:
    BitOutputStreamRelease(&bos);
    goto exit;
}
