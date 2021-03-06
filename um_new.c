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
uint32_t memory_map(Memory_T m, uint32_t length);
void memory_unmap(Memory_T m, uint32_t seg_num);
uint32_t load_program_new(Memory_T memory, uint32_t *registers, uint32_t ra, uint32_t rb, uint32_t rc);
// void memory_put(Memory_T m, uint32_t seg, uint32_t off, uint32_t val);

int main(int argc, char *argv[]) 
{
    if (argc != 2) {
        fprintf(stderr, "Usage: ./um <Um file>\n");
        return EXIT_FAILURE;
    }

    FILE *fp = fopen(argv[1], "r");
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
        // memory_put(memory, 0, index, word);     // populate(um, index, word);
        uint32_t *queried_segment = Seq_get(memory->segments, 0);
        queried_segment[index+1] = word;
    }
    fclose(fp);
    

    // um_execute(um);
    // ========================================================================
    uint32_t *seg_zero = Seq_get(memory->segments, 0);

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
        
        opcode = ((uint64_t) word << 32) >> 60;
        prog_counter++;

        /* Load value */
        if (opcode == 13) {
            ra = ((uint64_t) word << 36) >> 61;
            uint32_t value = ((uint64_t) word << 39) >> 39;
            A = &((registers)[ra]);  
            *A = value;
            continue;
        } 

        if (opcode == 9 || opcode == 10 || opcode == 11) {
            rc = ((uint64_t) word << 61) >> 61;
            C = &((registers)[rc]);
        } 
        else if (opcode == 8 || opcode == 12) {
            rb = ((uint64_t) word << 58) >> 61;
            rc = ((uint64_t) word << 61) >> 61;
            B = &((registers)[rb]);   assert(B != NULL);
            C = &((registers)[rc]);   assert(C != NULL);
        } 
        else {
            ra = ((uint64_t) word << 55) >> 61;
            rb = ((uint64_t) word << 58) >> 61;
            rc = ((uint64_t) word << 61) >> 61;
            A = &((registers)[ra]);   
            B = &((registers)[rb]);  
            C = &((registers)[rc]);   
        }

        switch (opcode) {
            case 0:     //CMOV
                if(*C != 0) { 
                    *A = *B; 
                } 
                break;

            case 1:     //SLOAD
                assert(C != NULL);
                uint32_t *queried_segment = Seq_get(memory->segments, *B);  
                *A = (queried_segment)[*C+1];
                break;

            case 2:     //SSTORE
                assert(B != NULL);
                uint32_t *queried_segment1 = Seq_get(memory->segments, *A);
                queried_segment1[*B+1] = *C;
                break;

            case 3:     //ADD
                *A = (*B + *C) % 4294967296;                
                break;

            case 4:     //MULTIPLY
                *A = ((*B) * (*C)) % 4294967296;          
                break;

            case 5:     //DIVIDE
                assert(*C != 0);
                *A = ((*B) / (*C)) % 4294967296;            
                break;

            case 6:     //NAND
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
                memory_unmap(memory, *C);   
                break;

            case 10:    //OUTPUT 
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
                seg_zero_len = seg_zero[0];
                break;

            default: fprintf(stderr, "Invalid instruction: opcode %d\n", opcode);
                    exit(EXIT_FAILURE); break;
        }
    }

    // um_free(&um);
    // ========================================================================
    free(registers);   
    memory_free(&memory);
    return EXIT_SUCCESS;
}



uint32_t construct_word(FILE *fp)
{
    uint32_t word = 0, c = 0;

    /* Reads in a char and creates word in big endian order */
    for (int c_loop = 0; c_loop < 4; c_loop++) {
        c = getc(fp);

        unsigned lsb = 24 - (8 * c_loop);
        unsigned hi = 8 + lsb;
        word = (((uint64_t) word << hi) >> hi) | (((uint64_t) word << (64 - lsb)) >> (64 - lsb)) | (c << lsb);
    }
    return word;
}



Memory_T memory_new(uint32_t length)
{
    Memory_T m_new = malloc(sizeof(*m_new));

    m_new->segments = Seq_new(10);
    m_new->free = Seq_new(10);

    for (int seg_num = 0; seg_num < 10; ++seg_num) {
            Seq_addlo(m_new->segments, NULL);

            uint32_t *temp = malloc(4);

            *temp = seg_num;
            Seq_addhi(m_new->free, temp);
    }

    memory_map(m_new, length);
    return m_new;
}


void memory_free(Memory_T *m)
{
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

// void memory_put(Memory_T m, uint32_t seg, uint32_t off, uint32_t val)
// {
//         uint32_t *queried_segment = Seq_get(m->segments, seg);
//         queried_segment[off+1] = val;
// }

uint32_t memory_map(Memory_T m, uint32_t length)
{
    /* Setting values in new segment to 0 */
    uint32_t *seg = calloc(length + 1, 4);
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


void memory_unmap(Memory_T m, uint32_t seg_num)
{
    uint32_t *unmap = Seq_get(m->segments, seg_num);
    free(unmap);

    uint32_t *free_seg = malloc(4);
    // assert(free_seg != NULL);
    *free_seg = seg_num;
    Seq_addhi(m->free, free_seg);

    Seq_put(m->segments, seg_num, NULL);
}


uint32_t load_program_new(Memory_T memory, uint32_t *registers, 
                      uint32_t ra, uint32_t rb, uint32_t rc)
{
    (void) ra;

    uint32_t *B = &((registers)[rb]);   assert(B != NULL);
    uint32_t *C = &((registers)[rc]);   assert(C != NULL);

    /* If rb value is 0, 0 is already loaded into segment 0 */
    if (*B == 0) {
        return *C;
    }

    /* Get the segment to copy */
    uint32_t *to_copy = Seq_get(memory->segments, *B);

    /* Creating a copy with the same specifications */
    int seg_len = to_copy[0];
    uint32_t *copy = calloc(seg_len+1, 4);

    /* Deep copying */
    for (int i = 0; i < seg_len+1; i++) {
        copy[i] = to_copy[i];
    }

    /* Freeing segment 0 and inserting the copy */
    uint32_t *seg_zero = Seq_get(memory->segments, 0);
    free(seg_zero);
    Seq_put(memory->segments, 0, copy);

    return (*C);
}