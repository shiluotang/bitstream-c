#ifndef BITSTREAM_BITSTREAM_H_INCLUDED
#define BITSTREAM_BITSTREAM_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    struct tagBitInputStream;
    typedef struct tagBitInputStream BitInputStream;
    struct tagBitInputStream {
        uint8_t const   *_M_bytes;
        size_t          _M_position;
        size_t          _M_size;

        size_t          _M_marked_position;
    };

    typedef enum tagBSStatus { BS_SUCCESS, BS_FAIL, BS_EOS } BSStatus;

    typedef struct tagReadResult {
        BSStatus _M_status;
        union {
            uint64_t uint;
            int64_t  sint;
        } _M_value;
    } ReadResult;

    typedef struct tagWriteResult {
        BSStatus _M_status;
    } WriteResult;

#define BS_SUCCEEDED(R) ((R)._M_status == BS_SUCCESS)
#define BS_FAILED(R) ((R)._M_status != BS_SUCCESS)

    extern BitInputStream* BitInputStreamInitialize(BitInputStream*, void const*, size_t);
    extern void BitInputStreamRelease(BitInputStream*);

    extern ReadResult BitInputStreamReadBit(BitInputStream*);

    extern void BitInputStreamMark(BitInputStream*);
    extern void BitInputStreamReset(BitInputStream*);

    extern ReadResult BitInputStreamReadInt(BitInputStream*, size_t);
    extern ReadResult BitInputStreamReadUInt(BitInputStream*, size_t);
    extern ReadResult BitInputStreamReadSInt(BitInputStream*, size_t);
    extern ReadResult BitInputStreamReadChar8(BitInputStream*, size_t, char*);
    extern ReadResult BitInputStreamReadUtf8(BitInputStream*, size_t, char*);
    extern void const* BitInputStreamGetBuffer(BitInputStream const*);
    extern size_t BitInputStreamGetBitPosition(BitInputStream const*);
    extern size_t BitInputStreamGetPosition(BitInputStream const*);
    extern int BitInputStreamSkipPaddingBits(BitInputStream*);
    extern int BitInputStreamSeek(BitInputStream*, long, int);
    extern int BitInputStreamSeekBits(BitInputStream*, long, int);

    struct tagBitOutputStream;
    typedef struct tagBitOutputStream BitOutputStream;
    struct tagBitOutputStream {
        uint8_t *_M_bytes;
        size_t _M_position;
        size_t _M_size;

        int _M_fixed;
    };

    extern BitOutputStream* BitOutputStreamInitialize(BitOutputStream*, void*, size_t);
    extern void BitOutputStreamRelease(BitOutputStream*);

    extern WriteResult BitOutputStreamWriteBit(BitOutputStream*, int);
    extern WriteResult BitOutputStreamWriteInt(BitOutputStream*, size_t, int64_t);
    extern WriteResult BitOutputStreamWriteUInt(BitOutputStream*, size_t, uint64_t);
    extern WriteResult BitOutputStreamWriteSInt(BitOutputStream*, size_t, int64_t);
    extern WriteResult BitOutputStreamWriteChar8(BitOutputStream*, size_t, char const*);
    extern WriteResult BitOutputStreamWriteUtf8(BitOutputStream*, size_t, char const*);
    extern void const* BitOutputStreamGetBuffer(BitOutputStream const*);
    extern size_t BitOutputStreamGetSize(BitOutputStream const*);
    extern size_t BitOutputStreamGetBitSize(BitOutputStream const*);
    extern void BitOutputStreamReset(BitOutputStream*);
    extern int BitOutputStreamSeek(BitOutputStream*, long, int);
    extern int BitOutputStreamSeekBits(BitOutputStream*, long, int);
    extern size_t BitOutputStreamPaddingBits(BitOutputStream*, int);
    extern size_t BitOutputStreamGetCapacity(BitOutputStream const*);

#ifdef __cplusplus
}
#endif

#endif /*BITSTREAM_BITSTREAM_H_INCLUDED*/
