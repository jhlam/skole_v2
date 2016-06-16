#include "core.h"


//This function should be the function that works on the datapath.
unsigned int MyDatapathFunction(unsigned int localBufferIn[BUFFER_SIZE], unsigned int frontierBuffer[TOTAL_NODE], unsigned int localBufferOut[TOTAL_NODE], int row_pos, unsigned int numEle, int globalProbability, unsigned int randomNumber[1], int part ){
	uint16_t new_random;
	//printf("Line %i:\n", row_pos);
	new_random = vector_vector_multiplication(numEle, localBufferIn, frontierBuffer,localBufferOut,row_pos,globalProbability,randomNumber[0],part);
	randomNumber[0]=new_random;
	return 0;
}


//NumBytes is the byte that the stream would need to transfer. In this case, since all the elements are of the type int, we need to multiply(total_node, sizeof(int))
int TopLevelWrapper(unsigned int *in, unsigned int *out, unsigned int *frontier, unsigned int numBytes, unsigned int randomSeed,  int globalPro)
{
#pragma HLS UNROLL
#pragma HLS DEPENDENCE
	// signals to be mapped to the AXI MM slave interface (control/status registers)
	// have a look at the generated AXI lite controller verilog file to see which value lives in which register
#pragma HLS INTERFACE s_axilite port=return bundle=control
#pragma HLS INTERFACE s_axilite port=numBytes bundle=control
#pragma HLS INTERFACE s_axilite port=randomSeed bundle=control
#pragma HLS INTERFACE s_axilite port=globalPro bundle=control
	// signals to be mapped to the AXI MM master interface
#pragma HLS INTERFACE m_axi offset=slave port=in bundle=hostmem depth=16*16
#pragma HLS INTERFACE s_axilite port=in bundle=control

#pragma HLS INTERFACE m_axi offset=slave port=out bundle=hostmem depth=16
#pragma HLS INTERFACE s_axilite port=out bundle=control

#pragma HLS INTERFACE m_axi offset=slave port=frontier bundle=hostmem depth=16
#pragma HLS INTERFACE s_axilite port=frontier bundle=control

#pragma HLS PIPELINE
	// on-chip buffers for the input and output data
	// HLS will probably map these to block RAM
	unsigned int localBufferIn[BUFFER_SIZE], localBufferOut[TOTAL_NODE], frontierBuffer[TOTAL_NODE],TempResultBuffer[TOTAL_NODE], randomNumber[1];


	//------Set your own variables, above is the same as toy-core examples
	//set the global probability
	int globalProbability=(globalPro*655);
	unsigned int part;
	unsigned int processedElement;
	unsigned int activEle;
	int i,idx;
	unsigned int converged;
	unsigned int returnNumByte;
	returnNumByte=TOTAL_NODE*(sizeof(unsigned int));
	int level=0;
	part=0;
	converged=0;
	processedElement=part*BUFFER_SIZE;

	//The random number, this way we get to keep it.
	randomNumber[0]=randomSeed;


		// DMA transaction to copy frontier into the buffer from in ptr
		memcpy(frontierBuffer, frontier, returnNumByte);

		for(idx=0;idx<TOTAL_NODE;idx++){
			TempResultBuffer[idx]=frontierBuffer[idx];
			localBufferOut[idx]=0;
		}
		//The bellow is just for 1 run with 1 choosen frontier.

		while(!converged){

			/*printf("Frontier from one run is:");
			for(int j=0;j<TOTAL_NODE; j++){
				printf("%i",frontierBuffer[j]);
			}printf("\n");*/

			//This for-loop is one single SpMV
			while(processedElement<TOTAL_NODE){
				for( i=0; i<TOTAL_NODE; i++){
					memcpy(localBufferIn, in+(i*TOTAL_NODE)+(part*BUFFER_SIZE), numBytes);
				//from this part, you need to be able to take ust small part of the node.
				// DMA transaction to copy buffer from in ptr
					//memcpy(localBufferIn, in+(i*TOTAL_NODE)+(part*BUFFER_SIZE), numBytes);
					// memcpy(localBufferIn,in,numBytes);
					// call your datapath function
					//If the node was activated, no need to continue.

					MyDatapathFunction(localBufferIn, frontierBuffer, TempResultBuffer, i, BUFFER_SIZE,globalProbability,randomNumber, part);

				}

				part++;
				processedElement=part*BUFFER_SIZE	;

		}
			/*printf("TempResultBuffer from one run is:");
					for( j=0;j<TOTAL_NODE; j++){
						printf("%i",TempResultBuffer[j]);
					}printf("\n");
*/

			//printf("Current run:%i\n",level);
			//For the purpose of testing. Shows the vectors along the way.

			converged= distGen(TOTAL_NODE,level,localBufferOut,TempResultBuffer,frontierBuffer);

			//printf("Converged:%i\n",converged);
			//Adds the newly activated nodes into the localBufferOut, which is the final result
			AddVector(TOTAL_NODE,TempResultBuffer,localBufferOut);

		/*	printf("local result from one run is:");
					for( j=0;j<TOTAL_NODE; j++){
						printf("%i",localBufferOut[j]);
					}printf("\n");*/

			part=0;
			processedElement=0;
			level++;
		}



		// copy outputs back to DRAM
			memcpy(out, localBufferOut, returnNumByte);


	return (int) level;
}


uint16_t new_random(uint16_t in_state){
	uint16_t bit;
	uint16_t out_val;

	/* taps: 16 14 13 11; feedback polynomial: x^16 + x^14 + x^13 + x^11 + 1 */
	bit  = ((in_state >> 0) ^ (in_state >> 2) ^ (in_state >> 3) ^ (in_state >> 5) ) & 1;
	out_val =  (in_state >> 1) | (bit << 15);
	//printf("random number:%i \n", out_val);

	return out_val;
}


uint16_t vector_vector_multiplication(int elements, unsigned int *matrix, unsigned int *x_buffer, unsigned int *result, int pos, int global_probability, uint16_t random_seed, int part ){
	uint16_t random_state=random_seed;
	uint16_t pre_rand=new_random(random_state);
	int local_result = 0;
	int i,j,x_res,m_res,global_res;
	global_res  = result[pos];
	for(j=0; j<elements; j++){
		 x_res= x_buffer[j+(part*BUFFER_SIZE)];
		 m_res= matrix[j];
		//Can make this more sparse, might save a lot of computation steps.
		if(x_res && m_res){
			random_state = new_random(pre_rand);
			local_result= random_state<global_probability;
			pre_rand=random_state;
			global_res  = result[pos];
			if(local_result && global_res!=1){
				result[pos] = 1;
			}
					}
	}
	if(global_res!=1 && global_res!=0)
		result[pos]=0;

	return pre_rand;
}

unsigned int distGen(int total_node, int level, unsigned int *x_vector, unsigned int *y_vector, unsigned int *frontier_vector){
	int x_vec,y_vec;
	int update = 0;
	for (int i= 0; i<total_node; i++){
		x_vec = x_vector[i];
		y_vec = y_vector[i];
		if(x_vec == 0 && y_vec == 1){
			//dist[i] = level;
			frontier_vector[i] =1;
			update=update+1;
		}
		else
			frontier_vector[i]=0;
	}
	return update==0;
}

unsigned int AddVector(int total_node,unsigned int *PreviousVector,unsigned int *NewVector){
	int old, new;
	for(int i=0; i<total_node; i++){
		old = PreviousVector[i];
		new =NewVector[i];
		if(old != new){
			NewVector[i]=1;
		}
	}
	return 0;
}

unsigned int writeToMem(unsigned int numBytes, unsigned int part, unsigned int *from, unsigned int *destination){
		//copyMemory
		memcpy( destination+(part*BUFFER_SIZE),from, numBytes);
		return 0;
}


unsigned int readFromMem(unsigned int numBytes,unsigned int part, unsigned int *from, unsigned int *destination){
		//copyMemory
		memcpy(destination, from+(part*BUFFER_SIZE), numBytes);
		return 0;
}

