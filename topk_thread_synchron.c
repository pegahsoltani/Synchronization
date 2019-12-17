#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/*
You will now develop a program that will have N worker threads created by the main
thread. The program will have a shared binary search tree (pointed by a global
variable) with size at most K nodes. The tree is shared among all threads. The tree
will be implemented with pointers, not using an array. The tree will keep the K
largest integers seen so far in the input files. Each worker thread will process an
input file of integers (one integer per line). There will be N input files. Each such
worker thread will read an input file one integer at a time. This time we will read with
fscanf (easier). After reading an integer from the input file, the worker thread will
insert it into the binary search tree if necessary, otherwise it will discared the integer.
Then it will read the next integer from the file. All N worker threads will work like this
concurrently. When all worker threads finish, we will have the K largest integers seen
in those N files in the binary tree. Then the main thread will print them out in sorted
decreasing order to an output file file. You need to implement the tree youself. You
can not use a library or an existing code for this purpose.
*/

typedef struct node Node;
int sizeOfTree;
struct node{
  int value;
  Node* left;
  Node* right;
};
// global variables
void *processInputFile(void*);
Node* insert(Node*, int);
Node* deleteNode(Node*);
void inorderTraversal(Node*, FILE*);
Node* findMinValueParent(Node*);
char** globalArgv;
pthread_mutex_t lock;
int sizeOfTree = 0;
Node* tree = NULL;
int K;
// struct node part



// insert function
Node* insert(Node *node, int value){
  // create the node and insert

  if(node == NULL){
    // create the bst here
    node = (Node*)malloc(sizeof (Node));
    // set the left right children to NULL
    node->value = value;
    node->left = NULL;
    node->right = NULL;
    sizeOfTree ++;
    return node;
  }
  if(value < node->value){
    node->left = insert(node->left , value);
    return node;
  }
  else{
    node->right = insert(node->right, value);
    return node;
  }
}

// for traversing the tree
void inorderTraversal(Node* root, FILE* fp){
  if(root==NULL){
    return;
  }
  inorderTraversal(root->right,fp);
  fprintf(fp,"%d\n", root->value);
  inorderTraversal(root->left,fp);
}

// works correctly
Node* findMinValue(Node* root){
  // traverse the tree from the root return the node with the smallest value
  Node* current = root;
  while(current != NULL && current->left != NULL ){
    current = current->left;
  }
  return current;
}


// delete function
Node* deleteNode(Node* root){
  // base case
  if(root == NULL){
    return root;
  }
  // find the smallest node
  Node* current = findMinValue(root);
  int minValue = current->value;
  if(minValue< root->value){
    root->left = deleteNode(root->left);
  }
  else if(minValue > root->value){
    root->right = deleteNode(root->right);
  }
  else{
    if(root->left == NULL){
      Node *temp = root->right;
      free(root);
      sizeOfTree--;
      return temp;
    }
    else if (root->right == NULL) {
      Node *temp = root->left;
      free(root);
      sizeOfTree--;
      return temp;
    }
  }
}



// main function
int main(int argc, char* argv[]){
  globalArgv = argv;
  K = atoi(argv[1]); // tree size
  int N = atoi(argv[2]); // getting the number of threads (files)
  int threadIndices[N];
  pthread_t threads[N];

  for(int threadIndex = 0; threadIndex < N; threadIndex++){
    threadIndices[threadIndex] = threadIndex;
    pthread_create(&threads[threadIndex], NULL, processInputFile, &threadIndices[threadIndex]);
  }
  for(int threadIndex = 0; threadIndex < N; threadIndex++){
    pthread_join(threads[threadIndex], NULL);
  }
//  printf("before traversal\n");
  int array[K];
  FILE* fp = fopen(argv[N+3], "w");
  inorderTraversal(tree, fp);
  fflush(fp);
  fclose(fp);
  //printf("\n");
  return 0;
}

void *processInputFile(void* arg){
  int threadIndex = *((int*)arg);
  FILE* inputFile = fopen(globalArgv[threadIndex + 3], "r");
  if(inputFile == NULL){
    return NULL;
  }
  int number;
  // reading the file
  while(fscanf(inputFile, "%d", &number) == 1){

    pthread_mutex_lock(&lock);
    if(sizeOfTree<K){
      tree = insert(tree, number);
    }
    else {
      Node* smallest = findMinValue(tree);
      if(smallest->value < number){
        tree = deleteNode(tree);
        tree = insert(tree, number);
      }
    }
    pthread_mutex_unlock(&lock);
  }

  // readNumbers(inputFile);
  // int number;
  // fscanf(inputFile, "%d", &number); // test
  // printf("%d %d\n", threadIndex, number);
  // printf("%d\n", threadIndex); // thread numbers
}
