/*
	Brandon Meng
	CSC374 Tue/Thur 10:10 AM
	Cachelab
	


	OVERVIEW:
	This code is an a simulator for how a cache would handle hits, misses, and evicitons.
	The main cache is stored as an array of structs. Each struct represents an entry, and
	a struct is made for every entry in each set. After all init work, fscanf reads 
	the infiles and makes calls to missOrHit, the cache simulation code.
	hitOrMiss looks through all of the cache entries for a given set to see if a hit can
	be made. If there is a hit, the timeStamp(my LRU implementation) and hit_count are 
	increased. If not hit is made, the findEviction function is called. This function 
	looks through all of the items in a given set. If there is an entry that is still 
	cold, it is used. If no cache is cold, the entry that hasnt been used in the longest
	is found. The eviction total is updated and the evictee's index is returned. hitOrMiss
	then overrides any needed values, updates timeStamps, updates misscounts, and returns. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include "cachelab.h"
// globals////////////////////////////////////////////////////////////////////////////
int lines_per_set;
int timeStamp;//global timer that functions as LRU
int hit_total= 0, miss_total= 0, eviction_total= 0;

//the cPart struct represents an entry/line 
//The tagInfo is the entry tage and the cTime is the LRU
struct cPart 
{
	int cTime;
	long long tagInfo;
};

void printUsage()
{
	printf( "Usage: csim [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n" );
}
/////////////////////////////////////////////////////////////////////////////////////////////
//this function is given a copy of the struct arr and the current set number, and uses that//
//to calculate the best place to insert a new value. If the timestamp is 0, we can assume a//
//cold cache and use that place. Otherwise, the loop searches with the entry with the      //
//smallest timestamp, as this entry would be the oldest and the best one to evict.         // 
//Then, the eviction total is increased.                                                   //       
//The index of the entry that will be evicted/cold cache entry is returned                 //
/////////////////////////////////////////////////////////////////////////////////////////////
int findEviction(int setNum2, struct cPart res2[])
{
	int index=lines_per_set*setNum2;
	int x;
	for (x=0; x<lines_per_set;x++)
	{
		if(res2[x+(lines_per_set*setNum2)].cTime==0 )
		{
			return (x+(setNum2*lines_per_set)) ;}
		else if(res2[x+(lines_per_set*setNum2)].cTime< res2[index].cTime)
		{
			index=x+(setNum2*lines_per_set);
		}
	}
	eviction_total++;
	return index;
}
///////////////////////////////////////////////////////////////////////////////////////////////
//This function takes the current instruct, the needed memory address, the tag/set/block     //
//bits, and a copy of the struct arr. The appropriate set mask is created, and that is used  //
//to the set number of the current memoryaddress. The tag bits are also calculated. The loop //
//then goes through the cache, only searching through the items that are in the same set     //
//as the memoryaddress. *Remember* All of the sets include lines_per_set lines, so that needs//
//to be multiplied to the set index. If there is a hit, the hit total is updated,the         //
//time stamp is updated and increased and the function ends.                                 //
//If no hit is made, the miss total is increased. Then, the find eviction function is        //
//called. Once the function returns an index, that information is changed, the timestamp     //
//is updated and increased, and then the function terminates.                                //
///////////////////////////////////////////////////////////////////////////////////////////////


void hitOrMiss(char instruction, long long memoryaddress,  int tag_bits1, int set_bits, int block_bits2, struct cPart res[])
{
	long long setMask;
	long long setNum;
	long long tagStuff;
	int x;
	int y;
	//masking and tag bits//
	setMask=((long long)pow(2, set_bits))-1;
	setNum=((memoryaddress)>>(block_bits2))&setMask;
	tagStuff=(( long long) memoryaddress)>>(set_bits+block_bits2);

	//main hit or miss detection loop//
	for (x=0; x<lines_per_set; x++)
	{
		if(tagStuff==((( long long) res[x+(setNum*lines_per_set)].tagInfo)>>(set_bits+block_bits2)) && res[x+(setNum*lines_per_set)].cTime != 0 )
		{
			res[x+(setNum*lines_per_set)].cTime=timeStamp;
			timeStamp++;
			hit_total++;
			return;
		}
	}

	//miss handling//
	miss_total++;
	y=findEviction(setNum, res);
	res[y].tagInfo=memoryaddress;
	res[y].cTime=timeStamp;
	timeStamp++;

	return;
}

//////////////////////////////////
//This code was already included//
//I initialized some of my      //
//variables here, but otherwise //
//unchanged.                    //
//////////////////////////////////
int main( int argc, char * argv[] )
{
	// using getopt(), parse the command line arguments
	char * filename= NULL;
	int option;
	char verbose= 0;
	lines_per_set= 0;
	int s_bits= 0, block_bits= 0, tag_bits= 0;
	char iRead=' ';
	timeStamp=1;
	FILE* traceIO;


	while ( (option= getopt( argc, argv, "hvs:E:b:t:" )) != -1 )
	{
		switch( option )
		{
		case 'h':
			printUsage();
			exit(EXIT_SUCCESS);
			break;
		case 'v':
			verbose= 1;
			break;
		case 's':
			s_bits= atoi( optarg );
			break;
		case 'E':
			lines_per_set= atoi( optarg );
			break;
		case 'b':
			block_bits= atoi( optarg );
			break;
		case 't':
			filename= optarg;
			break;
		default:
			printf( "Unknown option encountered\n" );
			printUsage();
			exit(EXIT_FAILURE);
		}		
	}
	if ( s_bits == 0 || lines_per_set == 0 || block_bits == 0 )
	{
		printf( "Value for cache parameter not passed on command line\n" );
		printUsage();
		exit(EXIT_FAILURE);
	}

//////////////////////////////////////////////////////////////////////////////////
//The code that I added here is minimal. The cache is stored as an array of     //
//structs. These structs are replicas of entries. The tag bits are caculcated   //
//and the file is opened and read. The timestamp is initialize, and the input   //
//values for scanf are created. Then all of the values for the cache are changed//
//to zero as an initialization. Next, the scanf is called on the trace file.    //
//This information is stored and then the appropriate calls are made. If an     //
//instruction is made, nothing happens, if a modify is made, a call to hitOrMiss//
//(our cache simulator) is made, and then the hit total is increased to         //
//represent a data store. Otherwise, a call is made to the function.            //
//*Note* The hit/miss/eviction totals were made global to prevent them from     //
//being passed to hitOrMiss. Once the loop is complete, printSummary is called. //
//*Note* Since the cache is stored as an array, all accesses have to consider   //
//the number of lines per set. As a result, all accesses here and in hitOrMiss  //
//multiply the set number by the number of lines in a set.                      //
//////////////////////////////////////////////////////////////////////////////////

	int totalParts=(int)(pow(2, s_bits)*lines_per_set);//caculates number of entries
	struct cPart fullCache[totalParts];//cache Array
	tag_bits=sizeof(long long)*8- (s_bits+block_bits);
	int fd = open(filename, O_RDONLY);
	read(fd, &iRead, 1);
	timeStamp=1;
	long long memAd=0;
	char instruct='a';
	int acSize= 0;
	int x ;


	//init//
	for (x=0; x< totalParts; x++)
	{
		fullCache[x].cTime=0;
		fullCache[x].tagInfo=0;
	}


	traceIO=fopen(filename, "r");
	while(fscanf(traceIO, " %c %llx,%d", &instruct, &memAd, &acSize) ==3)
	{
		if (instruct=='I')
		{
			continue;
		}
		else if(instruct=='M')
		{
			hitOrMiss(instruct, memAd, tag_bits, s_bits,block_bits, fullCache);
			hit_total++;
		}
		else
			{hitOrMiss(instruct, memAd, tag_bits, s_bits,block_bits, fullCache);}
	}

	printSummary( hit_total, miss_total, eviction_total );
    return 0;
}