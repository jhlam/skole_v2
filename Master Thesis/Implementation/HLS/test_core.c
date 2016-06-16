#include "core.h"

unsigned int *inputStream;

double currentRun[2];
double BestRun[2] ={0,0};
unsigned int bestResult[TOTAL_NODE];
	unsigned int *outputStream;


	unsigned int *frontierStream;


	unsigned int numBytes =BUFFER_SIZE*(sizeof(unsigned int));

	//Variable that i generate for this core.
	unsigned int activatedElement=0;
	int ch;
	int i;
	int level=0;
	int global_probability = 5;
	int randomSeed=42;
	double globalTime=0;
	double globalCoverage=0;

//The test function, read the matrix and send it to the IP-core.
int main(){
	//Initiate important variable that is essential.(from Yaman)
	inputStream=(unsigned int *)malloc(MATRIX_SIZE*sizeof(int));
	outputStream=(unsigned int *)malloc( TOTAL_NODE*sizeof(int));
	frontierStream=(unsigned int *)malloc( TOTAL_NODE*sizeof(int));
	//Reads file as input
	FILE *file = fopen("matrix_16.txt", "r");
	size_t n = 0;
	int c;

	if (file == NULL)
		return 1; //could not open file

	while ((c = fgetc(file)) != EOF)
	{
		inputStream[n++] =  (c-'0');
	}
	inputStream[0]=0;

	FILE *file_2 = fopen("seed_16.txt", "r");
	n = 0;
	if (file_2 == NULL)
		return 1; //could not open file
	while ((ch = fgetc(file_2)) != EOF)
	{
		frontierStream[n++] =  (ch-'0');
	}

	for(int l = 0; l<TOTAL_NODE; l++){
				frontierStream[l] = bestResult[l];
	}


	time_t start, end;
	start = clock();
	greedySeedSelection(3);
	end=clock();
	float seconds = (float)(end - start) / (CLOCKS_PER_SEC);

	printf("\n Best result was: %f coverage, %f sec\n", BestRun[0],BestRun[1]);
	printf("program Time usage was:%f seconds\n", seconds);

	for(int i=0; i<TOTAL_NODE; i++){
		printf("%i ",bestResult[i]);
	}
	printf("\n");

	return 0;
}

int  ipCore(){
	globalTime=0;
	globalCoverage=0;
	for(int run =0; run <50; run++){
		for(i=0;i<TOTAL_NODE; i++){
			outputStream[i]=frontierStream[i];
		};



		time_t start, end;
		start = clock();

		TopLevelWrapper(inputStream,outputStream,frontierStream,numBytes,randomSeed++, global_probability);
		end=clock();

		float seconds = (float)(end - start) / (CLOCKS_PER_SEC);

		for(i=0;i<TOTAL_NODE; i++){
			if(outputStream[i]){
				activatedElement++;
			}
				//printf("Result at %i gave: %i\n",i,outputStream[i]);
		}
		//printf("Total activated node is:%i out of %i, result in a coverage of: %f, Time usage was:%f seconds\n",activatedElement,TOTAL_NODE, (activatedElement/(double)TOTAL_NODE),seconds);
		//printf("Total run was:%i.\n",randomSeed);
		globalTime+= seconds;
		globalCoverage += activatedElement/(double)TOTAL_NODE;

		activatedElement =0;
	}
	printf("Average output took %f sec\n", globalTime/50.0);
	printf("Average coverage was %f\n",globalCoverage/50);
	currentRun[0]=globalCoverage/50;
	currentRun[1]=globalTime/50.0;


	return 0;
}

int compare(){
	int ret=0;
	if(currentRun[0]>BestRun[0]){
		double covergage = currentRun[0];
		BestRun[0]= covergage;
		double time = currentRun[1];
		BestRun[1] = time;
		ret =1;
	}
	else if(currentRun[0]==BestRun[0]){
			if(currentRun[1]<BestRun[0]){
				double covergage = currentRun[0];
						BestRun[0]= covergage;
						double time = currentRun[1];
						BestRun[1] = time;
						ret =1;
			}
		}
	return ret;
}

int greedySeedSelection(int k){
int i=0;
	while (i<k){
		for(int j=0; j<TOTAL_NODE; j++){
			if(frontierStream[j]);

			else {
				frontierStream[j]=1;
				for(int g=0;g<TOTAL_NODE; g++){
					printf("%i ",frontierStream[g]);
				}
				printf("\n)");
				ipCore();
				if(compare()){
					for(int m=0; m<TOTAL_NODE; m++){
						bestResult[m]=frontierStream[m];
					}
				}
				frontierStream[j]=0;
			}
		}
		for(int l = 0; l<TOTAL_NODE; l++){
			frontierStream[l] = bestResult[l];
		}
		i++;
	}
	return 0;
}
