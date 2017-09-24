#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/*Struct for a cache line*/
typedef struct {
	int valid;
	int dirty;
	int lruindex;
	int dirtybit;
	char *tag;
} Line;

/*Struct for a set; stores a line*/
typedef struct {
	Line* baseline;
    int size;
    Line **lines;
} Set;

/*Struct for a cache; pointer to array of sets and stores all relevant variables*/
typedef struct {
        int cachesize;
        int blocksize;
		int setsize;
        char* associativity;
        int avalue;
        char *replacealg;
        char* writepol;
        int numsets;
        int reads;
        int writes;
        int misses;
        int hits;
	Set **sets;
} Cache;

/*Struct that stores relevant values of a line in the trace file including address-related values*/
typedef struct {
	int numtagbits;
	int numsetindexbits;
	int numblockoffsetbits;
	int decimalsindex;
	char *setindex;
	char *tag;
	char *readwrite;
} addressinfo;

/*Main method*/
int main(int argc, char *argv[]);

void readsandwrites(char* name);

void updateCache(FILE *file, int num_lines);

/*Returns the binary form of a hexadecimal address*/
char *binaryFromHex(char *address);

/*Calculates number of sets in cache*/
int calculateNumberofSets();

void initializecache();

int calcNumBlockOffsetBits();

void writeBack(addressinfo *memory);

void append(char *s, char c);

void lruWB(addressinfo *memory);

void writeThrough(addressinfo *memory);

void updateRecents(addressinfo *memory);

int getHighestIndex(addressinfo *memory);

void initializeAddressInfo(addressinfo *memory);

void freecache();

void updateAddressValues(int mem_length, addressinfo *memory, char *address);

int binaryToDecimal(char *binary);

/*Creates and initializes a cache line*/
Line* makeLine();

/*Creates and mallocs a set*/
void createSet(Set *set);

/*Creates a cold cache*/
void coldHardCache();

int getNumLines(FILE *trace_file);