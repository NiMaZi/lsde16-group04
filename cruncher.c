#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "utils.h"

#define QUERY_FIELD_QID 0
#define QUERY_FIELD_A1 1
#define QUERY_FIELD_A2 2
#define QUERY_FIELD_A3 3
#define QUERY_FIELD_A4 4
#define QUERY_FIELD_BS 5
#define QUERY_FIELD_BE 6



clock_t start, finish;
pair *potl;
unsigned int pr_length=0, pr_set_size=1000;

Person *person_map;
unsigned int *knows_map;
unsigned short *interest_map;

unsigned long person_length, knows_length, interest_length;

FILE *outfile;

int result_comparator(const void *v1, const void *v2) {
    Result *r1 = (Result *) v1;
    Result *r2 = (Result *) v2;
    if (r1->qid < r2->qid)
    	return -1;
    else if (r1->qid > r2->qid)
    	return +1;
    else if (r1->score > r2->score)
        return -1;
    else if (r1->score < r2->score)
        return +1;
    else if (r1->person_id < r2->person_id)
        return -1;
    else if (r1->person_id > r2->person_id)
        return +1;
     else if (r1->knows_id < r2->knows_id)
        return -1;
    else if (r1->knows_id > r2->knows_id)
        return +1;
    else
        return 0;
}

unsigned char get_score(Person *person, unsigned short areltd[]) {
	long interest_offset;
	unsigned short interest;
	unsigned char score = 0;
	for (interest_offset = person->interests_first; 
		interest_offset < person->interests_first + person->interest_n; 
		interest_offset++) {

		interest = interest_map[interest_offset];
		if (areltd[0] == interest) score++;
		if (areltd[1] == interest) score++;
		if (areltd[2] == interest) score++;
		// early exit
		if (score > 2) {
			break;
		}
	}
	return score;
}

char likes_artist(Person *person, unsigned short artist) {
	long interest_offset;
	unsigned short interest;
	unsigned short likesartist = 0;

	for (interest_offset = person->interests_first; 
		interest_offset < person->interests_first + person->interest_n; 
		interest_offset++) {

		interest = interest_map[interest_offset];
		if (interest == artist) {
			likesartist = 1;
			break;
		}
	}
	return likesartist;
}

void query(unsigned short qid, unsigned short artist, unsigned short areltd[], unsigned short bdstart, unsigned short bdend) {
	unsigned int person_offset;

	P *person;
	F *friends;
	
	Person *person_t;
	unsigned char score;

	unsigned int p_length=0, p_set_size=1000, f_length=0, f_set_size=1000;

	person = malloc(p_set_size*sizeof(P));
	friends = malloc(f_set_size*sizeof(F));

	//printf("Running query %d\n", qid);

	unsigned long ite_length=person_length/sizeof(Person);

	for (person_offset = 0; person_offset < ite_length; person_offset++) {
		person_t = &person_map[person_offset];

		if (person_offset > 0 && person_offset % REPORTING_N == 0) {
			printf("%.2f%%\n", 100 * (person_offset * 1.0/ite_length));
		}
		// filter by birthday


		// person must not like artist yet
		if (likes_artist(person_t, artist)){
			if (p_length>=p_set_size){
				p_set_size*=2;
				person=realloc(person,p_set_size*sizeof(P));
			}
			person[p_length].person=*person_t;
			person[p_length].qid=qid;
			person[p_length].offset=person_offset;
			p_length++;
		}else{
			if (person_t->birthday < bdstart || person_t->birthday > bdend) continue;
			score=get_score(person_t, areltd);
			if (score>=1){
				if (f_length>=f_set_size){
					f_set_size*=2;
					friends=realloc(friends,f_set_size*sizeof(F));
				}
				friends[f_length].person=*person_t;
				friends[f_length].qid=qid;
				friends[f_length].score=score;
				friends[f_length].offset=person_offset;
				f_length++;
			}
		}
	}

		// but person must like some of these other guys

		// check if friend lives in same city and likes artist 
	for(unsigned int pite=0;pite<p_length;pite++){
		for(unsigned int fite=0;fite<f_length;fite++){
			if (person[pite].person.location==friends[fite].person.location){
				if (pr_length>=pr_set_size){
					pr_set_size*=2;
					potl=realloc(potl,pr_set_size*sizeof(pair));
				}
				potl[pr_length].qid=qid;
				potl[pr_length].score=friends[fite].score;
				potl[pr_length].Pid=person[pite].person.person_id;
				potl[pr_length].Pos=person[pite].offset;
				potl[pr_length].Fid=friends[fite].person.person_id;
				potl[pr_length].Fos=friends[fite].offset;
				potl[pr_length].Pkf=person[pite].person.knows_first;
				potl[pr_length].Pkn=person[pite].person.knows_n;
				potl[pr_length].Fkf=friends[fite].person.knows_first;
				potl[pr_length].Fkn=friends[fite].person.knows_n;
				potl[pr_length].flag=0;
				pr_length++;
			}
		}
	}
}

void query_line_handler(unsigned char nfields, char** tokens) {
	unsigned short q_id, q_artist, q_bdaystart, q_bdayend;
	unsigned short q_relartists[3];

	q_id            = atoi(tokens[QUERY_FIELD_QID]);
	q_artist        = atoi(tokens[QUERY_FIELD_A1]);
	q_relartists[0] = atoi(tokens[QUERY_FIELD_A2]);
	q_relartists[1] = atoi(tokens[QUERY_FIELD_A3]);
	q_relartists[2] = atoi(tokens[QUERY_FIELD_A4]);
	q_bdaystart     = birthday_to_short(tokens[QUERY_FIELD_BS]);
	q_bdayend       = birthday_to_short(tokens[QUERY_FIELD_BE]);


	query(q_id, q_artist, q_relartists, q_bdaystart, q_bdayend);
}

int main(int argc, char *argv[]) {
	start=clock();
	if (argc < 4) {
		fprintf(stderr, "Usage: [datadir] [query file] [results file]\n");
		exit(1);
	}
	/* memory-map files created by loader */
	person_map   = (Person *)         mmapr(makepath(argv[1], "person",   "bin"), &person_length);
	interest_map = (unsigned short *) mmapr(makepath(argv[1], "interest", "bin"), &interest_length);
	knows_map    = (unsigned int *)   mmapr(makepath(argv[1], "knows",    "bin"), &knows_length);

  	outfile = fopen(argv[3], "w");  
  	if (outfile == NULL) {
  		fprintf(stderr, "Can't write to output file at %s\n", argv[3]);
		exit(-1);
  	}
  	unsigned long knows_offset, knows_offset2;
  	Person *knows;
  	char flag=0;

  	/* run through queries */
  	potl = malloc(pr_set_size*sizeof(pair));	
	parse_csv(argv[2], &query_line_handler);
	unsigned int result_length = 0, result_idx, result_set_size = 1000;
	Result* results = malloc(result_set_size * sizeof (Result));

	for (unsigned int i = 0; i < pr_length; i++) {
		for(unsigned int j=potl[i].Pkf;j<potl[i].Pkf+potl[i].Pkn;j++){
			if (knows_map[j]==potl[i].Fos){
				potl[i].flag=1;
				break;
			}
		}
	}
	for(unsigned int i=0;i<pr_length;i++){
		if (potl[i].flag==0) continue;
		for(unsigned int j=potl[i].Fkf;j<potl[i].Fkf+potl[i].Fkn;j++){
			if (knows_map[j]==potl[i].Pos){
				if (result_length>=result_set_size){
					result_set_size*=2;
					results=realloc(results,result_set_size*sizeof(Result));
				}
				results[result_length].person_id = potl[i].Fid;
				results[result_length].knows_id = potl[i].Pid;
				results[result_length].score = potl[i].score;
				results[result_length].qid = potl[i].qid;
				result_length++;
				break;
			}
		}
	}
	qsort(results, result_length, sizeof(Result), &result_comparator);

	for (result_idx = 0; result_idx < result_length; result_idx++) {
		fprintf(outfile, "%d|%d|%lu|%lu\n", results[result_idx].qid, results[result_idx].score, 
			results[result_idx].person_id, results[result_idx].knows_id);
	}

	finish=clock();
	printf("%f\n", (double)(finish-start)/CLOCKS_PER_SEC);
	return 0;
}
