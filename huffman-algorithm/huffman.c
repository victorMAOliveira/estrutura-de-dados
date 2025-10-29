/**
 * @file huffman.c
 * @authors Victor Amaral, Lorenzo Holanda, Lucitanea Lopes
 * @date 28/10/2025
 * @version 0.1
 * @details implementation of huffman's compression algorithm in C
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME_SIZE 200
#define ASCII_SIZE 256

/**
 * @brief - node used for the main operations of the algorithm
 * @brief - it is used for the linked list process
 * @brief - it is used for the binary tree process
 * @param element points to the element that repeats itself throughout the
 * file used in the program
 * @param frequency points to the frequency in which said element repeats
 * itself in the file
 * @param next points to the next node in the linked list
 * @param left points to this node's left child
 * @param right points to this node's right child
 * @note - create it using create_node()
 * @note - destroy an instance of it using destroy_node()
 */
typedef struct node {
  void* element;
  size_t frequency;
  struct node* next;
  struct node* left;
  struct node* right;
} node_t;

/**
 * @brief - gets the name of the to be used file
 * @return - name of the file
 * @warning - exits program if not able to allocate memory for the string
 * @warning - allocates memory for the string that needs to be freed after use
 * @category UTILITIES
 */
char* get_file_name() {
  // allocate memory for string
  char* file_name = malloc(FILE_NAME_SIZE * sizeof(char));
  if (!file_name) {
    perror("ERR: could not allocate memory in get_file_name()\n");
    exit(EXIT_FAILURE);
  }

  // gets user input for the name
  printf("Enter the file name: ");
  scanf("%s", file_name);

  // returns the name of the file
  return file_name;
}

/**
 * @brief - opens file with the name given and reads its contents
 * @return - a uchar string with the file content
 * @warning - exits program if file is not opened
 * @warning - exits program if memory could not be allocated
 * @warning - allocates memory for the return string that has to be freed later
 * @category UTILITIES
 */
unsigned char* get_file_content(const char* file_name, size_t* file_size) {
  // opens file for reading
  FILE* file = fopen(file_name, "rb");
  if (!file) {
    perror("ERR: could not open file in get_file_contents()\n");
    exit(EXIT_FAILURE);
  }

  // get file size
  fseek(file, 0, SEEK_END);
  *file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  // allocate memory fot the contents
  unsigned char* content = malloc((*file_size + 1) * sizeof(unsigned char));
  if (!content) {
    perror("ERR: could not allocate memory in get_file_content()\n");
    exit(EXIT_FAILURE);
  }

  // read the file content into the string
  fread(content, *file_size, sizeof(unsigned char), file);
  content[*file_size] = '\0';

  // closes file and returns the content
  fclose(file);
  return content;
}

/**
 * @brief - function for printing the file content
 * @param content content of the file
 * @category DEBUG
 */
void print_content(const unsigned char* content) {
  printf("file content:\n%s\n", content);
}

/**
 * @brief - iterates through file and gets the frequency of every byte in it
 * @param content content of the file
 * @param file_size size of the file
 * @return a pointer to the table holding the byte frequencies
 * @warning exits program if memory could not be allocated
 * @category ALGORITHM
 */
size_t* get_frequencies(const unsigned char* content, const size_t file_size) {
  // allocate memory for the frequencies list
  size_t* frequencies = calloc(ASCII_SIZE, sizeof(size_t));
  if (!frequencies) {
    perror("ERR: could not allocate memory in get_frequencies()\n");
    exit(EXIT_FAILURE);
  }

  //  gets frequencies of every byte of file by accessing its correspondent
  //  integer value in the frequencies table
  for (size_t byte = 0; byte < file_size; byte++) {
    frequencies[content[byte]]++;
  }

  // returns formed frequencies table
  return frequencies;
}

/**
 * @brief function for printing the frequencies table
 * @param frequencies the frequencies table
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
 * @brief adds new element to the ordered list
 * @param element element being added to the list
 * @param frequency frequency in which the element appears in the file
 * @param head head of the current list
 * @return the head to the updated list
 * @warning allocates memory for a new node
 * @category ALGORITHM
 */
node_t* add_to_list_ordered(const unsigned char element, const size_t frequency,
                            node_t* head) {
  // allocates memory for new node and defines its variables to standard values
  node_t* new_node = malloc(sizeof(node_t));
  new_node->element = malloc(sizeof(unsigned char));
  *(unsigned char*)new_node->element = element;
  new_node->frequency = frequency;
  new_node->next = NULL;
  new_node->left = NULL;
  new_node->right = NULL;

  // insertion process
  if (!head) {
    head = new_node;
  } else if (head->frequency > new_node->frequency) {
    new_node->next = head;
    head = new_node;
  } else {
    node_t* aux = head;
    while (aux->next && aux->next->frequency < new_node->frequency) {
      aux = aux->next;
    }
    new_node->next = aux->next;
    aux->next = new_node;
  }

  // returns head to updated list
  return head;
}

/**
 * @brief - adds all needed elements to a ordered linked list structure
 * @param frequencies the array that holds all char frequencies
 * @return - the head of the linked list
 * @warning - allocates memory for every different node used
 * @category ALGORITHM
 */
node_t* create_list(const size_t* frequencies) {
  // node that will represent the head of the list
  node_t* head = NULL;

  // iterates by every char in the ascii table
  for (unsigned int index = 0; index < ASCII_SIZE; index++) {
    // only adds elements to the list that appear in the file
    if (frequencies[index] > 0) {
      head =
          add_to_list_ordered((unsigned char)index, frequencies[index], head);
    }
  }

  // returns the head at the proper location
  return head;
}

/**
 * @brief function for printing the linked list
 * @param head the head of the linked list
 * @category DEBUG
 */
void print_list(node_t* head) {
  printf("linked list:\n");
  while (head) {
    printf("\t%c: %zu\n", *(unsigned char*)head->element, head->frequency);
    head = head->next;
  }
}

node_t* create_tree(node_t* head) {
  if (!head) {
    perror("ERR: empty list in create_tree()\n");
    exit(EXIT_FAILURE);
  }
}

int main() {
  // declaring variables for the program
  char* file_name;
  unsigned char* content;
  size_t file_size;
  size_t* frequencies;
  node_t* head;

  // getting the file, its content and its size
  file_name = get_file_name();
  content = get_file_content(file_name, &file_size);

  // setting up element frequencies
  frequencies = get_frequencies(content, file_size);

  // setting up the linked list
  head = create_list(frequencies);

  // freeing leftover memory
  free(content);
  free(file_name);
  free(frequencies);

  return 0;
}