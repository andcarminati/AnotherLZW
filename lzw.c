/*
Another simple implementation of LZW compression algorithm
Copyright (C) 2020  Andreu Carminati
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "lzw.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_ENTRIES 4096
#define MAX_SEQ 50
#define LAST_UNUSED_ENTRY 256

typedef struct __attribute__((__packed__)) entry {
    unsigned char *sequence;
    unsigned short int size : 7;
    unsigned char in_use : 1;

}
entry;

// LZW dictionary entries
static entry entries[MAX_ENTRIES];
static int value_count;

// Buffer for sequences in dictionary (static allocation)
static unsigned char seq_buff[MAX_SEQ_BUFF];
static unsigned char* buff_ptr;
static unsigned char* buff_ptr_end = seq_buff + MAX_SEQ_BUFF;

// ASCII table for the first part of the dicionary
static unsigned char ASCII[256];

static inline int dict_get_code(unsigned char* seq, int size) {
    if (size == 1) {
        //  go direct to value, because it is a string with a single ascii value
        return seq[0];
    }
    for (int i = LAST_UNUSED_ENTRY; i < value_count; i++) {
        entry* elem = &entries[i];
        if (memcmp(elem->sequence, seq, size) == 0) {
            return i;
        }
    }
    return -1;
}

unsigned char* alloc_seq(int size) {
    unsigned char* ptr = buff_ptr;
    buff_ptr += size;
    return ptr;
}

static inline void fill_entry(entry* ent, unsigned char* seq, int size) {
    ent->sequence = alloc_seq(size);
    memcpy(ent->sequence, seq, size);
    ent->size = size;
    ent->in_use = 1;
}

static inline int insert_entry(unsigned char* seq, int size) {
    int code = value_count;
    entry* entry_ = &entries[value_count++];
    fill_entry(entry_, seq, size);
    return code;
}

static inline int exist_entry(short code) {
    entry* ent = &entries[code];
    return ent->in_use;
}

void reset() {
    int i;
    value_count = 0;
    buff_ptr = seq_buff;
    for (i = 0; i < LAST_UNUSED_ENTRY; i++) {
        ASCII[i] = i;
        insert_entry(&ASCII[i], 1);
    }
    memset(entries + LAST_UNUSED_ENTRY, 0, (MAX_ENTRIES - LAST_UNUSED_ENTRY) * sizeof (entry));
}

static inline int dict_contains(unsigned char seq[], int size) {
    int i;
    if (size == 1) {
        return 1;
    }
    for (i = LAST_UNUSED_ENTRY; i < value_count; i++) {
        entry* elem = &entries[i];
        if (elem->size == size && memcmp(elem->sequence, seq, size) == 0) {
            return 1;
        }
    }
    return 0;
}

static inline int dict_get_seq(short code, unsigned char** seq) {
    entry* ent = &entries[code];
    *seq = ent->sequence;
    return ent->size;
}

static inline int dict_is_full(int size) {
    if (value_count < MAX_ENTRIES && (buff_ptr + size < buff_ptr_end)) {
        return 0;
    }
    return 1;
}

static int compressed_write(short code, int size_out, unsigned char* out_data, int* has_remaining,
        unsigned char* rest, int* write_pos) {

    unsigned char part4bits, part8bits;

    if ((out_data + (2 + *write_pos)) >= (out_data + size_out)) {
        printf("#####Out of space in compression buffer (expand compression buffer or sequence buffer)#####\n");
        return 0;
    }

    if (*has_remaining) {
        part4bits = code >> 8;
        part8bits = code & 0x00FF;
        *rest = *rest | part4bits;
        out_data[(*write_pos)++] = *rest;
    } else {
        part8bits = code >> 4;
        part4bits = (code & 0x000F);
        *rest = part4bits << 4;
    }
    out_data[(*write_pos)++] = part8bits;
    *has_remaining = !*has_remaining;
    return 1;
}

/*
 * LZW Encode, transform byte sequences into 12-bit encoded data
 * with a dictionary with 4096 entries
 *  
 */
int encode(int size, unsigned char* in_data, int size_out, unsigned char* out_data) {

    // compression vars
    unsigned char P[MAX_SEQ], C;
    short code;
    int sizeP = 0;
    int i;
    // output vars
    unsigned char rest = 0;
    int has_remaining = 0, write_pos = 0;

    for (i = 0; i < size; i++) {
        C = in_data[i];
        P[sizeP] = C;
        if (dict_contains(P, sizeP + 1)) {
            sizeP++;
        } else {
            code = dict_get_code(P, sizeP);
            if (!compressed_write(code, size_out, out_data, &has_remaining, &rest, &write_pos)) {
                return 0;
            }
            if (!dict_is_full(sizeP + 1)) {
                insert_entry(P, sizeP + 1);
            }
            P[0] = C;
            sizeP = 1;
        }
    }
    code = dict_get_code(P, sizeP);
    if (!compressed_write(code, size_out, out_data, &has_remaining, &rest, &write_pos)) {
        return 0;
    }
    // write last remaining part
    if (has_remaining && rest != 0) {
        if (!compressed_write(0, size_out, out_data, &has_remaining, &rest, &write_pos)) {
            return 0;
        }
    }
    return write_pos;
}

static unsigned short compressed_read(unsigned char* in_data, int* has_remaining,
        unsigned char* remaining, int* read_pos) {
    unsigned short value = 0;
    unsigned char part8bits;

    part8bits = in_data[(*read_pos)++];

    if (*has_remaining) {
        value = (*remaining << 8) | part8bits;
    } else {
        unsigned char part4bits = in_data[(*read_pos)++];
        *remaining = part4bits & 0x0F;
        value = (part8bits << 4) | (part4bits >> 4);
    }
    *has_remaining = !*has_remaining;
    return value;
}

static inline int uncompressed_write(unsigned char* seq, int size, int size_out, unsigned char* out_data, int* pos) {
    if ((out_data + size + *pos) >= (out_data + size_out)) {
        printf("#####Out of space in decompression buffer#####\n");
        return 0;
    }
    memcpy(out_data + (*pos), seq, size);
    *pos += size;
    return 1;
}

/*
 * LZW Decode, using standard 12 bit encoded data
 * with a dictionary with 4096 entries
 *  
 */
int decode(int size, unsigned char* in_data, int size_out, unsigned char* out_data) {

    // decompression vars
    unsigned char P[MAX_SEQ], C;
    unsigned short Cw, Pw;
    int sizeP = 0, sizeCw;
    unsigned char *data, *dataP;
    // input manag. vars
    int data_count = 0, total_data = size, has_remaining = 0;
    unsigned char rest = 0;
    // output control vars
    int out_pos = 0;

    Cw = compressed_read(in_data, &has_remaining, &rest, &data_count);
    sizeCw = dict_get_seq(Cw, &data);
    if (!uncompressed_write(data, sizeCw, size_out, out_data, &out_pos)) {
        return 0;
    }
    while (data_count < total_data) {
        Pw = Cw;
        Cw = compressed_read(in_data, &has_remaining, &rest, &data_count);
        if (exist_entry(Cw)) {
            sizeCw = dict_get_seq(Cw, &data);
            if (!uncompressed_write(data, sizeCw, size_out, out_data, &out_pos)) {
                out_pos = 0;
                break;
            }
            sizeP = dict_get_seq(Pw, &dataP);
            memcpy(P, dataP, sizeP);
            C = data[0];
            P[sizeP] = C;
            if (!dict_is_full(sizeP + 1)) {
                insert_entry(P, sizeP + 1);
            }
        } else {
            sizeP = dict_get_seq(Pw, &dataP);
            memcpy(P, dataP, sizeP);
            C = dataP[0];
            P[sizeP] = C;
            if (!uncompressed_write(P, sizeP + 1, size_out, out_data, &out_pos)) {
                out_pos = 0;
                break;
            }
            if (!dict_is_full(sizeP + 1)) {
                insert_entry(P, sizeP + 1);
            }
        }
    }
    return out_pos;
}

void dumpstat() {

    printf("==================================\n");
    printf("Entries in dictionatry: %d\n", value_count);
    printf("Characters in seq buffer: %d\n", buff_ptr - seq_buff);

    printf("Total of static allocated data: %d Kb\n", (sizeof (entries) + sizeof (seq_buff)) / 1000);
    printf("==================================\n");
}

