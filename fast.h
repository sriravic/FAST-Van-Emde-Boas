#ifndef __FAST_H__
#define __FAST_H__

#pragma once

#ifdef _MSC_VER
#include <intrin.h>
#endif


// architecture specific parameters
// to be set appropriately

#define SIMD_WIDTH  4
#define KB			1 << 10
#define MB			1 << 20
#define GB			1 << 30

#define uint		unsigned int
#define ulong		unsigned long
#define uchar		unsigned char

inline static int pow2(int p) { return 1 << p; }
inline static bool isPow2(int p) { return (p & (p-1)) == 0; }
// returns the location of the Most significant bit 1
inline static char fls(ulong p, ulong& pos) {
	return _BitScanReverse(&pos, p);
}
// given an inoder node, this will give the index in the fast tree.!
/*

Logic:
	Since we have dK, dL, dP, dN - levels.
	dN > dP > dL > dK
*/

struct Tuple
{
	uint key;
	uint rid;
	Tuple():key(UINT_MAX), rid(UINT_MAX) {}
	Tuple(uint _k, uint _r):key(_k), rid(_r) {}
	bool operator< (const Tuple& T) { return key < T.key; }
	bool operator<= (const Tuple& T) { return key <= T.key; }
	bool operator> (const Tuple& T) { return key > T.key; }
	bool operator>= (const Tuple& T) { return key >= T.key; }
	bool operator== (const Tuple& T) { return key == T.key; }
};


// Tree construct values
// sizes
static int E;					// Key size
static int K;					// SIMD width in bytes
static int L = 64;				// Cache line width in bytes
static int C;					// LLC size
static int P = 2 * MB;			// Page size in bytes

static uint dN = 31;			// number of elements
static uint dP = 21;			// page level depth
static uint dL = 4;				// cache line depth
static uint dK = 2;				// SIMD depth

static uint numPages;
static uint numCacheLines;


// number of elements
static int N;					// Number of elements
static int Nk = (1<<dK) - 1;
static int Nl = (1<<dL) - 1;
static int Np;

// given a node, i will return a bunch of stuff for calculating a lot of stuff
typedef struct NodeData
{
	uint pageAddress;
	uint pageOffset;
	uint levelOffset;
	uint cacheOffset;
}NodeData;


// given a root node, and another node, we can find at which depth, the node is from the root
// Note: Depth- length of path from root to node
// Logic: From the LSB, how many bits are common with the root. This value is subtracted from height
const uint table[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144,
				524288, 1048576, 2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728, 268435456,
				536870912, 1073741824, 2147483648};

inline uint findNodeDepth(uint node, uint root, uint height) {
	// in a complete BST, the even nodes are always at the bottom.
	// so we can return immediately in such cases
	if(node % 2 == 0) return height;
	int i = 0;
	while(i <= 31) {
		if(((node & 0x1) == 1) && ((root & 0x1) == 1)) {
			//shift the node/root to the right
			root >>= 1;
			node >>= 1;
			i++;
		} else {
			break;
		}
	}
	return height-i;
}


/*
// optimized findNodeDepth. I guess this should reduce some operations of the shifting and such
inline uint findNodeDepth(uint node, uint root, uint height) {
	if(node % 2 == 0) return height;
	int i;
	for(i = 0; i <= 31; i++) {
		uint t = table[i];
		if(((node & t) == 1) && ((root & t) == 1))
			continue;
		else break;
	}
	return height - i;
}
*/
// given a tree, we determine some constants required for the tree construction
// add the constants as we go
inline void findTreeConstants(uint root, uint height, uint& numPageLevels, uint& numCacheLevels) {
	numPageLevels = height / dP + (height % dP != 0);
	numCacheLevels = height / dL + (height % dL != 0);
}

uint pageBlocking(uint node, const uint& root, const uint& height, uint& numPrePageBlockNodes, uint& pageRootIdx, uint& pageBlockingHeight);
uint cacheBlocking(uint node, const uint& root, const uint& height, const uint& pageRootIdx, const uint& pageBlockingHeight, uint& numPreCacheBlockNodes, uint& cacheRootIdx, uint& cacheBlockingHeight);
uint simdBlocking(uint node, const uint& root, const uint& height, const uint& cacheRootIdx, const uint& cacheBlockingHeight, uint& numPreSimdBlockNodes, uint& simdRootIdx, uint& simdBlockingHeight);


// we assume that all nodes are sorted. Root is mid node, height is calculated appropriately (0 indexed only)
uint inorderToFast(uint node, uint root, uint height);


// this is the traversal code
// search code - SIMD code
bool traverseTree(Tuple* tuples, uint* keys, const uint& key1, const uint& key2);


#endif
