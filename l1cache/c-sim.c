#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "c-sim.h"

/*Global variable representing the cache*/
Cache *currentsim = NULL;

int main(int argc, char* argv[]){
	//test case 1: make sure that if file 1 is empty it prints out 0

    if (argc != 6 && argc != 7) {
        fprintf(stderr, "ERROR: Invalid amount of arguments\n");
        return 1;
    }

    initializecache();

    currentsim->cachesize = atoi(argv[1]);

    char* assoc_substring;
    char* assoc_val;
    char* assoc = argv[2];
    int n;

    if(strcmp(assoc,"direct") == 0) {
   		 currentsim->associativity = assoc;
  	} else if(strcmp(assoc,"assoc") == 0) {
    	currentsim->associativity = assoc;
    }
     else {
     	assoc_substring = (char*) malloc(6);
        strncpy(assoc_substring, assoc, 6);
        n = atoi(assoc_val);
        if (strcmp(assoc_substring, "assoc:") == 0) {
            if (strlen(assoc) - 6 == 0) {
                    fprintf(stderr, "ERROR: No associative value\n");
                    free(assoc_substring);
                    exit(0);
                }
                assoc_val = (char*) malloc(strlen(assoc) - 6);
                strncpy(assoc_val, assoc + 6, strlen(assoc) - 6);
                n = atoi(assoc_val);
                if (n == 0) {
                    fprintf(stderr, "ERROR: Invalid associativity value\n");
                    free(assoc_val);
                    free(assoc_substring);
                    exit(0);
                } else {
                    currentsim -> associativity = assoc;
                    currentsim -> avalue = n;
                }
        }
    }

    currentsim->blocksize = atoi(argv[3]);

    currentsim->numsets = calculateNumberofSets();

    char* replacealgorithm = argv[4];

    //LRU is least recently used cacheline to replace
    if(strcmp(replacealgorithm,"LRU") == 0) {
    	currentsim -> replacealg = replacealgorithm;
    //FIFO is replace the block that has been in the cache the longest amount of time
  	} else if(strcmp(replacealgorithm,"FIFO") == 0) {
    	currentsim -> replacealg = replacealgorithm;
  	}

  	char* writepolicy = argv[5];

  	if(strcmp(writepolicy,"wt") == 0) {
    	currentsim -> writepol = writepolicy;
  	}else if(strcmp(writepolicy,"wb") == 0) {
    	currentsim -> writepol = writepolicy;
	}

    coldHardCache(); /*Builds the cache with a bunch of empty arrays*/

    char* name = argv[6]; 
	//done reading in inputs and by now we should have created our cache
	readsandwrites(name);

} //end main

void readsandwrites(char* name) {

	FILE* file;
	file = fopen(name, "r");
	//if file does not exist 
	if(!file){
		printf("error\n");
	}
	else {
        int num_lines;

        num_lines = getNumLines(file);
        updateCache(file, num_lines);

        fclose(file);
        freecache();
		printf("Memory reads : %d\n",currentsim->reads);
	  	printf("Memory writes : %d\n",currentsim->writes);
	  	printf("Cache hits : %d\n",currentsim->hits);
	  	printf("Cache misses : %d\n",currentsim->misses);
  }

}

/*Updates the cache given a trace file*/
void updateCache(FILE *file, int num_lines) {
    //keep track of sets and lines
    Line *currentline;
    Set *currentset;
    Set *baseset;

    int mem_length;
    char ip[12], rw[2], address[11], *binary_add;

    addressinfo *memory;
    while (fscanf(file, "%s %s %s", ip, rw, address) != EOF && strcmp(ip, "#eof") != 0) {
            memory = (addressinfo *) malloc(sizeof(addressinfo));
            initializeAddressInfo(memory);
            if (strcmp(rw, "W") == 0) {
                    memory -> readwrite = "W";
            }
            else {
                    memory -> readwrite = "R";
            }
            binary_add = binaryFromHex(address);
            mem_length = strlen(binary_add);
            updateAddressValues(mem_length, memory, binary_add); /*Calls malloc for the memory's set index and tag in this function*/
            
            //LRU algorithm implementation
            if(strcmp((currentsim -> replacealg), "LRU") == 0){
                /*Performs operations for a write-through or write-back depending on the write policy*/
                if (strcmp((currentsim -> writepol), "wt") == 0) {
                        writeThrough(memory);
                } else {
                    writeBack(memory);
                }
            //FIFO algorithm implementation
            } else if(strcmp((currentsim -> replacealg), "FIFO") == 0) {

                int k;
                for(k = 0; k < currentsim->setsize; k++) {
                    currentline = currentset->baseline + k;

                    if(currentline->tag == memory->tag) {
                        currentsim->hits++;
                        /*writes directly through for write-through cache*/
                        if(strcmp(rw, "W") == 0) {
                            if(strcmp((currentsim -> writepol), "wt") == 0)
                                currentsim->writes++;
                            else
                            /*writes only the cache and sets the dirty bit in the written line*/
                                currentline->dirtybit = 1;
                            }
                        else
                            currentsim->reads++;
                        } else {
                            currentsim->misses++;
                    }

                    /*if dirtybit is set, writes before evicting
                    * dirtybit is only applicable for the write-back cache*/
                    if(currentline->dirtybit == 1) {
                        currentsim->writes++;
                        currentline->dirtybit = 0;
                    }

                    /*evicts and reads in new block (currently only direct cache is implemented)*/
                    currentline->tag = memory->tag;
                    currentline->valid = 1;

                    if(strcmp(rw, "W") == 0) {
                        if(strcmp((currentsim -> writepol), "wt") == 0) {
                        /*reads and writes through to memory immediately, so both read and write*/
                            (currentsim->reads)++;
                            (currentsim->writes)++;
                        } else {
                            /*reads but holds off on writing until eviction time (or some other time like instruction halting
                            * or the computer turning off*/
                            (currentsim->reads)++;
                            currentline->dirtybit = 1;
                        }
                    }
                }
            }
            free(binary_add);
            free(memory);
        }
}

/*Calculates the number of sets*/
int calculateNumberofSets() {
    if (strcmp(currentsim->associativity, "direct") == 0) {
        return (currentsim->cachesize)/(currentsim-> blocksize);
    } else if (currentsim->avalue != 0) {
        return (currentsim->cachesize)/(currentsim->blocksize * currentsim->avalue);
    } else {
		return 1;
    }
}

/*Calculates number of block offset bits*/
int calcNumBlockOffsetBits() {
    return (int)(log((double)currentsim -> blocksize)/log(2.0));
}

/*Calculates number of set index bits*/
int calcNumSetIndexBits() {
    return (int)(log((double)currentsim -> numsets)/log(2.0));
}

/*Returns the number of bits needed for the tag*/
int calcNumTagBits(int mem_length, addressinfo *memory) {
    	return mem_length - memory -> numsetindexbits - memory -> numblockoffsetbits;
}

char* binaryFromHex(char *address){
    int i;
    char *bin_add = (char*) malloc(sizeof(char) * 33);
    for(i = 2; i < strlen(address); i++) {
        switch(address[i]) {
		case '0': strcat(bin_add,"0000"); break;
        	case '1': strcat(bin_add,"0001"); break;
        	case '2': strcat(bin_add,"0010"); break;
        	case '3': strcat(bin_add,"0011"); break;
            case '4': strcat(bin_add,"0100"); break;
            case '5': strcat(bin_add,"0101"); break;
            case '6': strcat(bin_add,"0110"); break;
            case '7': strcat(bin_add,"0111"); break;
            case '8': strcat(bin_add,"1000"); break;
            case '9': strcat(bin_add,"1001"); break;
            case 'a': strcat(bin_add,"1010"); break;
            case 'b': strcat(bin_add,"1011"); break;
            case 'c': strcat(bin_add,"1100"); break;
            case 'd': strcat(bin_add,"1101"); break;
            case 'e': strcat(bin_add,"1110"); break;
            case 'f': strcat(bin_add,"1111"); break;
        }
   	}
	strcat(bin_add,"\0");
    	return bin_add;
}

/*Initialize default cache values*/
void initializecache() {
    	currentsim = (Cache*) malloc(sizeof(Cache));
    	currentsim -> cachesize = 0;
    	currentsim -> blocksize = 0;
    	currentsim -> setsize = 0;
    	currentsim -> associativity = NULL;
    	currentsim -> avalue = 0;
    	currentsim -> writepol = NULL;
    	currentsim -> numsets = 0;
    	currentsim -> sets = NULL;
}

/*Creates a cold cache*/
void coldHardCache() {
    int i, l;
    currentsim -> sets = (Set**) malloc(currentsim -> numsets * sizeof(Set*));
        for (i = 0; i < currentsim -> numsets; i++) {
        currentsim -> sets[i] = (Set*) malloc(sizeof(Set));
        createSet(currentsim -> sets[i]);
                for (l = 0; l < currentsim -> setsize; l++) {
            currentsim -> sets[i] -> lines[l] = makeLine();
                }
        }
}

/*Creates a line in the cache*/
Line* makeLine() {
        Line *temp;
        temp = (Line*) malloc(sizeof(Line));
        temp -> valid = 0;
    temp -> dirty = 0;
        temp -> tag = (char *)malloc(sizeof(char) * 33);
        temp -> lruindex = 0;
        return temp;
}

/*Gets number of lines in the trace file*/
int getNumLines(FILE *file) {
    int ch, num_lines = 0;
        do {
        ch = fgetc(file);
                if (ch == '\n')
                    num_lines++;
        } while (ch != EOF);
        return num_lines;
}


/*Creates and mallocs a set*/
void createSet(Set *set) {
        set -> lines = (Line**) malloc(currentsim -> setsize * sizeof(Line));
}

void initializeAddressInfo(addressinfo *memory) {
    	memory -> numtagbits = 0;
    	memory -> numsetindexbits = 0;
    	memory -> numblockoffsetbits = 0;
    	memory -> setindex = NULL;
    	memory -> decimalsindex = 0;
    	memory -> tag = NULL;
    	memory -> readwrite = NULL;
}

void updateAddressValues(int mem_length, addressinfo *memory, char *address) {
    	int i;
    	memory -> numblockoffsetbits = calcNumBlockOffsetBits();
    	memory -> numsetindexbits = calcNumSetIndexBits();
    	memory -> numtagbits = calcNumTagBits(mem_length, memory);
    	
		memory -> tag = (char*) malloc(memory -> numtagbits * sizeof(char) + 1);
    	memory -> setindex = (char*) malloc(memory -> numsetindexbits * sizeof(char) + 1);
    	for (i = 0; i < memory -> numtagbits; i++) {
        	append(memory -> tag, address[i]);
    	}
    	
    	while (i < (memory -> numsetindexbits + memory -> numtagbits)) {
        	append(memory -> setindex, address[i]);
        	i++;
    	}
    	memory -> decimalsindex = binaryToDecimal(memory -> setindex); /*Stores decimal value of the set index*/
}

/*Converts a binary string to a decimal-form integer*/
int binaryToDecimal(char *binary) {
    	int i;
    	int result = 0;
    	int power = 0;
    	for (i = strlen(binary) - 1; i >= 0; i--) {
    	    	int added = (binary[i] - '0') * (int)pow(2, power);
    	    	result += added;
    	    	power++;
    	}
    	return result;
}

/*Gets the highest index for LRU*/
int getHighestIndex(addressinfo *memory) {
	int i, high_index = 0, lru_i = 0;
	for (i = 0; i < currentsim -> setsize; i++) {
		if (currentsim -> sets[memory -> decimalsindex] -> lines[i] -> lruindex > lru_i) {
                    	lru_i = currentsim -> sets[memory -> decimalsindex] -> lines[i] -> lruindex;
                    	high_index = i;
                }
	}
	return high_index;
}

/*Updates indices used for LRU algorithm*/
void updateRecents(addressinfo *memory) {
    	int i, high_index = getHighestIndex(memory);
    	for(i = 0; i < currentsim -> setsize; i++) {
        	currentsim -> sets[memory -> decimalsindex] -> lines[i] -> lruindex++;
    	}
    	currentsim -> sets[memory -> decimalsindex] -> lines[high_index] -> lruindex = 0; /*Resets the most recently used index*/
}

/*LRU algorithm for updating a write-through cache*/
void lruWT(addressinfo *memory) {
	int i, high_index = 0, lru_i = 0;
	for (i = 0; i < currentsim -> setsize; i++) {
                //check for unused lines
                if (currentsim -> sets[memory -> decimalsindex] -> lines[i] -> valid == 0) {
                    	strcpy(currentsim -> sets[memory -> decimalsindex] -> lines[i] -> tag, memory -> tag);
                    	currentsim -> sets[memory -> decimalsindex] -> lines[i] -> valid = 1;
			            updateRecents(memory);
                    	return;
                }
        // here all lines in cache are valid, need to evict
		if (currentsim -> sets[memory -> decimalsindex] -> lines[i] -> lruindex > lru_i) {
			lru_i = currentsim -> sets[memory -> decimalsindex] -> lines[i] -> lruindex;
			high_index = i;
		}
	}
	currentsim -> sets[memory -> decimalsindex] -> lines[high_index] -> tag = memory -> tag;
	updateRecents(memory);
}

/*Update operations for a write-through cache*/
void writeThrough(addressinfo *memory) {
	int i, j;
    	for (i = 0; i < currentsim -> setsize; i++) {
        	if (currentsim -> sets[memory -> decimalsindex] -> lines[i] -> valid == 1 && strcmp(memory -> tag, currentsim -> sets[memory -> decimalsindex] -> lines[i] -> tag) == 0) {
                    currentsim -> hits++;
			        currentsim -> sets[memory -> decimalsindex] -> lines[i] -> lruindex = 0;
			if (strcmp(memory -> readwrite, "W") == 0) {
				currentsim -> writes++;
			}
			for (j = 0; j < currentsim -> setsize; j++) {
				if (j != i) { /*Ignore the index that got a hit*/
					currentsim -> sets[memory -> decimalsindex] -> lines[j] -> lruindex++;
				}
			}
			return;
            	}
    	}
    //if program reaches this code then the cache missed, need to get data from mem, using LRU replacement_policy
	currentsim -> misses++;
	currentsim -> reads++;
	if (strcmp(memory -> readwrite, "W") == 0) {
		currentsim -> writes++;
	}
	lruWT(memory);
}

/*LRU algorithm for updating a write-through cache*/
void lruWB(addressinfo *memory) {
	int i, high_index = 0, lru_i = 0;
	for (i = 0; i < currentsim-> setsize; i++) {
                if (currentsim -> sets[memory -> decimalsindex] -> lines[i] -> valid == 0) {
                    	strcpy(currentsim -> sets[memory -> decimalsindex] -> lines[i] -> tag, memory -> tag);
                    	currentsim -> sets[memory -> decimalsindex] -> lines[i] -> valid = 1;
			if (currentsim -> sets[memory -> decimalsindex] -> lines[high_index] -> dirty == 1) {
				currentsim -> writes++;
			}
			if (strcmp(memory -> readwrite, "R") == 0) {
				currentsim -> sets[memory -> decimalsindex] -> lines[high_index] -> dirty = 0;
			}
			else {
				currentsim -> sets[memory -> decimalsindex] -> lines[high_index] -> dirty = 1;
			}
			updateRecents(memory);
                    	return;
                }
		if (currentsim -> sets[memory -> decimalsindex] -> lines[i] -> lruindex > lru_i) {
			lru_i = currentsim -> sets[memory -> decimalsindex] -> lines[i] -> lruindex;
			high_index = i;
		}
	}
	if (currentsim -> sets[memory -> decimalsindex] -> lines[high_index] -> dirty == 1) {
		currentsim -> writes++;
	}
	if (strcmp(memory -> readwrite, "R") == 0) {
		currentsim-> sets[memory -> decimalsindex] -> lines[high_index] -> dirty = 0;
	}
	else {
		currentsim -> sets[memory -> decimalsindex] -> lines[high_index] -> dirty = 1;
	}
	currentsim -> sets[memory -> decimalsindex] -> lines[high_index] -> tag = memory -> tag;
	updateRecents(memory);
}

/*Update operations for a write-back cache*/
void writeBack(addressinfo *memory) {
	int i, j;
    	for (i = 0; i < currentsim -> setsize; i++) {
        	if (currentsim -> sets[memory -> decimalsindex] -> lines[i] -> valid == 1 && strcmp(memory -> tag, currentsim -> sets[memory -> decimalsindex] -> lines[i] -> tag) == 0) {
                	currentsim -> hits++;
			currentsim -> sets[memory -> decimalsindex] -> lines[i] -> lruindex = 0;
			if (strcmp(memory -> readwrite, "W") == 0) {
				currentsim -> sets[memory -> decimalsindex] -> lines[i] -> dirty = 1;
			}
			for (j = 0; j < currentsim -> setsize; j++) {
				if (j != i) { /*Ignore the index that got a hit*/
					currentsim -> sets[memory -> decimalsindex] -> lines[j] -> lruindex++;
				}
			}
			return;
            	}
    	}
	currentsim -> misses++;
	currentsim -> reads++;
	lruWB(memory);
}

/*Adds a character to the end of a string*/
void append(char* s, char c)
{
    	int len = strlen(s);
    	s[len] = c;
    	s[len + 1] = '\0';
}

/*Frees the cache*/
void freecache() {
	int i, j;
	for (i = 0; i < currentsim -> numsets; i++) {
		for (j = 0; j < currentsim -> setsize; j++) {
			free(currentsim -> sets[i] -> lines[j] -> tag);
			free(currentsim -> sets[i] -> lines[j]);
		}
		free(currentsim -> sets[i] -> lines);
		free(currentsim -> sets[i]);
	}
	free(currentsim);
}
