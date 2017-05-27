#include "bitstream.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

BitInputStream* BitInputStreamInitialize(BitInputStream *const bis, void const *bytes, size_t bits) {
    if (!bis)
        return bis;
    bis->_M_bytes = (uint8_t const*) bytes;
    bis->_M_size = bits;
    bis->_M_position = 0;

    bis->_M_marked_position = 0;
    return bis;
}

void BitInputStreamRelease(BitInputStream *bis) {
    if (bis) {
        bis->_M_bytes = NULL;
        bis->_M_position = 0;
        bis->_M_size = 0;

        bis->_M_marked_position = 0;
    }
}

int const READ_ONE_BIT_SHIFT_WIDTH[] = {
    7, 6, 5, 4, 3, 2, 1, 0
};

ReadResult BitInputStreamReadBit(BitInputStream *const bis) {
    ReadResult result;
    result._M_status = BS_SUCCESS;
    if (bis->_M_position >= bis->_M_size) {
        result._M_status = BS_EOS;
        return result;
    }
    result._M_value.uint = (bis->_M_bytes[bis->_M_position >> 3] >> READ_ONE_BIT_SHIFT_WIDTH[bis->_M_position & 0x7]) & 0x1;
    ++bis->_M_position;
    return result;
}

void BitInputStreamMark(BitInputStream *const bis) {
    bis->_M_marked_position = bis->_M_position;
}

void BitInputStreamReset(BitInputStream *const bis) {
    bis->_M_position = bis->_M_marked_position;
}

ReadResult BitInputStreamReadInt(BitInputStream *bis, size_t bits) {
    ReadResult result;
    ReadResult r;

    result._M_status = BS_SUCCESS;
    result._M_value.sint = 0;

    if (!BS_SUCCEEDED(r = BitInputStreamReadBit(bis)))
        return r;
    if (r._M_value.uint == 1) {
        /* negative. */
        result._M_value.sint = -1;
        result._M_value.sint <<= (bits - 1);
    }
    if (!BS_SUCCEEDED(r = BitInputStreamReadUInt(bis, bits - 1)))
        return r;
    result._M_value.sint |= r._M_value.uint;
    return result;
}

static int const READ_LSB_BITS_OF_ONE_BYTE[] = {
    0,
    0x1,
    (0x1 << 1) | 0x1,
    (0x1 << 2) | (0x1 << 1) | 0x1,
    (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | 0x1,
    (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | 0x1,
    (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | 0x1,
    (0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | 0x1,
};

ReadResult BitInputStreamReadUInt(BitInputStream *bis, size_t bits) {
    ReadResult result;
    int pos = bis->_M_position;
    int w = 0;

    result._M_status = BS_SUCCESS;
    if (pos + bits > bis->_M_size) {
        result._M_status = BS_EOS;
        return result;
    }
    result._M_value.uint = 0;

    while (bits > 0) {
        /* at the byte first bit. */
        if ((pos & 0x7) == 0) {
            while (bits >= 8) {
                /* more than one byte width (8 bits) */
                result._M_value.uint <<= 8;
                result._M_value.uint |= bis->_M_bytes[pos >> 3];
                pos += 8;
                bits -= 8;
            }
            if (bits > 0) {
                /* less than 8 bits width to read. */
                result._M_value.uint <<= bits;
                result._M_value.uint |= (bis->_M_bytes[pos >> 3] >> (8 - bits)) & READ_LSB_BITS_OF_ONE_BYTE[bits];
                pos += bits;
                bits = 0;
                break;
            }
        } else {
            /* next start bit position. */
            w = ((pos >> 3) + 1) << 3;
            /* not from the first bit of byte. */
            if (pos + bits >= w) {
                /* at least to read the remaining bits in one byte. */
                w -= pos;
                result._M_value.uint <<= w;
                result._M_value.uint |= bis->_M_bytes[pos >> 3] & READ_LSB_BITS_OF_ONE_BYTE[w];
                pos += w;
                bits -= w;
            } else {
                /* less than one byte. */
                w -= pos + bits;
                result._M_value.uint <<= bits;
                result._M_value.uint |= (bis->_M_bytes[pos >> 3] >> w) & READ_LSB_BITS_OF_ONE_BYTE[bits];
                pos += bits;
                bits = 0;
                break;
            }
        }
    }

    bis->_M_position = pos;
    return result;
}

ReadResult BitInputStreamReadSInt(BitInputStream *bis, size_t bits) {
    ReadResult result;
    ReadResult r;
    int sign = 0;

    result._M_status = BS_SUCCESS;
    result._M_value.sint = 0;

    if (!BS_SUCCEEDED(r = BitInputStreamReadBit(bis)))
        return r;
    sign = r._M_value.uint;
    if (!BS_SUCCEEDED(r = BitInputStreamReadUInt(bis, bits - 1)))
        return r;
    result._M_value.sint |= r._M_value.uint;
    if (sign)
        result._M_value.sint *= -1;
    return result;
}

ReadResult BitInputStreamReadChar8(BitInputStream *const bis, size_t nbytes, char *char8String) {
    ReadResult r;
    size_t i;

    r._M_status = BS_SUCCESS;
    for (i = 0; i < nbytes; ++i) {
        if (!BS_SUCCEEDED(r = BitInputStreamReadUInt(bis, 8)))
            return r;
        *char8String++ = r._M_value.uint;
    }
    return r;
}

ReadResult BitInputStreamReadUtf8(BitInputStream *const bis, size_t nbytes, char *utf8String) {
    return BitInputStreamReadChar8(bis, nbytes, utf8String);
}

void const* BitInputStreamGetBuffer(BitInputStream const *const bis) {
    return bis->_M_bytes;
}

size_t BitInputStreamGetBitPosition(BitInputStream const *bis) {
    return bis->_M_position;
}

size_t BitInputStreamGetPosition(BitInputStream const *bis) {
    return BitInputStreamGetBitPosition(bis) >> 3;
}

int BitInputStreamSkipPaddingBits(BitInputStream *const bis) {
    size_t bits = 0;
    if ((bis->_M_position & 0x7) == 0)
        return bits;
    bits = 8 - (bis->_M_position & 0x7);
    bis->_M_position += bits;
    return bits;
}

int BitInputStreamSeekBits(BitInputStream *const bis, long offset, int origin) {
    switch (origin) {
        case SEEK_SET: break;
        case SEEK_CUR: offset += bis->_M_position; break;
        case SEEK_END: offset += bis->_M_size; break;
        default: return -1;
    }
    if (offset < 0 || offset > bis->_M_size)
        return -1;
    bis->_M_position = offset;
    return 0;
}

int BitInputStreamSeek(BitInputStream *const bis, long offset, int origin) {
    return BitInputStreamSeekBits(bis, offset * 8, origin);
}

BitOutputStream* BitOutputStreamInitialize(BitOutputStream *bos, void *mem, size_t size) {
    if (!bos)
        return bos;
    if (mem) {
        bos->_M_fixed = 1;
        bos->_M_bytes = (uint8_t*) mem;
    } else {
        size = 16 * 8;
        bos->_M_fixed = 0;
        /* initial capacity */
        bos->_M_bytes = (uint8_t*) malloc(size);
        if (!bos->_M_bytes) {
            BitOutputStreamRelease(bos);
            return NULL;
        }
    }
    bos->_M_size = size;
    bos->_M_position = 0;
    return bos;
}

void BitOutputStreamRelease(BitOutputStream *bos) {
    if (bos) {
        if (!bos->_M_fixed) {
            if (bos->_M_bytes)
                free(bos->_M_bytes);
        }
        bos->_M_bytes = NULL;
        bos->_M_position = 0;
        bos->_M_size = 0;
        bos->_M_fixed = 0;
    }
}

int SET_BIT_ONE_MASKS[] = {
    0x1 << 7, 0x1 << 6, 0x1 << 5, 0x1 << 4,
    0x1 << 3, 0x1 << 2, 0x1 << 1, 0x1 << 0
};
int SET_BIT_ZERO_MASKS[] = {
    ~(0x1 << 7), ~(0x1 << 6), ~(0x1 << 5), ~(0x1 << 4),
    ~(0x1 << 3), ~(0x1 << 2), ~(0x1 << 1), ~(0x1 << 0),
};

WriteResult BitOutputStreamWriteBit(BitOutputStream *bos, int bit) {
    WriteResult result = { BS_SUCCESS };
    void *p = NULL;

    if (bos->_M_position >= bos->_M_size) {
        /* no enough space */
        if (bos->_M_fixed) {
            result._M_status = BS_FAIL;
            return result;
        }
        /**
         * As initial size is 16 bytes, value is of uint64_t (8 bytes),
         * and we never reduce memory size, so twice the size definitely
         * make it large enough for write uint64_t value.
         */
        /* expand memory. */
        p = realloc(bos->_M_bytes, bos->_M_size >> 3);
        if (!p) {
            result._M_status = BS_FAIL;
            return result;
        }
        bos->_M_bytes = p;
        bos->_M_size <<= 1;
    }
    if (bit & 0x1) {
        bos->_M_bytes[bos->_M_position >> 3] |= SET_BIT_ONE_MASKS[bos->_M_position & 0x7];
    } else {
        bos->_M_bytes[bos->_M_position >> 3] &= SET_BIT_ZERO_MASKS[bos->_M_position & 0x7];
    }
    ++bos->_M_position;
    return result;
}

WriteResult BitOutputStreamWriteUInt(BitOutputStream *bos, size_t bits, uint64_t value) {
    WriteResult result = { BS_SUCCESS };
    void *p = NULL;
    int first_bit_of_next_byte;
    int preserve;
    int rbits;
    /* byte position */
    int bpos;
    /* bit position */
    int pos = bos->_M_position;

    if (pos + bits > bos->_M_size) {
        /* no enough space */
        if (bos->_M_fixed) {
            result._M_status = BS_FAIL;
            return result;
        }
        /**
         * As initial size is 16 bytes, value is of uint64_t (8 bytes),
         * and we never reduce memory size, so twice the size definitely
         * make it large enough for write uint64_t value.
         */
        /* expand memory. */
        p = realloc(bos->_M_bytes, bos->_M_size >> 2);
        if (!p) {
            result._M_status = BS_FAIL;
            return result;
        }
        bos->_M_bytes = p;
        bos->_M_size <<= 1;
    }

    /**
     * Now, memory size is large enough, no need to check out of
     * boundary every time after write.
     */
    while (bits > 0) {
        if (!(pos & 0x7)) {
            /**
             * We start writing to the first leading bit.
             */
            while (bits >= 8) {
                bits -= 8;
                bos->_M_bytes[pos >> 3] = (value >> bits) & 0xff;
                pos += 8;
            }
            /**
             * Writing to the first leading bit(MSB), but not the whole byte.
             */
            if (bits > 0) {
                bpos = pos >> 3;
                preserve = bos->_M_bytes[bpos] & READ_LSB_BITS_OF_ONE_BYTE[8 - bits];
                bos->_M_bytes[bpos] = (value & READ_LSB_BITS_OF_ONE_BYTE[bits]) << (8 - bits);
                bos->_M_bytes[bpos] |= preserve;
                pos += bits;
                bits = 0;
                break;
            }
        } else {
            /**
             * Next byte first bit position
             */
            first_bit_of_next_byte = ((pos >> 3) + 1) << 3;
            if (pos + bits >= first_bit_of_next_byte) {
                rbits = first_bit_of_next_byte - pos;
                /**
                 * At least write to the last padding bits(LSB) in one byte.
                 */
                bpos = pos >> 3;
                preserve = bos->_M_bytes[bpos] & (~READ_LSB_BITS_OF_ONE_BYTE[rbits]) & 0xff;
                bits -= rbits;
                bos->_M_bytes[bpos] = (value >> bits) & READ_LSB_BITS_OF_ONE_BYTE[rbits];
                bos->_M_bytes[bpos] |= preserve;
                pos += rbits;
            } else {
                rbits = first_bit_of_next_byte - (pos + bits);
                /**
                 * Write less than one byte which does not start from the first leading bit(MSB).
                 */
                bpos = pos >> 3;
                preserve = bos->_M_bytes[bpos] & ~(READ_LSB_BITS_OF_ONE_BYTE[bits] << rbits) & 0xff;
                bos->_M_bytes[bpos] = (value & READ_LSB_BITS_OF_ONE_BYTE[bits]) << rbits;
                bos->_M_bytes[bpos] |= preserve;
                pos += bits;
                bits = 0;
                break;
            }
        }
    }

    bos->_M_position = pos;
    return result;
}

WriteResult BitOutputStreamWriteInt(BitOutputStream *bos, size_t bits, int64_t value) {
    return BitOutputStreamWriteUInt(bos, bits, value);
}

WriteResult BitOutputStreamWriteSInt(BitOutputStream *bos, size_t bits, int64_t value) {
    WriteResult result = { BS_SUCCESS };

    if (!BS_SUCCEEDED(result = BitOutputStreamWriteBit(bos, value < 0 ? 1 : 0)))
        return result;
    value = value < 0 ? -value : value;
    return BitOutputStreamWriteUInt(bos, bits - 1, value);
}

WriteResult BitOutputStreamWriteChar8(BitOutputStream *const bos, size_t nbytes, char const *char8String) {
    size_t i;
    WriteResult r;
    r._M_status = BS_SUCCESS;
    for (i = 0; i < nbytes; ++i)
        if (!BS_SUCCEEDED(r = BitOutputStreamWriteUInt(bos, 8, (*char8String++) & 0xff)))
            return r;
    return r;
}

WriteResult BitOutputStreamWriteUtf8(BitOutputStream *const bos, size_t nbytes, char const *utf8String) {
    return BitOutputStreamWriteChar8(bos, nbytes, utf8String);
}

void const* BitOutputStreamGetBuffer(BitOutputStream const* bos) {
    return bos->_M_bytes;
}

size_t BitOutputStreamGetBitSize(BitOutputStream const* bos) {
    return bos->_M_position;
}

size_t BitOutputStreamGetSize(BitOutputStream const* bos) {
    return (BitOutputStreamGetBitSize(bos) + 8 - 1) / 8;
}

void BitOutputStreamReset(BitOutputStream *bos) {
    bos->_M_position = 0;
}

int BitOutputStreamSeekBits(BitOutputStream *const bos, long offset, int origin) {
    switch (origin) {
        case SEEK_SET: break;
        case SEEK_CUR: offset += bos->_M_position; break;
        case SEEK_END: offset += bos->_M_size; break;
    }
    if (offset < 0 || offset > bos->_M_size)
        return -1;
    bos->_M_position = offset;
    return 0;
}

int BitOutputStreamSeek(BitOutputStream *const bos, long offset, int origin) {
    return BitOutputStreamSeekBits(bos, offset * 8, origin);
}

size_t BitOutputStreamPaddingBits(BitOutputStream *const bos, int bit) {
    size_t bits;
    size_t i;
    bits = (8 - (bos->_M_position & 0x7)) & 0x7;
    for (i = 0; i < bits; ++i)
        BitOutputStreamWriteBit(bos, bit);
    return bits;
}

size_t BitOutputStreamGetCapacity(BitOutputStream const *bos) {
    return bos->_M_size >> 3;
}
