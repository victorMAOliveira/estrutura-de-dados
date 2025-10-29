/**
 * @file huffman.c
 * @authors Victor Amaral, Lorenzo Holanda, Lucitanea Lopes
 * @date 28/10/2025
 * @version 0.1
 * @details Implementation of Huffman's compression algorithm in C
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME_SIZE 200
#define ASCII_SIZE 256

/**
 * @brief - Node used for the main operations of the algorithm
 * @brief - It is used for the linked list process
 * @brief - It is used for the binary tree process
 * @param element Points to the element that repeats itself throughout the
 * file used in the program
 * @param frequency Points to the frequency in which said element repeats
 * itself in the file
 * @param next Points to the next node in the linked list
 * @param left Points to this node's left child
 * @param right Points to this node's right child
 * @note - Create it using create_node()
 * @note - Destroy an instance of it using destroy_node()
 */
typedef struct node {
  void* element;
  size_t frequency;
  struct node* next;
  struct node* left;
  struct node* right;
} node_t;

/**
 * @brief - Gets the name of the file to be used
 * @return - Name of the file
 * @warning - Exits program if not able to allocate memory for the string
 * @warning - Allocates memory for the string that needs to be freed after use
 * @category UTILITIES
 */
char* get_file_name() {
  // Allocate memory for string
  char* file_name = malloc(FILE_NAME_SIZE * sizeof(char));
  if (file_name == NULL) {
    perror("ERR: could not allocate memory in get_file_name()\n");
    exit(EXIT_FAILURE);
  }

  // Get user input for the name
  printf("\nEnter the file name: ");
  scanf(" %s", file_name);

  // Return the name of the file
  return file_name;
}

/**
 * @brief - Opens file with the given name and reads its contents
 * @return - An unsigned char string containing the file content
 * @warning - Exits program if file is not opened
 * @warning - Exits program if memory could not be allocated
 * @warning - Allocates memory for the return string that has to be freed later
 * @category UTILITIES
 */
unsigned char* get_file_content(const char* file_name, size_t* file_size) {
  // Open file for reading
  FILE* file = fopen(file_name, "rb");
  if (file == NULL) {
    perror("ERR: could not open file in get_file_contents()\n");
    exit(EXIT_FAILURE);
  }

  // Get file size
  fseek(file, 0, SEEK_END);
  *file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  // Allocate memory for the contents
  unsigned char* content = malloc((*file_size + 1) * sizeof(unsigned char));
  if (!content) {
    perror("ERR: could not allocate memory in get_file_content()\n");
    exit(EXIT_FAILURE);
  }

  // Read the file content into the buffer
  fread(content, sizeof(unsigned char), *file_size, file);
  content[*file_size] = '\0';

  // Close file and return the content
  fclose(file);
  return content;
}

/**
 * @brief - Function for printing the file content
 * @param content File content
 * @category DEBUG
 */
void print_content(const unsigned char* content) {
  printf("file content:\n%s\n", content);
}

/**
 * @brief - Iterates through file and gets the frequency of every byte in it
 * @param content Content of the file
 * @param file_size Size of the file
 * @return A pointer to the table holding the byte frequencies
 * @warning Exits program if memory could not be allocated
 * @category ALGORITHM
 */
size_t* get_frequencies(const unsigned char* content, const size_t file_size) {
  // Allocate memory for the frequencies table
  size_t* frequencies = calloc(ASCII_SIZE, sizeof(size_t));
  if (frequencies == NULL) {
    perror("ERR: could not allocate memory in get_frequencies()\n");
    exit(EXIT_FAILURE);
  }

  // Get frequencies of every byte in the file by accessing its corresponding
  // index in the frequencies table
  for (size_t byte = 0; byte < file_size; byte++) {
    frequencies[content[byte]]++;
  }

  // Return formed frequencies table
  return frequencies;
}

/**
 * @brief - Prints the frequencies table
 * @param frequencies The frequencies table
 * @category DEBUG
 */
void print_frequencies(const size_t* frequencies) {
  printf("frequencies:\n");
  for (unsigned int index = 0; index < ASCII_SIZE; index++) {
    if (frequencies[index] > 0) {
      printf("\t%c: %zu\n", index, frequencies[index]);
    }
  }
}

/**
 * @brief Adds new element to the ordered list
 * @param element Element being added to the list
 * @param frequency Frequency in which the element appears in the file
 * @param head Head of the current list
 * @return The head to the updated list
 * @warning Allocates memory for a new node
 * @category ALGORITHM
 */
node_t* add_to_list_ordered(node_t* head, node_t* new_node) {
  // Insertion process
  if (head == NULL) {
    // List is empty, new node becomes head
    head = new_node;
  } else if (head->frequency > new_node->frequency) {
    // New node goes to the start of the list
    new_node->next = head;
    head = new_node;
  } else {
    // New node goes to the middle or end of the list
    node_t* aux = head;
    while (aux->next && aux->next->frequency < new_node->frequency) {
      aux = aux->next;
    }
    new_node->next = aux->next;
    aux->next = new_node;
  }

  // Returns head to updated list
  return head;
}

/**
 * @brief - Adds all needed elements to a ordered linked list structure
 * @param frequencies The array that holds all char frequencies
 * @return - The head of the linked list
 * @warning - Allocates memory for every different node used
 * @category ALGORITHM
 */
node_t* create_list(const size_t* frequencies) {
  // Node that will represent the head of the list
  node_t* head = NULL;

  // Iterates by every char in the ascii table
  for (unsigned int index = 0; index < ASCII_SIZE; index++) {
    // Only adds elements to the list that appear in the file
    if (frequencies[index] > 0) {
      // Allocates memory for new node and defines its variables to standard
      // values
      node_t* new_node = malloc(sizeof(node_t));
      new_node->element = malloc(sizeof(unsigned char));
      *(unsigned char*)new_node->element = index;
      new_node->frequency = frequencies[index];
      new_node->next = NULL;
      new_node->left = NULL;
      new_node->right = NULL;

      // Adds new node to the list in an ordered manner
      head = add_to_list_ordered(head, new_node);
    }
  }

  // Returns the head at the proper location
  return head;
}

/**
 * @brief Function for printing the linked list
 * @param head The head of the linked list
 * @category DEBUG
 */
void print_list(node_t* head) {
  printf("linked list:\n");
  while (head) {
    printf("\t%c: %zu\n", *(unsigned char*)head->element, head->frequency);
    head = head->next;
  }
}

/**
 * @brief - Fuses the first two nodes of the list into one for the tree
 * creation step
 * @param head Head of the current list
 * @return Head of the updated list
 * @warning - Allocates memory for the new node created
 * @category ALGORITHM
 */
node_t* fuse_nodes(node_t* head) {
  // Head must have at least two nodes
  node_t* first = head;
  node_t* second = head->next;
  node_t* rest = second->next;  // Remainder of the list after the two smallest

  // Create new internal node
  node_t* new_node = malloc(sizeof(node_t));
  if (new_node == NULL) {
    perror("ERR: could not allocate memory in fuse_nodes()\n");
    exit(EXIT_FAILURE);
  }
  new_node->element = malloc(sizeof(unsigned char));
  if (new_node->element == NULL) {
    perror("ERR: could not allocate memory for element in fuse_nodes()\n");
    exit(EXIT_FAILURE);
  }
  *(unsigned char*)new_node->element = '*';  // Element for non-leaf nodes
  new_node->frequency = first->frequency + second->frequency;
  new_node->left = first;
  new_node->right = second;
  new_node->next = NULL;

  // Detach first and second from the remaining list
  first->next = NULL;
  second->next = NULL;

  // Insert the fused node into the remaining list (rest), preserving order
  head = add_to_list_ordered(rest, new_node);
  return head;
}

/**
 * @brief - Creates the huffman tree from the linked list
 * @param head Head of the current list
 * @return Root of the created tree
 * @category ALGORITHM
 */
node_t* create_tree(node_t* head) {
  if (head == NULL) {
    perror("ERR: empty list in create_tree()\n");
    exit(EXIT_FAILURE);
  }

  // While the list has at least 2 elements
  while (head->next != NULL) {
    // Fuses the first two nodes into one for the tree step
    head = fuse_nodes(head);
  }

  // Returns the single remaining node (root)
  return head;
}

/**
 * @brief - Frees the tree nodes (recursively)
 * @param root Root of the tree
 * @category CLEANUP
 */
void destroy_tree(node_t* root) {
  if (root == NULL) return;
  destroy_tree(root->left);
  destroy_tree(root->right);
  if (root->element) free(root->element);
  free(root);
}

/**
 * @brief - Prints the tree structure (recursively)
 * @param root Root of the tree
 * @param depth Current depth in the tree
 * @category DEBUG
 */
void print_tree(node_t* root, int depth) {
  if (root == NULL) return;

  printf("\t%c: %zu (depth: %d)\n", *(unsigned char*)root->element,
         root->frequency, depth);

  print_tree(root->left, depth + 1);
  print_tree(root->right, depth + 1);
}

/**
 * @brief - Gets the height of the tree (recursively)
 * @param root Root of the tree
 * @return Height of the tree
 * @category DEBUG
 */
int get_tree_height(node_t* root) {
  if (root == NULL) return -1;

  int left_height = get_tree_height(root->left);
  int right_height = get_tree_height(root->right);

  return (left_height > right_height ? left_height : right_height) + 1;
}

/**
 * @brief - Generates the binary codes for each element in the tree
 * @param root Root of the tree
 * @param codes Array to store the generated codes
 * @param current_code Current code being formed
 * @param depth Current depth in the tree
 * @category ALGORITHM
 */
void generate_codes(node_t* root, char** codes, char* current_code, int depth) {
  if (root == NULL) return;

  // If it's a leaf node, store the code
  if (root->left == NULL && root->right == NULL) {
    current_code[depth] = '\0';  // Null-terminate the string
    strcpy(codes[*(unsigned char*)root->element], current_code);
    return;
  }

  // Traverse left
  current_code[depth] = '0';
  generate_codes(root->left, codes, current_code, depth + 1);

  // Traverse right
  current_code[depth] = '1';
  generate_codes(root->right, codes, current_code, depth + 1);
}

/**
 * @brief - Prints the generated codes
 * @param codes Array holding the generated codes
 * @category DEBUG
 */
void print_codes(char** codes) {
  printf("Huffman Codes:\n");
  for (int i = 0; i < ASCII_SIZE; i++) {
    if (codes[i][0] != '\0') {
      printf("\t%c: %s\n", i, codes[i]);
    }
  }
}

int main() {
  // Declaring variables for the program
  int mode;
  char* file_name;
  unsigned char* content;
  size_t file_size;
  size_t* frequencies;
  node_t* head;
  node_t* root;
  int tree_height;
  char** codes;

  // Deciding if the user wants to either compress or extract the given file
  printf("Press the key of the function you want to use:\n");
  printf("\t1. Compress file\n");
  printf("\t2. Extract file\n");
  scanf("%d", &mode);

  // Getting the file, its content and its size
  file_name = get_file_name();
  content = get_file_content(file_name, &file_size);

  if (mode == 1) {
    // Setting up element frequencies
    frequencies = get_frequencies(content, file_size);

    // Setting up the linked list
    head = create_list(frequencies);

    // Setting up the huffman tree
    root = create_tree(head);
    printf("Huffman Tree:\n");
    print_tree(root, 0);

    // Getting tree height
    tree_height = get_tree_height(root);
    printf("Tree height: %d\n", tree_height);

    // Generating codes
    codes = malloc(ASCII_SIZE * sizeof(char*));
    for (int i = 0; i < ASCII_SIZE; i++) {
      codes[i] = calloc(tree_height + 1, sizeof(char));
    }
    char* current_code = calloc(tree_height + 1, sizeof(char));
    generate_codes(root, codes, current_code, 0);
    print_codes(codes);

    // Free tree and leftover memory
    destroy_tree(root);
    for (int i = 0; i < ASCII_SIZE; i++) {
      free(codes[i]);
    }
    free(codes);
    free(current_code);
    free(content);
    free(file_name);
    free(frequencies);
  }

  return 0;
}