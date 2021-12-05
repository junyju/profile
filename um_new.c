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
#include "assert.h"
#include "seq.h"


typedef struct Memory_T {
    Seq_T segments;
    Seq_T free;
} *Memory_T;

/* Original functions */
uint32_t construct_word(FILE *fp);

/* Helper functions */
Memory_T memory_new(uint32_t length);
void memory_free(Memory_T *m);
void memory_put(Memory_T m, uint32_t seg, uint32_t off, uint32_t val);
uint32_t memory_get(Memory_T m, uint32_t seg, uint32_t off);
uint32_t memory_map(Memory_T m, uint32_t length);
void memory_unmap(Memory_T m, uint32_t seg_num);
uint32_t load_program_new(Memory_T memory, uint32_t *registers, uint32_t ra, uint32_t rb, uint32_t rc);








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

    // UM_T um = um_new(size);
    // assert(um != NULL); 
    // ========================================================================
    Memory_T memory = memory_new(size);
    uint32_t *registers = calloc(8,4);  

    
    // populate_seg_zero(um, fp, size);
    // ========================================================================
    for (uint32_t index = 0; index < size; ++index) {
        uint32_t word = construct_word(fp);
        memory_put(memory, 0, index, word);     // populate(um, index, word);
    }
    fclose(fp);
    

    // um_execute(um);
    // ========================================================================
    uint32_t *seg_zero = Seq_get(memory->segments, 0);
    assert(seg_zero != NULL);

    int seg_zero_len = seg_zero[0];
    int prog_counter = 0;
    uint32_t opcode, word;

    /* Execute words in segment zero until there are none left */
    while (prog_counter < seg_zero_len) {
        uint32_t ra = 0;
        uint32_t rb = 0;
        uint32_t rc = 0;
        uint32_t *A = NULL;
        uint32_t *B = NULL;
        uint32_t *C = NULL;
        word = seg_zero[prog_counter+1];
        opcode = Bitpack_getu(word, 4, 28); 
        printf("opcode: %d\n", opcode);
        prog_counter++;

        /* Load value */
        if (opcode == 13) {
            ra = Bitpack_getu(word, 3, 25);
            uint32_t value = Bitpack_getu(word, 25, 0);
            A = &((registers)[ra]);  
            *A = value;
            continue;
        } 

        if (opcode == 9 || opcode == 10 || opcode == 11) {
            rc = Bitpack_getu(word, 3, 0);
            C = &((registers)[rc]);
        } 
        else if (opcode == 8 || opcode == 12) {
            rb = Bitpack_getu(word, 3, 3);
            rc = Bitpack_getu(word, 3, 0);
            B = &((registers)[rb]);   assert(B != NULL);
            C = &((registers)[rc]);   assert(C != NULL);
        } 
        else {
            ra = Bitpack_getu(word, 3, 6);
            rb = Bitpack_getu(word, 3, 3);
            rc = Bitpack_getu(word, 3, 0);
            A = &((registers)[ra]);   
            B = &((registers)[rb]);  
            C = &((registers)[rc]);   
        }

        switch (opcode) {
            case 0:     //CMOV
                assert(A != NULL);
                assert(B != NULL);
                assert(C != NULL);
                if(*C != 0) { 
                    *A = *B; 
                } 
                break;

            case 1:     //SLOAD
                assert(A != NULL);
                assert(B != NULL);
                assert(C != NULL);
                *A = memory_get(memory, *B, *C);
                break;

            case 2:     //SSTORE
                assert(A != NULL);
                assert(B != NULL);
                assert(C != NULL);
                memory_put(memory, *A, *B, *C);
                break;

            case 3:     //ADD
                assert(A != NULL);
                assert(B != NULL);
                assert(C != NULL);
                *A = (*B + *C) % 4294967296;                
                break;

            case 4:     //MULTIPLY
                assert(A != NULL);
                assert(B != NULL);
                assert(C != NULL);
                *A = ((*B) * (*C)) % 4294967296;          
                break;

            case 5:     //DIVIDE
                assert(A != NULL);
                assert(B != NULL);
                assert(C != NULL);
                assert(*C != 0);
                *A = ((*B) / (*C)) % 4294967296;            
                break;

            case 6:     //NAND
                assert(A != NULL);
                assert(B != NULL);
                assert(C != NULL);
                *A = ~((*B) & (*C));            
                break;

            case 7:     //HALT
                free(registers);   
                memory_free(&memory);
                exit(EXIT_SUCCESS);             
                break;

            case 8:     //MAP
                assert(B != NULL);
                assert(C != NULL);
                uint32_t index = memory_map(memory, *C);
                *B = index;     
                break;

            case 9:     //UNMAP
                assert(C != NULL);
                memory_unmap(memory, *C);   
                break;

            case 10:    //OUTPUT 
                assert(C != NULL);
                assert(*C < 256);
                putchar(*C);           
                break;
                
            case 11:    //INPUT
                assert(C != NULL);
                int character = fgetc(stdin);
                if (character == EOF) {
                    *C = ~0;
                }
                *C = character;              
                break;

            case 12:
                prog_counter = load_program_new(memory, registers, ra, rb, rc);
                seg_zero = Seq_get(memory->segments, 0);
                assert(seg_zero != NULL);
                seg_zero_len = seg_zero[0];
                break;

            default: fprintf(stderr, "Invalid instruction: opcode %d\n", opcode);
                    exit(EXIT_FAILURE); break;
        }
    }




    // ========================================================================
    free(registers);   
    memory_free(&memory);
    return EXIT_SUCCESS;
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
    uint32_t word = 0, c = 0;

    /* Reads in a char and creates word in big endian order */
    for (int c_loop = 0; c_loop < 4; c_loop++) {
        c = getc(fp);
        assert(!feof(fp));

        unsigned lsb = 24 - (8 * c_loop);
        word = Bitpack_newu(word, 8, lsb, (uint64_t)c);
    }
    return word;
}









/* Name: memory_new
 * Input: a uint32_t representing the length of segment zero
 * Output: A newly allocated Memory_T struct
 * Does: * Creates a Sequence of UArray_T's, sets all segments to NULL besides
 *         segment 0, and segment 0 is initialized to be "length" long
         * Creates a Sequence of uint32_t pointers and 
           sets them to be the index of the unmapped segments
 * Error: Asserts if memory is not allocated
 */
Memory_T memory_new(uint32_t length)
{
    Memory_T m_new = malloc(sizeof(*m_new));
    assert(m_new != NULL);

    /* Creating the segments */
    m_new->segments = Seq_new(10);
    assert(m_new->segments != NULL);

    /* Creating the sequence to keep track of free segments */
    m_new->free = Seq_new(10);
    assert(m_new->free != NULL);

    /* Sets all segments to NULL and populates free segment sequence */
    for (int seg_num = 0; seg_num < 10; ++seg_num) {
            Seq_addlo(m_new->segments, NULL);

            uint32_t *temp = malloc(4);
            assert(temp != NULL);

            *temp = seg_num;
            Seq_addhi(m_new->free, temp);
    }

    /* Creating segment zero with proper length*/
    memory_map(m_new, length);
    return m_new;
}

/* Name: memory_free
 * Input: A pointer to a Memory_T struct 
 * Output: N/A 
 * Does: Frees all memory associated with the struct
 * Error: Asserts if struct is NULL
 */
void memory_free(Memory_T *m)
{
    assert(*m != NULL);

    /* Freeing the UArray_T segments */
    int seg_len = Seq_length((*m)->segments);
    for (int seg_num = 0; seg_num < seg_len; ++seg_num) {
        uint32_t *aux = Seq_remhi((*m)->segments);
        /* If the segment is unmapped, there is nothing to free */
        if (aux == NULL) {
            continue;
        } else {
            free(aux);
        }
    }

    /* Freeing the uint32_t pointers */
    int free_len = Seq_length((*m)->free);
    for (int seg_num = 0; seg_num < free_len; ++seg_num) {
        uint32_t *integer = (uint32_t *)Seq_remhi((*m)->free);
        free(integer);
    }

    /* Freeing everything else */
    Seq_free(&(*m)->segments);
    Seq_free(&(*m)->free);
    free(*m);
}

/* Name: memory_put
 * Input: A Memory_T struct, a segment number, an offset, and a value
 * Output: N/A
 * Does: Inserts value at the specificed segment and offset
 * Error: Asserts if struct is NULL
 *        Asserts if segment is not mapped
 *        Asserts if offset is not mapped
 */
void memory_put(Memory_T m, uint32_t seg, uint32_t off, uint32_t val)
{
    assert(m != NULL);
    assert(seg < (uint32_t)Seq_length(m->segments));

    uint32_t *queried_segment = Seq_get(m->segments, seg);
    assert(queried_segment != NULL);
    assert(off < queried_segment[0]);
    /* To account for extra space to store size, add 1 to offset */
    queried_segment[off+1] = val;
}

/* Name: memory_get
 * Input: A Memory_T struct, a segment number, and an offset
 * Output: A uint32_t which represents the value at that segment and offset
 * Does: Gets the value at the specified segment number and offset and returns
 * Error: Asserts if struct is NULL
 *        Asserts if segment is not mapped
 *        Asserts if offset is not mapped
 */
uint32_t memory_get(Memory_T m, uint32_t seg, uint32_t off)
{
    assert(m != NULL);
    // assert(seg < (uint32_t)Seq_length(m->segments));
    printf("seg: %d       segment_len: %d\n", seg, (uint32_t)Seq_length(m->segments));
    uint32_t *queried_segment = Seq_get(m->segments, seg);
    assert(queried_segment != NULL);
    printf("off: %d       queried_len: %d\n", off, queried_segment[0]);;
    // assert(off < queried_segment[0]);       
    return ((queried_segment)[off+1]);
}

/* Name: memory_map
 * Input: A Memory_T struct, a segment number, and segment length
 * Output: the index of the mapped segment
 * Does: Creates a segment that is "length" long 
 *       with all of the segment's values being zero and 
 *       returns index of the mapped segment
 * Error: Asserts if struct is NULL
 *        Asserts if memory for segment is not allocated
 */
uint32_t memory_map(Memory_T m, uint32_t length)
{
    assert(m != NULL);

    /* Setting values in new segment to 0 */
    uint32_t *seg = calloc(length + 1, 4);
    assert(seg != NULL);
    seg[0] = length;        /* Set 0th index as size of the segment */

    /* Mapping a segment */
    uint32_t index = Seq_length(m->segments);
    if (Seq_length(m->free) == 0) {
        /* If there are no free segments, 
            put UArray_T at end of sequence */
        Seq_addhi(m->segments, seg);
    } else {
        /* If there is a free segment, 
            get the index and put the UArray_T at that index */
        uint32_t *free_seg_num = (uint32_t *)Seq_remlo(m->free);
        index = *free_seg_num;
        free(free_seg_num);
        Seq_put(m->segments, index, seg);
    }
    return index;
}

/* Name: memory_unmap
 * Input: A Memory_T struct and a segment number
 * Output: N/A
 * Does: Unmaps a specified segment at the "seg_num" index and frees memory
 *       of the associated segment as well as adds that index back into the
 *       free segment sequence
 * Error: Asserts if struct is NULL
 *        Asserts if unmap segment 0
 *        Asserts if segment is NULL
 */
void memory_unmap(Memory_T m, uint32_t seg_num)
{
    assert(m != NULL);
    assert(seg_num != 0);

    uint32_t *unmap = Seq_get(m->segments, seg_num);
    assert(unmap != NULL);
    free(unmap);

    uint32_t *free_seg = malloc(4);
    assert(free_seg != NULL);
    *free_seg = seg_num;
    Seq_addhi(m->free, free_seg);

    Seq_put(m->segments, seg_num, NULL);
}




/* Name: load_program_new
 * Input: UM_T struct; uint32_t's reprsenting registers A, B, C
 * Output: a uint32_t representing the index that program should start at
 * Does: copies segment[rb value] into segment zero and returns rc value
 * Error: Asserts UM_T struct is NULL
 *        Asserts if any register number is valid
 */
uint32_t load_program_new(Memory_T memory, uint32_t *registers, 
                      uint32_t ra, uint32_t rb, uint32_t rc)
{
    assert(memory != NULL);
    assert(registers != NULL);
    assert(ra < 8 && rb < 8 && rc < 8);

    uint32_t *B = &((registers)[rb]);   assert(B != NULL);
    uint32_t *C = &((registers)[rc]);   assert(C != NULL);

    /* If rb value is 0, 0 is already loaded into segment 0 */
    if (*B == 0) {
        return *C;
    }

    /* Get the segment to copy */
    uint32_t *to_copy = Seq_get(memory->segments, *B);
    assert(to_copy != NULL);

    /* Creating a copy with the same specifications */
    int seg_len = to_copy[0];
    uint32_t *copy = calloc(seg_len+1, 4);
    assert(copy != NULL);

    /* Deep copying */
    for (int i = 0; i < seg_len+1; i++) {
        copy[i] = to_copy[i];
    }

    /* Freeing segment 0 and inserting the copy */
    uint32_t *seg_zero = Seq_get(memory->segments, 0);
    free(seg_zero);
    Seq_put(memory->segments, 0, copy);

    return (*C+1);
}