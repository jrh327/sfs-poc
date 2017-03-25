#ifndef STDINT
#define STDINT 1
#include <stdint.h>
#endif

// https://www.ibm.com/developerworks/aix/library/au-endianc/
const int i = 1;
#define is_bigendian() ( (*(char*)&i) == 0 )

uint16_t read_uint16(FILE* fp) {
    uint16_t value;
    fread(&value, sizeof value, 1, fp);
    if (is_bigendian()) {
        return value;
    } else {
        uint8_t byte1 = (value & 0xff);
        uint8_t byte2 = (value >> 8) & 0xff;
        return (byte1 << 8) | byte2;
    }
}

void write_uint16(FILE* fp, uint16_t value) {
    if (is_bigendian()) {
        fwrite(&value, sizeof value, 1, fp);
    } else {
        uint8_t byte1 = (value & 0xff);
        uint8_t byte2 = (value >> 8) & 0xff;
        value = (byte1 << 8) | byte2;
        fwrite(&value, sizeof value, 1, fp);
    }
}
