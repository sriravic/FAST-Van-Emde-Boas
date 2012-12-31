#ifndef __BST_H__
#define __BST_H__

#pragma once
#include <queue>

// no duplicates please
template<typename T>
class BST
{
private:
  int size;
	//template<typename T>
	struct Node
	{
		T val;
		Node* parent;
		Node* left;
		Node* right;
		Node(T _val) { 
			parent = left = right = NULL;
			val = _val;
		}
		// comparison operators
		bool operator<= (const Node& N) { return val <= N.val; }
		bool operator> (const Node& N) { return val > N.val; }
		bool operator< (const Node& N) { return val < N.val; }
		bool operator== (const Node& N) { return val == N.val; }
	};
	Node* root;

	// private methods
	void inorderTraversal(Node* n, T* linearArray) {
		// left->root->right
		static int cnt = 0;
		if(n->left != NULL) inorderTraversal(n->left, linearArray);
		linearArray[cnt++] = n->val;
		if(n->right != NULL) inorderTraversal(n->right, linearArray);
	}
	T preorderTraversal(Node* n, T* linearArray) {
		// root->left->right
		static int cnt = 0;
		linearArray[cnt++] = n->val;
		if(n->left != NULL) preorderTraversal(n->left, linearArray);
		if(n->right != NULL) preorderTraversal(n->right, linearArray);
	}
	T postOrderTraversal(Node* n, T* linearArray) {
		// left->right->root
		static int cnt = 0;
		if(n->left != NULL) postOrderTraversal(n->left, linearArray);
		if(n->right != NULL) postOrderTraversal(n->right, linearArray);
		linearArray[cnt++] = n->val;
	}

	// some misc methods
	// gets the minimum node starting from some node
	Node* minimum(Node* n) {
		while(n->left != NULL) 
			n = n->left;
		return n;
	}
	Node* maximum(Node* n) {
		while(n->left != NULL)
			n = n->right;
		return n;
	}
	Node* successor(Node* n) {

		// check if right subtree is not null
		if(n->right != NULL) return minimum(n->right);
		// else climb up the tree to find the node that is left of the parent
		// INSHORT: climb up buddy
		Node* p = n->parent;
		while (p != NULL && p->right == n) {
			n = p;
			p = n->parent;
		}
		return p;
	}
	// replaces u's subtree with v's subtree
	// appropriate parent nodes are modified
	void transplant(Node* u, Node* v) {
		//1. First condition is checking if u is root
		//   which is given by u.parent == NULL
		if(u->parent == NULL) root = v;
		else if(u->parent->left == u) {
			u->parent->left = v;
		} else {
			u->parent->right = v;
		}
		// now v can be null also
		// so we change the pointer of v to parent
		if(v != NULL) v->parent = u->parent;
	}

	void free(Node* n) {
		// do a post order delete
		if(n->left != NULL) free(n->left);
		if(n->right != NULL) free(n->right);
		delete n;
	}
public:
	BST() {
		size = 0;
		root = NULL;
	}
	~BST() {
		// do an post order traversal and delete the nodes
		free(root);
	}

	void insert(T val) {
		if(root == NULL) {
			// create a root node first
			root = new Node(val);
			size++;
		} else {
			// go to the appropriate location and then insert
			Node* toInsert = new Node(val);
			Node* node, *parent;
			// init please
			node = root; 
			parent = root;
			do {
				if(*toInsert < *node) {
					parent = node;
					node = node->left;
				} else if(*toInsert > *node) {
					parent = node;
					node = node->right;
				} else {
					// duplicates 
					// return immediately
					return;
				}
			} while (node != NULL);
			// ok corrent parent reached
			// insert left or right
			if(*toInsert < *parent) {
				parent->left = toInsert;
				toInsert->parent = parent;
			} else {
				parent->right = toInsert;
				toInsert->parent = parent;
			}
			size++;
		}
	}
	// deletes the first occurence of the node
	void remove(T val) {
		// first we have to find the node
		Node* toRemove = find(val);
		// first we check if left subtree is null
		// if so we replace the node with node->right
		if(toRemove->left == NULL) {
			transplant(toRemove, toRemove->right);
			size--;
		}
		else if(toRemove->right == NULL) {
			transplant(toRemove, toRemove->left);
			size--;
		}
		// now we have both subtrees
		// so find inorder successor
		// which is minimum rooted at this node->right. and then we have to replace
		else {
			Node* inorderSuccessor = successor(toRemove->right);
			// check if immediate successor
			// if not, then replace, inordersuccessor, by its right node
			if(inorderSuccessor->parent != toRemove) {
				transplant(inorderSuccessor, inorderSuccessor->right);
				inorderSuccessor->right = toRemove->right;
				inorderSuccessor->right->parent = inorderSuccessor;
			}
			// do the left tree
			transplant(toRemove, inorderSuccessor);
			inorderSuccessor->left = toRemove->left;
			inorderSuccessor->left->parent = inorderSuccessor;
			size--;
		}

	}
	// removes all entry from the binary tree and sets size to zero.
	// essentially similar to destructor, but without actually destroying stuff
	void clear() {
		// call free
		free(root);
		size = 0;
	}
	bool bFind(T val) {
		Node* node = root;
		Node* toFind = new Node(val);
		while(node != NULL) {
			if(*toFind < *node) node = node->left;
			else if(*toFind > *node) node = node->right;
			else if(*toFind == *node) {
				delete toFind;
				return true;
			}
		}
		return false;
	}
	Node* find(T val) {
		Node* node = root;
		Node* toFind = new Node(val);
		while(node != NULL) {
			if(*toFind < *node) node = node->left;
			else if(*toFind > *node) node = node->right;
			else if(*toFind == *node) {
				delete toFind;
				return node;
			}
		}
		return NULL;
	}
	void inorder(T* linearArray) {
		// prints the tree in order fashion
		// fills out the linear array. Assumes linear array to contain enuf memory to hold all elements of tree
		inorderTraversal(root, linearArray);
	}
	void preorder(T* linearArray) { preorderTraversal(root, linearArray); }
	void postOrder(T* linearArray) { preorderTraversal(root, linearArry); }
	void bfs(T* linearArray) {
		std::queue<Node*> bfsQ;
		unsigned int count = 0;
		//push root first
		bfsQ.push(root);
		while(!bfsQ.empty()) {
			// get the left and right
			Node* node = bfsQ.front();
			// insert element into linear array
			if(node != NULL) linearArray[count++] = node->val;
			// now check for left and right
			if(node->left != NULL) bfsQ.push(node->left);
			if(node->right != NULL) bfsQ.push(node->right);
			// pop the element just used
			bfsQ.pop();
		}
	}
	int getSize() { return size; }

};



#endif
