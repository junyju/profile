/*
 * Alexander Zsikla (azsikl01)
 * Partner: Ann Marie Burke (aburke04)
 * um_driver.c
 * COMP40 HW6
 * Fall 2019
 *
 * Driver file for UM Implementation.
 * The driver opens the provided .um file,
 * creates a UM_T struct,
 * reads in all instructions from given file,
 * and populates segment zero with all instructions.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <sys/stat.h>

#include "um.h"
#include "bitpack.h"

void populate_seg_zero(UM_T um, FILE *fp, uint32_t size);
uint32_t construct_word(FILE *fp);

int main(int argc, char *argv[]) 
{
    if (argc != 2) {
        fprintf(stderr, "Usage: ./um <Um file>\n");
        return EXIT_FAILURE;
    }

    FILE *fp = fopen(argv[1], "r");
    assert(fp != NULL);

    struct stat file_info;
    stat(argv[1], &file_info);
    uint32_t size = file_info.st_size / 4;

    UM_T um = um_new(size);
    assert(um != NULL);

    populate_seg_zero(um, fp, size);

    fclose(fp);
    um_execute(um);

    um_free(&um);

    return EXIT_SUCCESS;
}

/* Name: populate_seg_zero
 * Input: A UM_t struct, a file pointer, and a size
 * Output: N/A
 * Does: Populates segment zero with words from the file
 * Error: Asserts if UM_T struct is NULL
 *        Asserts if fp does not exist
 */
void populate_seg_zero(UM_T um, FILE *fp, uint32_t size)
{
    assert(um != NULL);
    assert(fp != NULL);

    for (uint32_t index = 0; index < size; ++index) {
        uint32_t word = construct_word(fp);
        populate(um, index, word);
    }
}

/* Name: construct_word
 * Input: a file pointer
 * Output: a uint32_t representing a word
 * Does: grabs 8 bits in big endian order and 
 *       creates a 32 bit word which is returned
 * Error: Asserts if file pointer is NULL
 */
uint32_t construct_word(FILE *fp)
{
    assert(fp != NULL);

    uint32_t word = 0;
    unsigned char c;

    /* Reads in a char and creates word in big endian order */
    for (int c_loop = 0; c_loop < 4; c_loop++) {
        c = getc(fp);
        assert(!feof(fp));

        unsigned lsb = 24 - (8 * c_loop);
        word = Bitpack_newu(word, 8, lsb, (uint64_t)c);
    }

    return word;
}