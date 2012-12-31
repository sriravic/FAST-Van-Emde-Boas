// van emde boas layout tree structure

/**
NOTE: well the best way to start is to find the nearest power of two greater than number of elements and then fill in the rest of the elements
with the key greater than all keys or say INF value. We then can proceed ahead with BFS approach that gives us some starting point for the van emde layout
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <omp.h>
#include <random>
#include <queue>
#include <stdint.h>
#include "BST.h"
#include "veb.h"

#define VEB
#define NUM_ELEMENTS  (1 << 20) - 1				// assume a complete bst

#define uint	unsigned int

inline float round(float val) {
	return std::floor(val + 0.5f);
}

void printArray(int *arr, unsigned int N) {
	for(unsigned int i = 0; i < N; i++)
		std::cout<<arr[i]<<"\n";
}

struct Range
{
	unsigned int start;
	unsigned int end;
	Range(unsigned int _s, unsigned int _e):start(_s), end(_e) {}
};

// given an inorder tree, we arrange the nodes in BFS
// recursive. 
// output is assumed to have enough memory
void arrangeBFS(int* arr, int* output, unsigned int start, unsigned int end, unsigned int N) {
	static unsigned int index = 0;
	unsigned int mid = static_cast<unsigned int>((end-start)/2);
	if(mid < 0 || mid > N) return;
	output[index++] = arr[mid];
	// recurse
	arrangeBFS(arr, output, start, mid-1, N);
	arrangeBFS(arr, output, mid+1, end, N);
}

void arrangeBFS(int* arr, int* output, unsigned int N) {
	// calculate height of tree
	std::queue<unsigned int> parents;
	// we know that in a complete binary tree (NOTE :!! Complete binary tree)
	// n = 2^h-1
	// h = log(n+1)/log(2)
	unsigned int height = static_cast<int>(std::ceil(std::log(N+1)/std::log(2)));
	// from the paper, we now know the position of child nodes in an inorder tree is given by
	// i-2^((h[v])-2) and i+2^((h[v]-2))

	// root is the middle element in the range of [0-2^height-1]
	unsigned int numNodes = static_cast<int>(std::pow(2, height)) - 1;
	unsigned int root = static_cast<int>(numNodes/2);
	unsigned int idx = 0;
	output[idx++] = arr[root];
	parents.push(root);
	// root already added. so height-1
	unsigned int numParents = 1;
	for(unsigned int h = height; h > 1; h--) {
		// for all levels of the tree
		// now for all nodes within the same level
		for(unsigned int node = 0; node < numParents; node++) {
			unsigned int p = parents.front();
			unsigned int lchild = p - static_cast<int>(std::pow(2, h-2));
			unsigned int rchild = p + static_cast<int>(std::pow(2, h-2));
			//std::cout<<lchild<<" "<<rchild<<"\n";
			parents.push(lchild);
			parents.push(rchild);
			output[idx++] = arr[lchild];
			output[idx++] = arr[rchild];
			parents.pop();
		}
		// update height -> done in loop itself
		// update node at each level height
		numParents = parents.size();
	}
}

void BinarySearchIndices(int *arr, int start, int end) {
	if(start > end || start == end) return;
	else {
		int mid = (end-start)/2 + start;
		std::cout<<"Index :"<<mid<<"\n";
		BinarySearchIndices(arr, start, mid-1);
		BinarySearchIndices(arr, mid+1, end);
	}
}

void arrangeVEB(int* inputArray, int* outputArray, int root, int height, int depth, int N) {
	static int recursion = 0;
	static int idx = 0;
	if(N <= 3) {
		outputArray[idx++] = inputArray[root];
		if(N == 3) {
			int lchild = 2*root + 1;
			int rchild = 2*root + 2;
			outputArray[idx++] = inputArray[lchild];
			outputArray[idx++] = inputArray[rchild];
		}
		return;
	}

	//int totalHeight = static_cast<int>((std::log(N+1)/std::log(2)));
	//int h0 = static_cast<int>(std::floor(totalHeight/2));
	//int t = totalHeight - h0;
	uint totalHeight = static_cast<uint>(std::log(N+1)/std::log(2));
	uint h0 = static_cast<uint>(std::floor(static_cast<float>(height)/2));
	uint t = static_cast<uint>(std::ceil(static_cast<float>(height)/2));

	std::cout<<"Recursion         :"<<recursion<<std::endl;
	std::cout<<"Total Tree Height :"<<totalHeight<<"\n";
	std::cout<<"H0 height         :"<<h0<<"\n";
	std::cout<<"Rem height        :"<<t<<"\n";

	int h0nodes = static_cast<int>(std::pow(2, h0)) - 1;
	int remNodes = N - h0nodes;
	std::cout<<"H0 nodes          :"<<h0nodes<<std::endl;
	std::cout<<"Rem nodes         :"<<remNodes<<std::endl;
	
	// now calculate number of subtrees 
	int numSubtrees = static_cast<int>(std::pow(2, h0));
	int perSubtreeElement = remNodes/numSubtrees;
	std::cout<<"Num of Subtrees	  :"<<numSubtrees<<std::endl;
	std::cout<<"Per subtree cnt   :"<<perSubtreeElement<<std::endl;
	std::cout<<"----------------------------------------------------"<<std::endl;
	recursion++;
	// now for tree rooted at @h0, we launch one recursive function
	arrangeVEB(inputArray, outputArray, root, h0, t, h0nodes);
	// and for all subtrees @later height, we launch recursive functions
	for(int i = 0; i < numSubtrees; i++) {
		// find the root elements for all the subtrees
		int r = static_cast<int>(std::pow(2, t)) - 1;
		r += i;		//move appropriately along the dimension
		arrangeVEB(inputArray, outputArray, r, t, t+1, perSubtreeElement);
	}
}

void arrangeVEB2(int* inputArray, int* outputArray, uint root, uint level, int N) {

	static uint idx = 0;
	if(N <= 3) {
		// root is the idx. So use appropriately
		outputArray[idx++] = inputArray[root];
		/*
		if(N == 3) {
			int lchild = 2*root + 1;
			int rchild = 2*root + 2;
			outputArray[idx++] = inputArray[lchild];
			outputArray[idx++] = inputArray[rchild];
		}
		*/
		return;
	}
	// root indicates which root element idx we are working on subtree now
	// level indicates at what level of tree we are. We use a '1' indexed level. starting from 1.
	uint height = static_cast<uint>(std::log(N+1)/std::log(2));
	uint h0 = static_cast<uint>(std::floor(static_cast<float>(height)/2));
	uint rem = static_cast<uint>(std::ceil(static_cast<float>(height)/2));

	// IMPORTANT: Well the paper said that we cut in such a way that h0 always has a

	// if i split it at h0, what are the nodes present in h0+1?
	// so i will add that elements to the queue to be processed
	uint subTreeCnt = static_cast<uint>(std::pow(2, h0));
	uint h0nodes = static_cast<uint>(std::pow(2, h0)) - 1;
	uint perSubTreeCnt = (N-h0nodes)/subTreeCnt;
	// once for root
	arrangeVEB2(inputArray, outputArray, root, level, h0nodes);		// use the same level as the recursion is called on this
	// once for all subtrees
	for(uint i = 0; i < subTreeCnt; i++) {
		// we need one small offset because, when we recurse down, positions are determined by the recursive height and node length
		// hence we need to maintain that original position for calculating correct indices
		uint clevel = level + h0;
		uint offset = static_cast<uint>(std::pow(2, clevel-1)) - 1;		// these many nodes are actually present before me.!!
		//uint cidx = offset + i;
		uint cidx = 2*(root+1) + i;
		arrangeVEB2(inputArray, outputArray, cidx, clevel, perSubTreeCnt);
	}	
}

void arrangeVEB3(int* inputArray, int* outputArray, uint root, uint level, int N) {

	static uint idx = 0;
	if(N == 1) {
		outputArray[idx++] = inputArray[root];
		return;
	}
	
	uint height = static_cast<uint>(std::log(N+1)/std::log(2));
	uint h0, rem;
	if(height % 2 == 0) {
		h0 = height/2;
		rem = height - h0;
	} else {
		// for odd height trees, we split the tree with h0 = 1. i.e: we say root is of height 1
		h0 = 1;
		rem = height - 1;
	}

	uint subTreeCnt = static_cast<uint>(std::pow(2, h0));
	uint h0nodes = static_cast<uint>(std::pow(2, h0)) - 1;
	uint perSubTreeCnt = (N - h0nodes)/subTreeCnt;

	std::queue<uint> subTreeRoots;
	std::queue<uint> parents;
	uint tempRoot = root;
	uint cnt = 0;
	// at height h0, find all the subtree root indices starting from h0+1
	// go from root till h0 to get the correct left and right child
	do {
		uint lchild = 2*tempRoot + 1;
		uint rchild = 2*tempRoot + 2;
		parents.push(lchild);
		parents.push(rchild);
		tempRoot = parents.front();
		parents.pop();
		cnt++;
		if(cnt == h0) break;
	}while(!parents.empty());
	
	uint startIdx = static_cast<uint>(std::pow(2, level-1)) - 1;
	for(uint i = 0; i < subTreeCnt/2; i++) {
		// get the lchild and rchild of each subtree and then push them
		uint parent = startIdx + i;
		uint lchild = 2 * parent + 1;
		uint rchild = 2 * parent + 2;
		subTreeRoots.push(lchild);
		subTreeRoots.push(rchild);
	}
	
	// now start one recursion for the root
	arrangeVEB3(inputArray, outputArray, root, level, h0nodes);
	// start recursion for all subtrees at roots
	for(uint i = 0; subTreeRoots.size() > 0; i++) {
		// update the level
		uint clevel = level + h0;
		arrangeVEB3(inputArray, outputArray, subTreeRoots.front(), clevel, perSubTreeCnt);
		// pop out the used roor
		subTreeRoots.pop();
	}
}


int main()
{
	// allocate memory
	BST<uint> bst;
	bool written = true;
	if(!written) {

		std::default_random_engine generator;
		std::uniform_int_distribution<uint> distribution(0, UINT_MAX);
		double start, end, elapsedTime;
		std::cout<<"Filling Data for Elements \n";
		start = omp_get_wtime();
		//for(unsigned int i = 0; i < NUM_ELEMENTS; i++)
		//	keys[i] = distribution(generator);
		while(bst.getSize() != NUM_ELEMENTS) {
			// add such elements
			uint value = distribution(generator);
			bst.insert(value);
		}
		end = omp_get_wtime();
		elapsedTime = end - start;
		std::cout<<"Done Filling Data :\n Elapsed Time :"<<elapsedTime<<std::endl;

		uint* bfs = new uint[NUM_ELEMENTS];
		bst.bfs(bfs);

		// write that content to a file
	
		std::ofstream opfile("database.txt", std::ios::binary);
		uint val;
		for(uint i = 0; i < NUM_ELEMENTS; i++) {
			opfile<<bfs[i];
		}

		std::cout<<"\n";
		uint* veb = new uint[NUM_ELEMENTS];
		uint height = ilog2(NUM_ELEMENTS+1);
		for(uint i = 1; i <= NUM_ELEMENTS; i++) {
			uint vebIdx = bfs_to_veb(i, height);
			//std::cout<<vebIdx<<" ";
			// 1 offseted. so negate a 1
			veb[vebIdx-1] = bfs[i-1];
		}
		std::cout<<"\n";
		// write to output
		std::ofstream vebFile("veb_database.txt", std::ios::binary);
		for(uint i = 0; i < NUM_ELEMENTS; i++) {
			vebFile<<veb[i];
		}

	} else {
		// read file contents
		/*
		uint* bfs = new uint[NUM_ELEMENTS];
		std::ifstream ipfile("database.txt", std::ios::binary);
		for(uint i = 0; i < NUM_ELEMENTS; i++)
			ipfile>>bfs[i];
		*/
		// now arrange the content in van emde boas layout

		uint* veb = new uint[NUM_ELEMENTS];
		std::ifstream vebIpFile("veb_database.txt", std::ios::binary);
		for(uint i = 0; i < NUM_ELEMENTS; i++) {
			vebIpFile>>veb[i];
		}

		// ok now the linear array has the contents
		// we can perform searches on the van emde boas layout tree itself directly

	}
}



