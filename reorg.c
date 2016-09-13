#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "utils.h"

typedef struct
{
	unsigned long  person_id;
	unsigned short birthday;
	unsigned short location;
	unsigned long  knows_first;
	unsigned short knows_n;
	unsigned short interest_n;
	unsigned short interest[0];
} Person_custom;

Person *person_map;
unsigned short *interest_map;
Person_custom *person_custom_map;

FILE *output;

unsigned long person_length, interest_length, person_custom_length;

int main(int argc, char *argv[]) {
	char* person_output_file   = makepath(argv[1], "person",   "bin");
	char* interest_output_file = makepath(argv[1], "interest", "bin");
	char* knows_output_file    = makepath(argv[1], "knows",    "bin");

	/*person_map   = (Person *)         mmapr(makepath(argv[1], "person",   "bin"), &person_length);
	interest_map = (unsigned short *) mmapr(makepath(argv[1], "interest", "bin"), &interest_length);
	
	/*output=fopen("data/output.bin","wb");
	if (output==NULL){
		printf("reorg failed!\n");
		return 0;
	}
	unsigned long ite_length=person_length/sizeof(Person);
	for (unsigned long i=0;i<ite_length;i++){
		unsigned long first=person_map[i].interests_first, num=person_map[i].interest_n;
		unsigned long size=2*sizeof(unsigned long)+(4+num)*sizeof(unsigned short);
		Person_custom *temppc=malloc(size);
		temppc->person_id=person_map[i].person_id;		
		temppc->birthday=person_map[i].birthday;	
		temppc->location=person_map[i].location;
		temppc->knows_first=person_map[i].knows_first;
		temppc->knows_n=person_map[i].knows_n;
		temppc->interest_n=person_map[i].interest_n;
		for(unsigned long j=0;j<num;j++){
			temppc->interest[j]=interest_map[j+first];
		}
		fwrite(temppc,size,1,output);
	}
	fclose(output);
	printf("writing succeed!\n");*/

	/*person_custom_map = (Person_custom *) mmapr(makepath(argv[1], "output", "bin"), &person_custom_length);
	
	printf("%lu\n", person_map[1].person_id);
	printf("%lu\n", person_custom_map[1].person_id);
	

	

	FILE *outPC=fopen("data/outpc.bin","wb");
	/*for(unsigned long i=0;i<ll;i++){
		for(unsigned long j=0;j<person_custom_map[i].interest_n;j++){
			printf("%hu\n", person_custom_map[i].interest_n);
			printf("%hu ", person_custom_map[i].interest[j]);
			//fwrite(&(person_custom_map[i].interest[j]),sizeof(unsigned short),1,outPC);
		}	
		printf("\n");
	}*/

	/*fclose(outPC);
	printf("done!\n");*/
	

	// this does not do anything yet. But it could...
	return 0;
}

