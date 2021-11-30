/*
 * Alexander Zsikla (azsikl01)
 * Partner: Ann Marie Burke (aburke04)
 * unittest.c
 * COMP40 HW6
 * Fall 2019
 *
 * Testing UM implementation
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "registers.h"
#include "memory.h"
#include "um.h"
#include "uarray.h"
#include "seq.h"

struct Registers_T {
        UArray_T registers;
}; 

struct Memory_T {
        Seq_T segments;
        Seq_T free;
};

struct UM_T {
    Registers_T reg;
    Memory_T mem;
};

bool test_registers();
bool test_memory();
bool test_um();

int main(int argc, char *argv[])
{
        (void)argc;
        (void)argv;

        fprintf(stderr, "\n%s\n", "Starting Register Test");
        if (test_registers()) {
                fprintf(stderr, "Register Test Successful\n\n");
        } else {
                fprintf(stderr, "** Register Test Failed **\n\n");
                exit(EXIT_FAILURE);
        }

        fprintf(stderr, "%s\n", "Starting Memory Test");
        if (test_memory()) {
                fprintf(stderr, "Memory Test Successful\n\n");
        } else {
                fprintf(stderr, "** Memory Test Failed **\n\n");
                exit(EXIT_FAILURE);
        }

        fprintf(stderr, "%s\n", "Starting UM Test");
        if (test_um()) {
                fprintf(stderr, "UM Test Successful\n\n");
        } else {
                fprintf(stderr, "** UM Test Failed **\n\n");
                exit(EXIT_FAILURE);
        }

        return EXIT_SUCCESS;
}

bool test_registers()
{
        bool check = true;

        // Test 1 - new
        Registers_T aux = registers_new();
        if (UArray_length(aux->registers) == 8 && 
            UArray_size(aux->registers) == 4) {
                fprintf(stderr, "%s\n", "Test 1 Passed");
        } else {
                fprintf(stderr, "%s\n", "Test 1 Failed");
                check = false;
        }

        // Test 2 - new 
        // Checking that all values in register is zero
        bool all_zero = true;
        for (int i = 0; i < UArray_length(aux->registers); i++) {
                if (*(uint32_t *)UArray_at(aux->registers, i) != 0) {
                        all_zero = false;
                }
        }

        if (all_zero) {
                fprintf(stderr, "Test 2 Passed\n");
        } else {
                fprintf(stderr, "Test 2 Failed\n");
                check = false;
        }

        // Test 3 - free - checked with valgrind
        registers_free(&aux);
        fprintf(stderr, "Test 3 Passed\n");

        // Test 4 - put
        aux = registers_new();
        registers_put(aux, 4, 59);
        registers_put(aux, 6, 254);
        if (*(uint32_t *)UArray_at(aux->registers, 4) == 59 &&
            *(uint32_t *)UArray_at(aux->registers, 6) == 254) {
                fprintf(stderr, "Test 4 Passed\n");
        } else {
                fprintf(stderr, "Test 4 Failed\n");
                check = false;
        }
        // Testing invalid registers
        //registers_put(aux, 8, 1102);
        //registers_put(aux, -2, 1102);

        // Test 5 - get
        if (registers_get(aux, 4) == 59 &&
            registers_get(aux, 6) == 254 &&
            registers_get(aux, 3) == 0) {
                fprintf(stderr, "Test 5 Passed\n");
        } else {
                fprintf(stderr, "Test 5 Failed\n");
                check = false;
        }

        registers_free(&aux);

        return check;
}

bool test_memory()
{
    bool check = true;

    // Test 1 - basic new
    Memory_T aux = memory_new(30);
    if (Seq_length(aux->segments) == 10 &&
        Seq_get(aux->segments, 1) == NULL &&
        Seq_get(aux->segments, 9) == NULL &&
        Seq_length(aux->free) == 9 &&
        *(int *)Seq_get(aux->free, 0) == 1 &&
        *(int *)Seq_get(aux->free, 8) == 9) {
        fprintf(stderr, "Test 1 Passed\n");
    } else {
        fprintf(stderr, "Test 1 Failed\n");
        check = false;
    }

    // Test 2 - basic free - tested using valgrind
    memory_free(&aux);
    fprintf(stderr, "Test 2 Passed\n");

    // Test 3 - map segment in new 
    // Segment zero is correct length
    aux = memory_new(30);
    if (UArray_length((UArray_T)Seq_get(aux->segments, 0)) == 30) {
        fprintf(stderr, "Test 3 Passed\n");
    } else {
        fprintf(stderr, "Test 3 Failed\n");
        check = false;
    }

    // Test 4 - map segment not in new
    uint32_t index = memory_map(aux, 14);
    UArray_T temp = (UArray_T)Seq_get(aux->segments, index);
    if (UArray_length(temp) == 14 &&
        *(uint32_t *)UArray_at(temp, 5) == 0 &&
        Seq_length(aux->segments) == 10 &&
        index == 1) {
        fprintf(stderr, "Test 4 Passed\n");
    } else {
        fprintf(stderr, "Test 4 Failed\n");
        check = false;
    }

    // Test 4.1 - mapping a segment again
    index = memory_map(aux, 20);
    temp = (UArray_T)Seq_get(aux->segments, index);
    if (UArray_length(temp) == 20 &&
        *(uint32_t *)UArray_at(temp, 5) == 0 &&
        Seq_length(aux->free) == 7 &&
        index == 2) {
        fprintf(stderr, "Test 4.1 Passed\n");
    } else {
        fprintf(stderr, "Test 4.1 Failed\n");
        check = false;
    }

    // Test 5 - unmap segment that was just mapped
    memory_unmap(aux, 1);
    if (Seq_get(aux->segments, 8) == NULL &&
        Seq_length(aux->free) == 8) {
        fprintf(stderr, "Test 5 Passed\n");
    } else {
        fprintf(stderr, "Test 5 Failed\n");
        check = false;
    }

    // Test 5.1 - checking that unmapped segment is in sequence again
    if (*(uint32_t *)Seq_get(aux->free, Seq_length(aux->free) - 1) == 1) {
        fprintf(stderr, "Test 5.1 Passed\n");
    } else {
        fprintf(stderr, "Test 5.1 Failed\n");
    }

    // Test 6 - unmap edge cases - tested with checking assertions
    // memory_unmap(aux, 8);
    // memory_unmap(aux, 0);
    fprintf(stderr, "Test 6 Passed\n");

    // Test 7 - put
    memory_put(aux, 2, 15, 50);
    temp = (UArray_T)Seq_get(aux->segments, 2);
    if (*(uint32_t *)UArray_at(temp, 15) == 50) {
        fprintf(stderr, "Test 7 Passed\n");
    } else {
        fprintf(stderr, "Test 7 Failed\n");
        check = false;
    }

    // Test 7.1 - put into unmapped segment
    // memory_put(aux, 1, 15, 30);
    fprintf(stderr, "Test 7.1 Passed\n");

    // Test 8 - put edge cases - tested with checking assertions
    // memory_put(aux, 1, 0, 100); // Putting in unmapped segment
    // memory_put(aux, 11, 3, 100); // Putting in segment that DNE
    // memory_put(aux, 2, 21, 100); // Putting in offset that DNE
    fprintf(stderr, "Test 8 Passed\n");

    // // Test 9 - get
    if (memory_get(aux, 2, 15) == 50 &&
        memory_get(aux, 2, 4) == 0) {
        fprintf(stderr, "Test 9 Passed\n");
    } else {
        fprintf(stderr, "Test 9 Failed\n");
        check = false;
    }

    // Test 10 - get edge cases - tested with checking assertions
    // memory_get(aux, 1,  0); // Getting in unmapped segment
    // memory_get(aux, 11, 3); // Getting in segment that DNE
    // memory_get(aux, 2, 21); // Getting in offset that DNE
    fprintf(stderr, "Test 10 Passed\n");

    memory_free(&aux);

    return check;
}

bool test_um()
{
    bool check = true;

    // Test 1 - new
    UM_T aux = um_new(30);
    if(Seq_length(aux->mem->segments) == 10 &&
       Seq_get(aux->mem->segments, 1) == NULL &&
       UArray_length(aux->reg->registers) == 8 && 
       UArray_size(aux->reg->registers) == 4 &&
       Seq_length(aux->mem->free) == 9 &&
       *(int *)Seq_get(aux->mem->free, 0) == 1 &&
       *(int *)Seq_get(aux->mem->free, 8) == 9) {
        fprintf(stderr, "Test 1 Passed\n");
    } else {
        fprintf(stderr, "Test 1 Failed\n");
        check = false;
    }

    // Test 2 - free - tested using valgrind
    um_free(&aux);
    fprintf(stderr, "Test 2 Passed\n");

    // Test 3 - halt - tested using valgrind and echo
    aux = um_new(30);
    // halt(aux, 0, 0, 0);
    fprintf(stderr, "Test 3 Passed\n");

    // Test 4 - output - tested by looking at outputs
    registers_put(aux->reg, 4, 84);
    registers_put(aux->reg, 6, 101);
    registers_put(aux->reg, 1, 115);
    registers_put(aux->reg, 5, 116);
    output(aux, 6, 1, 4);
    output(aux, 1, 4, 6);
    output(aux, 4, 6, 1);
    output(aux, 6, 1, 5);
    fprintf(stdout, " 4 Passed\n");

    // Test 5 - load val
    load_value(aux, 4, 345);
    if (registers_get(aux->reg, 4) == 345) {
        fprintf(stderr, "Test 5 Passed\n");
    } else {
        fprintf(stderr, "Test 5 Failed\n");
        check = false;
    }
    
    // Test 6 - add
    registers_put(aux->reg, 2, (1 << 31) + 2);
    registers_put(aux->reg, 3, 1 << 31);
    add(aux, 7, 4, 6);
    add(aux, 6, 2, 3);
    if (registers_get(aux->reg, 7) == 446 &&
        registers_get(aux->reg, 6) == 2) {
        fprintf(stderr, "Test 6 Passed\n");
    } else {
        fprintf(stderr, "Test 6 Failed\n");
        check = false;
    }

    // Test 7 - mult
    registers_put(aux->reg, 1, 4);
    registers_put(aux->reg, 2, 5);
    registers_put(aux->reg, 4, 20);
    registers_put(aux->reg, 5, 50);
    multiply(aux, 3, 1, 2);
    multiply(aux, 6, 4, 5);
    if (registers_get(aux->reg, 3) == 20 &&
        registers_get(aux->reg, 6) == 1000) {
        fprintf(stderr, "Test 7 Passed\n");
    } else {
        fprintf(stderr, "Test 7 Failed\n");
        check = false;
    }

    // Test 8 - divide
    divide(aux, 3, 4, 2);
    divide(aux, 6, 5, 2);
    divide(aux, 7, 1, 2);
    if (registers_get(aux->reg, 3) == 4 &&
        registers_get(aux->reg, 6) == 10 &&
        registers_get(aux->reg, 7) == 0) {
        fprintf(stderr, "Test 8 Passed\n");
    } else {
        fprintf(stderr, "Test 8 Failed\n");
        check = false;
    }

    // Test 9 - NAND
    nand(aux, 3, 1, 2);
    if (registers_get(aux->reg, 3) == 4294967291) {
        fprintf(stderr, "Test 9 Passed\n");
    } else {
        fprintf(stderr, "Test 9 Failed\n");
        check = false;
    }

    // Test 10 - input - tested with printf
    // input(aux, 1, 2, 3);
    // printf("%u\n", registers_get(aux->reg, 3));
    fprintf(stderr, "Test 10 Passed\n");

    // Test 11 - cmove
    registers_put(aux->reg, 0, 0);
    conditional_move(aux, 1, 2, 0);
    conditional_move(aux, 3, 2, 1);
    if (registers_get(aux->reg, 1) == 4 &&
        registers_get(aux->reg, 3) == 5) {
        fprintf(stderr, "Test 11 Passed\n");
    } else {
        fprintf(stderr, "Test 11 Failed\n");
        check = false;
    }

    // // Test 12 - Segmented Store
    int index = memory_map(aux->mem, 20);
    registers_put(aux->reg, 0, 2);
    registers_put(aux->reg, 1, 10);
    registers_put(aux->reg, 2, 5);
    registers_put(aux->reg, 3, 3);
    registers_put(aux->reg, 4, 7);
    registers_put(aux->reg, 5, index);
    registers_put(aux->reg, 6, 4);
    registers_put(aux->reg, 7, 24);
    segmented_store(aux, 5, 1, 0);
    if (memory_get(aux->mem, index, 10) == 2 &&
        memory_get(aux->mem, index, 11) == 0) {
        fprintf(stderr, "Test 12 Passed\n");
    } else {
        fprintf(stderr, "Test 12 Failed\n");
        check = false;
    }

    // Test 13 - Segmented Load
    segmented_load(aux, 6, 5, 1);
    segmented_load(aux, 7, 5, 4);
    if (registers_get(aux->reg, 6) == 2 &&
        registers_get(aux->reg, 7) == 0) {
        fprintf(stderr, "Test 13 Passed\n");
    } else {
        fprintf(stderr, "Test 13 Failed\n");
        check = false;
    }

    // Test 14 - load program
    index = memory_map(aux->mem, 10);
    registers_put(aux->reg, 0, 0);
    registers_put(aux->reg, 1, index);
    registers_put(aux->reg, 2, 3);
    registers_put(aux->reg, 3, 9);
    registers_put(aux->reg, 4, 7);
    registers_put(aux->reg, 5, 19);
    segmented_store(aux, 1, 2, 5);
    segmented_store(aux, 1, 4, 0);
    segmented_store(aux, 1, 0, 4);
    segmented_store(aux, 1, 3, 1);

    int rc_val = load_program(aux, 4, 1, 5);

    if(rc_val == 19 &&
        memory_get(aux->mem, 0, 0) == 7 &&
        memory_get(aux->mem, 0, 3) == 19 &&
        memory_get(aux->mem, 0, 7) == 0 &&
        memory_get(aux->mem, 0, 9) == 2) {
        fprintf(stderr, "Test 14 Passed\n");
    } else {
        fprintf(stderr, "Test 14 Failed\n");
        check = false;
    }

    // // Test 15 - Instruction call
    instruction_call(aux, 3, 7, 3, 4);
    index = memory_map(aux->mem, 50);
    registers_put(aux->reg, 5, index);
    instruction_call(aux, 2, 5, 3, 1);
    registers_put(aux->reg, 6, 48);
    // instruction_call(aux, 10, 0, 1, 6);

    if (registers_get(aux->reg, 7) == 16 &&
        memory_get(aux->mem, index, 9) == 2) {
        fprintf(stderr, "Test 15 Passed\n");
    } else {
        fprintf(stderr, "Test 15 Failed\n");
        check = false;
    }

    um_free(&aux);

    return check;
}