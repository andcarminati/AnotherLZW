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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lzw.h"

unsigned char block1[] = "Alice was beginning to get very tired of sitting by her sister on the bank, \n"
        "and of having nothing to do: once or twice she had peeped into the book her sister was reading, \n"
        "but it had no pictures or conversations in 'and what is the use of a book,' thought Alice \n"
        "'without pictures or conversations?\n"

        "\nSo she was considering in her own mind (as well as she could, for the hot day made \n"
        "her feel very sleepy and stupid), whether the pleasure of making a daisy-chain would \n"
        "be worth the trouble of getting up and picking the daisies, when suddenly a White Rabbit \n"
        "with pink eyes ran close by her.\n"

        "\nThere was nothing so VERY remarkable in that; nor did Alice think it so VERY \n"
        "much out of the way to hear the Rabbit say to itself, 'Oh dear! Oh dear! I shall be late!' \n"
        "(when she thought it over afterwards, it occurred to her that she ought to have wondered at \n"
        "this, but at the time it all seemed quite natural); but when the Rabbit actually TOOK A WATCH \n"
        "OUT OF ITS WAISTCOAT-POCKET, and looked at it, and then hurried on, Alice started to her feet, \n"
        "for it flashed across her mind that she had never before seen a rabbit with either a waistcoat-pocket,\n"
        "or a watch to take out of it, and burning with curiosity, she ran across the field after it, and\n"
        "fortunately was just in time to see it pop down a large rabbit-hole under the hedge. \n";

unsigned char comp_block1[1500];
unsigned char uncomp_block1[1500];

int main(int argc, char** argv) {

    int size_block1, i;
    int comp_size_block1;
    int uncomp_size_block1;

    size_block1 = sizeof (block1);

    printf("Original data...:\n");
    printf("Size of block 1: %d\n", size_block1);

    // reset compressor/decompressor
    reset();

    printf("\nCompressing...:\n");

    comp_size_block1 = encode(size_block1, (unsigned char*) block1, sizeof (comp_block1), comp_block1);

    if (comp_size_block1 == 0) {
        return -1;
    }


    printf("Size of block 1 (compressed): %d (reduction of %f)\n",
            comp_size_block1, 1.0 - (comp_size_block1 / (double) size_block1));


    dumpstat();
    // reset compressor/decompressor again
    reset();

    printf("\nUncompressing...:\n");
    uncomp_size_block1 = decode(comp_size_block1, (unsigned char*) comp_block1, sizeof (uncomp_block1), uncomp_block1);

    if (uncomp_size_block1 == 0) {
        return -1;
    }

    printf("Size of block 1 (decompressed): %d\n", uncomp_size_block1);
    printf("\nDumping result...:\n");

    for (i = 0; i < uncomp_size_block1; i++) {
        printf("%c", uncomp_block1[i]);
    }

    printf("\n");
    dumpstat();

    return 0;
    
}