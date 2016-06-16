#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ap_cint.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>

#define MATRIX_SIZE 16*16
#define TOTAL_NODE 16
#define BUFFER_SIZE 16


int TopLevelWrapper(unsigned int *in, unsigned int *out, unsigned int *frontier, unsigned int numBytes, unsigned int randomSeed, int globalProbability);
// unsigned int TopLevelWrapper_2(unsigned int *in, unsigned int *out, unsigned int *frontier, unsigned int numBytes, unsigned int randomSeed,  double globalPro);


void add_vector(int total_node, unsigned int *vector_1,unsigned int *vector_2, unsigned int *result_vector);
uint16_t new_random(uint16_t in_state);
void sub_vector(int total_node, unsigned int *vector_1,unsigned int *vector_2,unsigned int *result_vector);
uint16_t vector_vector_multiplication(int elements, unsigned int *matrix, unsigned int *x_buffer, unsigned int *result, int pos, int global_probability,uint16_t random_seed,int part);
unsigned int distGen(int total_node, int level, unsigned int *x_vector,unsigned int *y_vector, unsigned int *frontier_vector);
unsigned int AddVector(int total_node,unsigned int *PreviousVector,unsigned int *NewVector);
unsigned int readFromMem(unsigned int numBytes,unsigned int part, unsigned int *from, unsigned int *destination);
unsigned int writeToMem(unsigned int numBytes,unsigned int part, unsigned int *from, unsigned int *destination);
int greedySeedSelection(int k);
int ipCore();
