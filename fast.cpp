#include <iostream>
#include <vector>
#include <xmmintrin.h>
#include <algorithm>
#include "fast.h"

// tree construction code

// given a node, this function finds the number of nodes in all pageblocking subtrees before this subtree. It also retrieves the current nodes pageblocking 
// subtree's root idx. It also returns the corresponding height of the node's subtree's pageBlock height
// It returns the subtree number from the top
uint pageBlocking(uint node, const uint& root, const uint& height, uint& numPrePageBlockNodes, uint& pageRootIdx, uint& pageBlockingHeight) {
  // Ok now proceed.
	uint depth = findNodeDepth(node, root, height);
	uint pageLevel = depth / dP;
	uint numSubtrees = 0;
	for(uint p = 0; p < pageLevel; p++) {
		// find the number of subtrees at this level
		uint levelSubtrees = (1 << (p*dP));
		numSubtrees += levelSubtrees;
	}
	
	// now we are node level with all the subtrees. But note that node can be any where from 0th subtree to n-1 subtree. So we find the actual 
	// subtree and add that the numSubtrees. 
	// If a complete tree with an exact multiple of dP levels, then we just have to multiply n*2^dP nodes for all levels above this node's pageBlock level
	// Note: complete applies to the subtree within which the node is actually a complete subtree or partial subtree
	uint numNodesAbove = numSubtrees * ((1 << dP)-1);
	uint nearestParentLevel = pageLevel * dP;
	uint startingNode = (root >> depth);
	bool complete = ((nearestParentLevel + (dP - 1)) <= height);
	// find which subtree this is
	// formula: (nodeNum - startNodeAtDepth)/2^(totalTreeHeight-depth+1)
	// subtree = (above_val)/2^(depth-pageRootLevel) 
	uint subTreeNum = (node - startingNode)/(1<<(height-depth+1));			// this is subtree num at current tree depth - 1
	subTreeNum /= (1 << (depth - nearestParentLevel));
	// now subTree num contains the subTree from the left --> right (0 indexed)
	// so we need not negate a '1' that would have resulted from the actual subtree itself.
	if(complete) {
		numSubtrees += subTreeNum;
		numPrePageBlockNodes = numSubtrees * ((1 << dP) - 1);
		pageBlockingHeight = dP - 1;
	} else {
		// find the modulo height of the page's 
		uint residue = height % dP;
		// so subTreeNum * 2^(residue) nodes are to be added
		numPrePageBlockNodes += subTreeNum * ((1 << (residue+1)) - 1) + numNodesAbove;
		pageBlockingHeight = residue;			// Note: I am not sure of this.!! well this can be 0 indexed or 1 indexed. I have to check this out.!!
	}

	// assign the return variables
	pageRootIdx = (root >> nearestParentLevel) + subTreeNum * (1 << (height - nearestParentLevel + 1));
	return numSubtrees;
}

uint cacheBlocking(uint node, const uint& root, const uint& height, const uint& pageRootIdx, const uint& pageBlockingHeight, uint& numPreCacheBlockNodes, uint& cacheRootIdx, uint& cacheBlockingHeight) { 

	// ok we have the local page root, page height
	// we have to proceed ahead in the same way as page, but locally, all stuff changes
	uint depth = findNodeDepth(node, root, height);
	uint rootDepth = findNodeDepth(pageRootIdx, root, height);
	uint distance = depth - rootDepth;		// from local page root to node
	// this distance is the height we have to split into dL levels
	uint cacheLevel = distance / dL;
	uint numSubtrees = 0;

	// within this pageHeight, find number of subtrees of cache line size
	// so we go dL levels. check if there are enough levels to form subtrees
	for(uint c = 0; c < cacheLevel; c++) {
		// find the number of subtrees at this level
		uint levelSubtrees = (1 << (c*dL));
		numSubtrees += levelSubtrees;
	}
	uint numNodesAbove = numSubtrees * ((1 << dL)-1);
	uint nearestParentLevel = rootDepth + cacheLevel * dL;
	
	// new code
	uint sibling, child1, child2, parent;
	uint startingNode, subTreeNum, tempNode;
	uint currentDepth = depth;
	tempNode = parent = node;
	while(currentDepth != nearestParentLevel) {
		uint offset = 1 << (height-currentDepth + 1);
		startingNode = (root >> currentDepth);
		subTreeNum = (tempNode - startingNode) / offset;
		sibling = (subTreeNum % 2 == 0) ? tempNode + offset : tempNode - offset;
		child1 = tempNode, child2 = sibling;
		parent = (child1 + child2) >> 1;
		// update depth
		currentDepth--;
		// update current node
		tempNode = parent;
	}

	uint start = rootDepth;
	tempNode = pageRootIdx;
	while(start != currentDepth) {
		uint offset = 1 << (height - start - 1);
		tempNode -= offset;
		start++;
	}
	// so tempNode is the starting node of our nodes, level and subtree of the many subtrees
	subTreeNum = (parent - tempNode)/(1 << (height-currentDepth+1));
	//bool complete = ((nearestParentLevel % dP) + dL - 1) <= pageBlockingHeight;
	uint nearestParentLevelLocal = nearestParentLevel - rootDepth;
	bool complete = (nearestParentLevelLocal + dL) <= pageBlockingHeight;

	//bool complete = ((nearestParentLevel + (dL - 1)) <= height);
	if(complete) {
		numSubtrees += subTreeNum;
		numPreCacheBlockNodes = numSubtrees * ((1 << dL) - 1);
		cacheBlockingHeight = dL - 1;
	} else {
		uint residue = pageBlockingHeight % dL;
		numPreCacheBlockNodes += subTreeNum * ((1 << (residue+1)) - 1) + numNodesAbove;
		cacheBlockingHeight = residue;	
	}
	//cacheRootIdx = (root >> nearestParentLevel) + subTreeNum * (1 << (height - nearestParentLevel + 1));
	cacheRootIdx = parent;
	return numSubtrees;
}

// given a cache blocking structure, find the appropriate number for the simd stuff.
// NOTE: this method returns the simd line blocking from the top
// NOTE: the height and everything is local to the cache level subtree
uint simdBlocking(uint node, const uint& root, const uint& height, const uint& cacheRootIdx, const uint& cacheBlockingHeight, uint& numPreSimdBlockNodes, uint& simdRootIdx, uint& simdBlockingHeight) {

	// now proceed ahead with the same kind of stuff as before
	uint depth = findNodeDepth(node, cacheRootIdx, height);
	uint rootDepth = findNodeDepth(cacheRootIdx, root, height);
	uint distance = depth - rootDepth;
	uint simdLevel = distance / dK;
	uint numSubtrees = 0;
	for(uint s = 0; s < simdLevel; s++) {
		// find the number of subtrees at this level
		uint levelSubtrees = (1 << (s*dK));
		numSubtrees += levelSubtrees;
	}
	
	uint numNodesAbove = numSubtrees * ((1 << dK)-1);
	uint nearestParentLevel = rootDepth + simdLevel * dK;
	//uint startingNode = (root >> depth);
	//uint subTreeNum = (node - startingNode)/(1<<(height-depth+1));			
	
	// find the sibling of this node
	uint sibling, child1, child2, parent;
	uint startingNode, subTreeNum, tempNode;
	uint currentDepth = depth;
	tempNode = parent = node;
	while(currentDepth != nearestParentLevel) {
		uint offset = 1 << (height-currentDepth + 1);
		startingNode = (root >> currentDepth);
		subTreeNum = (tempNode - startingNode) / offset;
		sibling = (subTreeNum % 2 == 0) ? tempNode + offset : tempNode - offset;
		child1 = tempNode, child2 = sibling;
		parent = (child1 + child2) >> 1;
		// update depth
		currentDepth--;
		// update current node
		tempNode = parent;
	}
	
	// now we are at correct level
	// tempNode will have the ancestor of our node
	// now we go down again to the starting node of our node depth and then we find the subTreeNum
	// from cacherootIdx, we go down (currentDepth-cacheRootDepth) times from cache root
	uint start = rootDepth;
	tempNode = cacheRootIdx;
	while(start != currentDepth) {
		uint offset = 1 << (height - start - 1);
		tempNode -= offset;
		start++;
	}
	
	// so tempNode is the starting node of our nodes, level and subtree of the many subtrees
	subTreeNum = (parent - tempNode)/(1 << (height-currentDepth+1));
	// end of logic
	//bool complete = ((nearestParentLevel + (dK - 1)) <= height);
	//bool complete = ((nearestParentLevel % dL) + dK - 1) <= cacheBlockingHeight;
	
	uint nearestParentLevelLocal = nearestParentLevel - rootDepth;
	bool complete = (nearestParentLevelLocal + dK) <= cacheBlockingHeight;
	if(complete) {
		numSubtrees += subTreeNum;
		numPreSimdBlockNodes = numSubtrees * ((1 << dK) - 1);
		simdBlockingHeight = dK;
	} else {
		uint residue = cacheBlockingHeight % dK;
		numPreSimdBlockNodes += subTreeNum * ((1 << (residue+1)) - 1) + numNodesAbove;
		simdBlockingHeight = residue;	
	}
	
	//simdRootIdx = (root >> nearestParentLevel) + subTreeNum * (1 << (height - nearestParentLevel + 1));
	simdRootIdx = parent;
	return subTreeNum;
}

// The star of the code..!!
// convert inorder index to blocking structure
uint inorderToFast(uint node, uint root, uint height) {

	// get the number of variables that are to be used for all levels
	uint blockingIdx = 0;			// final value that we send out
	uint numPrePageBlockNodes = 0, pageRootIdx = 0, pageBlockingHeight = 0;
	uint numPreCacheBlockNodes = 0, cacheRootIdx = 0, cacheBlockingHeight = 0;
	uint numPreSimdBlockNodes = 0, simdRootIdx = 0, simdBlockingHeight = 0;

	pageBlocking(node, root, height, numPrePageBlockNodes, pageRootIdx, pageBlockingHeight);
	cacheBlocking(node, root, height, pageRootIdx,  pageBlockingHeight, numPreCacheBlockNodes, cacheRootIdx, cacheBlockingHeight);
	simdBlocking(node, root, height, cacheRootIdx, cacheBlockingHeight, numPreSimdBlockNodes, simdRootIdx, simdBlockingHeight);
	blockingIdx = numPrePageBlockNodes + numPreCacheBlockNodes + numPreSimdBlockNodes;
	uint offset = (node == simdRootIdx ? 0 : (node < simdRootIdx ? 1 : 2));
	blockingIdx += offset;
	return blockingIdx;
}


// define a lookup table
// table structure is as follow
/**
 000	- 0
 100	- N/a
 010	- 1
 110	- 2
 001	- N/A
 101	- N/A
 011	- N/A
 111	- 3
*/
const uint LUT[] = { 0, -1, 1, -1, -1, -1, 2, 3 };

// Tree traversal code
// updated with comments as understanding increases
// IMPORTANT: Are we supposed to have only signed values?? this imposes a strict ordering in SSE?

bool traverseTree(Tuple* tuples, uint* keys, const uint& key1, const uint& key2) {

	// a number of page constants are required. We will hardcode them as of now. But later we will add a structure that contains all of them
	
	__m128i vKeyq = _mm_set1_epi32(key1);
	__m128i* tree = (__m128i*)keys;			

	// in the traversal code, we have two variables that define the number of pages in the tree, and number of cache lines within a page
	// WARNING: HARD CODED VALUES BELOW..!!!
	/****************************************/
	uint numPages = 1;
	uint numCacheLevelsPerPage = 1;
	uint childOffset = 0, cacheOffset = 0, pageOffset, pageAddress;
	for(uint i = 0; i < numPages; i++) {
		pageOffset = 0;
		pageAddress = 0;				// compute which page the child is in actually.! .. this code we have to figure it out.
		for(uint j = 0; j < numCacheLevelsPerPage; j++) {
			// get the SIMD block
			__m128i vTree = _mm_load_si128(tree + pageAddress + pageOffset);		// aligned to 16byte boundary
			// create a mask
			__m128i mask = _mm_cmpgt_epi32(vKeyq, vTree);
			// convert the mask into an index
			uint index = _mm_movemask_ps(_mm_castsi128_ps(_mm_shuffle_epi32(mask, _MM_SHUFFLE(0, 1, 2, 3))));
			// right shift the index, because we dont want the last bit, we take into consideration only the 3 comparison operations
			index >>= 1;
			uint childIndex = LUT[index];

			// load the second level of SIMD blocking subtree
			uint offset = pageAddress + pageOffset + Nk*childIndex;
			vTree = _mm_load_si128(tree + 32);
			mask = _mm_cmpgt_epi32(vKeyq, vTree);
			index = _mm_movemask_ps(_mm_castsi128_ps(mask));
			// update cache offset and page offset
			cacheOffset = childIndex * 4 + LUT[index];
			pageOffset = pageOffset * 16 + cacheOffset;
		}
		// calculate child offset
		childOffset = childOffset * (1<<dP) + pageOffset;
	}

	// for range queries
	while(tuples[childOffset].key <= key2)
		childOffset++;

	return true;

}


