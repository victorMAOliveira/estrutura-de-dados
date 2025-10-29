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

void log_info(const char* message) { printf("%s\n", message); }
void log_error(const char* message) { fprintf(stderr, "Error: %s\n", message); }

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
    log_error("Could not allocate memory for file name");
    exit(EXIT_FAILURE);
  }

  // Get user input for the name
  printf("Enter the file name: ");
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
    log_error("Could not open file");
    exit(EXIT_FAILURE);
  }

  // Get file size
  fseek(file, 0, SEEK_END);
  *file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  // Allocate memory for the contents
  unsigned char* content = malloc((*file_size + 1) * sizeof(unsigned char));
  if (!content) {
    log_error("Could not allocate memory for file content");
    fclose(file);
    exit(EXIT_FAILURE);
  }

  // Read the file content into the buffer
  size_t bytes_read = fread(content, sizeof(unsigned char), *file_size, file);
  if (bytes_read != *file_size) {
    log_error("Could not read entire file");
    free(content);
    fclose(file);
    exit(EXIT_FAILURE);
  }

  content[*file_size] = '\0';

  // Close file and return the content
  fclose(file);
  return content;
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
    log_error("Could not allocate memory for frequencies table");
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
      if (!new_node) {
        log_error("Could not allocate memory for node");
        exit(EXIT_FAILURE);
      }

      new_node->element = malloc(sizeof(unsigned char));
      if (!new_node->element) {
        log_error("Could not allocate memory for element");
        free(new_node);
        exit(EXIT_FAILURE);
      }

      *(unsigned char*)new_node->element = (unsigned char)index;
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
 * @brief - Fuses the first two nodes of the list into one for the tree
 * creation step
 * @param head Head of the current list
 * @return Head of the updated list
 * @warning - Allocates memory for the new node created
 * @category ALGORITHM
 */
node_t* fuse_nodes(node_t* head) {
  // Head must have at least two nodes
  if (!head || !head->next) {
    log_error("List too short to fuse nodes");
    return head;
  }

  node_t* first = head;
  node_t* second = head->next;
  node_t* rest = second->next;  // Remainder of the list after the two smallest

  // Create new internal node
  node_t* new_node = malloc(sizeof(node_t));
  if (new_node == NULL) {
    log_error("Could not allocate memory for node fusion");
    exit(EXIT_FAILURE);
  }
  new_node->element = malloc(sizeof(unsigned char));
  if (new_node->element == NULL) {
    log_error("Could not allocate memory for element in node fusion");
    free(new_node);
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
    log_error("Empty list - cannot create tree");
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
 * @brief - Gets the height of the tree (recursively)
 * @param root Root of the tree
 * @return Height of the tree
 * @category DEBUG
 */
int get_tree_height(node_t* root) {
  if (root == NULL) return -1;
  if (root->left == NULL && root->right == NULL) return 0;

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
    unsigned char element = *(unsigned char*)root->element;
    strcpy(codes[element], current_code);
    return;
  }

  // Traverse left
  if (root->left != NULL) {
    current_code[depth] = '0';
    generate_codes(root->left, codes, current_code, depth + 1);
  }

  // Traverse right
  if (root->right != NULL) {
    current_code[depth] = '1';
    generate_codes(root->right, codes, current_code, depth + 1);
  }
}

/**
 * @brief - Changes the file extension of a given file name
 * @param file_name The original file name
 * @param new_extension The new extension to be added (including the dot)
 * @category UTILITIES
 */
void change_file_extension(char* file_name, const char* new_extension) {
  char* dot_position = strrchr(file_name, '.');
  if (dot_position != NULL) {
    *dot_position = '\0';  // Terminate the string at the dot
  }
  strcat(file_name, new_extension);  // Append the new extension
}

/**
 * @brief - Writes the trash size and tree size to the compressed file
 * @param file The file to write to
 * @param trash The number of trash bits
 * @param size The size of the tree in bytes
 * @category ALGORITHM
 */
void write_trash_and_size(FILE* file, unsigned int trash, unsigned int size) {
  // Combine trash and size into two bytes
  if (trash > 7) {
    trash = 7;
  }
  if (size > 0x1FFF) {
    size = 0x1FFF;
  }

  unsigned short header = (trash << 13) | (size & 0x1FFF);

  // Write the two bytes to the file
  fwrite(&header, sizeof(unsigned short), 1, file);
}

/**
 * @brief - Writes the tree structure to the compressed file (recursively)
 * @param file The file to write to
 * @param root Root of the tree
 * @category ALGORITHM
 */
void write_tree(FILE* file, node_t* root) {
  if (root == NULL) return;

  // If it's a leaf node, write the character (with escape if needed)
  if (root->left == NULL && root->right == NULL) {
    unsigned char element = *(unsigned char*)root->element;
    if (element == '*' || element == '\\') {
      // Write escape character
      unsigned char escape = '\\';
      fwrite(&escape, sizeof(unsigned char), 1, file);
    }
    // Write the character
    fwrite(&element, sizeof(unsigned char), 1, file);
  } else {
    // Write internal node marker
    unsigned char marker = '*';
    fwrite(&marker, sizeof(unsigned char), 1, file);
    // Recur for left and right children
    write_tree(file, root->left);
    write_tree(file, root->right);
  }
}

/**
 * @brief - Calculates the trash size based on file content and codes
 * @param content Content of the file
 * @param file_size Size of the file
 * @param codes Array holding the generated codes
 * @return The trash size in bits
 * @category ALGORITHM
 */
unsigned int calculate_trash_size(const unsigned char* content,
                                  size_t file_size, char** codes) {
  size_t total_bits = 0;

  // Calculate total bits needed for the compressed data
  for (size_t i = 0; i < file_size; i++) {
    unsigned char byte = content[i];
    total_bits += strlen(codes[byte]);
  }

  // Calculate trash size (bits not fitting into full bytes)
  unsigned int trash_size = (8 - (total_bits % 8)) % 8;
  return trash_size;
}

/**
 * @brief - Calculates the size of the tree in bytes (recursively)
 * @param root Root of the tree
 * @return Size of the tree in bytes
 * @category ALGORITHM
 */
unsigned int calculate_tree_size(node_t* root) {
  if (root == NULL) return 0;

  // If it's a leaf node
  if (root->left == NULL && root->right == NULL) {
    // Check if we need to account for escape character
    unsigned char element = *(unsigned char*)root->element;
    if (element == '*' || element == '\\') {
      return 2;  // Escape character + actual character
    }
    return 1;  // Just the character
  }

  // Internal node
  return 1 + calculate_tree_size(root->left) + calculate_tree_size(root->right);
}

/**
 * @brief - Writes the compressed file with header, tree, and data
 * @param file_name Name of the file to write to
 * @param content Content of the original file
 * @param file_size Size of the original file
 * @param codes Array holding the generated codes
 * @param root Root of the Huffman tree
 * @category ALGORITHM
 */
void write_compressed_file(const char* file_name, const unsigned char* content,
                           size_t file_size, char** codes, node_t* root) {
  // Open file for writing
  FILE* file = fopen(file_name, "wb");
  if (file == NULL) {
    log_error("Could not open file for writing");
    exit(EXIT_FAILURE);
  }

  // Calculate trash size and tree size
  unsigned int trash_size = calculate_trash_size(content, file_size, codes);
  unsigned int tree_size = calculate_tree_size(root);

  // Write trash size and tree size to file
  write_trash_and_size(file, trash_size, tree_size);

  // Write tree structure to file
  write_tree(file, root);

  // Write compressed data to file
  unsigned char buffer = 0;
  int bit_count = 0;
  for (size_t i = 0; i < file_size; i++) {
    unsigned char byte = content[i];
    char* code = codes[byte];

    for (size_t j = 0; j < strlen(code); j++) {
      // Add bit to buffer
      buffer <<= 1;
      if (code[j] == '1') {
        buffer |= 1;
      }
      bit_count++;

      // If buffer is full, write it to file
      if (bit_count == 8) {
        fwrite(&buffer, sizeof(unsigned char), 1, file);
        buffer = 0;
        bit_count = 0;
      }
    }
  }

  // Write remaining bits in buffer (if any)
  if (bit_count > 0) {
    buffer <<= (8 - bit_count);  // Shift remaining bits to the left
    fwrite(&buffer, sizeof(unsigned char), 1, file);
  }

  // Close the file
  fclose(file);
}

/**
 * @brief - Asks the user for the desired file extension
 * @return - The file extension provided by the user
 * @warning - Exits program if not able to allocate memory for the string
 * @warning - Allocates memory for the string that needs to be freed after use
 * @category UTILITIES
 */
char* ask_file_extension() {
  // Allocate memory for string
  char* file_extension = malloc(10 * sizeof(char));
  if (file_extension == NULL) {
    log_error("Could not allocate memory for file extension");
    exit(EXIT_FAILURE);
  }

  // Get user input for the extension
  scanf(" %s", file_extension);

  // Return the file extension
  return file_extension;
}

/**
 * @brief - Displays the main menu to the user
 * @category UTILITIES
 */
void display_menu() {
  printf("Select operation mode:\n");
  printf("1. Compress file\n");
  printf("2. Extract file\n");
  printf("Enter your choice: ");
}

/**
 * @brief - Reads the header (trash and tree size) from compressed file
 * @param file The file to read from
 * @param trash_size Pointer to store trash size
 * @param tree_size Pointer to store tree size
 * @category ALGORITHM
 */
void read_header(FILE* file, unsigned int* trash_size,
                 unsigned int* tree_size) {
  unsigned short header;
  if (fread(&header, sizeof(unsigned short), 1, file) != 1) {
    log_error("Could not read header from file");
    exit(EXIT_FAILURE);
  }
  *trash_size = (header >> 13) & 0x07;  // Get first 3 bits
  *tree_size = header & 0x1FFF;         // Get remaining 13 bits
}

/**
 * @brief - Reconstructs the Huffman tree from the compressed file
 * @param file The file to read from
 * @return Root of the reconstructed tree
 * @category ALGORITHM
 */
node_t* reconstruct_tree(FILE* file) {
  unsigned char byte;
  if (fread(&byte, sizeof(unsigned char), 1, file) != 1) {
    return NULL;
  }

  node_t* node = malloc(sizeof(node_t));
  if (!node) {
    log_error("Could not allocate memory for tree reconstruction");
    return NULL;
  }

  if (byte == '*') {
    node->element = malloc(sizeof(unsigned char));
    if (!node->element) {
      log_error("Could not allocate memory for element in tree reconstruction");
      free(node);
      return NULL;
    }
    *(unsigned char*)node->element = byte;
    node->left = reconstruct_tree(file);
    node->right = reconstruct_tree(file);
  } else if (byte == '\\') {
    if (fread(&byte, sizeof(unsigned char), 1, file) != 1) {
      free(node);
      return NULL;
    }
    node->element = malloc(sizeof(unsigned char));
    if (!node->element) {
      log_error("Could not allocate memory for element in tree reconstruction");
      free(node);
      return NULL;
    }
    *(unsigned char*)node->element = byte;
    node->left = NULL;
    node->right = NULL;
  } else {
    node->element = malloc(sizeof(unsigned char));
    if (!node->element) {
      log_error("Could not allocate memory for element in tree reconstruction");
      free(node);
      return NULL;
    }
    *(unsigned char*)node->element = byte;
    node->left = NULL;
    node->right = NULL;
  }

  return node;
}

/**
 * @brief - Decompresses the data using the reconstructed Huffman tree
 * @param input_file Input compressed file
 * @param output_file Output decompressed file
 * @param root Root of the Huffman tree
 * @param trash_size Number of trash bits
 * @param file_size Size of the compressed file
 * @category ALGORITHM
 */
void decompress_data(FILE* input_file, FILE* output_file, node_t* root,
                     unsigned int trash_size, long data_start_pos) {
  node_t* current = root;
  unsigned char byte;

  // Get current position and calculate data size
  fseek(input_file, 0, SEEK_END);
  long file_size = ftell(input_file);
  fseek(input_file, data_start_pos, SEEK_SET);

  long data_size = file_size - data_start_pos;
  size_t bytes_read = 0;

  while (fread(&byte, sizeof(unsigned char), 1, input_file)) {
    bytes_read++;
    int bits_to_read = (bytes_read == data_size) ? 8 - trash_size : 8;

    // Read bits from MSB to LSB
    for (int i = 7; i >= 8 - bits_to_read; i--) {
      int bit = (byte >> i) & 1;

      if (bit) {
        current = current->right;
      } else {
        current = current->left;
      }

      if (current == NULL) {
        log_error("Invalid Huffman tree path during decompression");
        return;
      }

      if (current->left == NULL && current->right == NULL) {
        fwrite(current->element, sizeof(unsigned char), 1, output_file);
        current = root;
      }
    }
  }
}

/**
 * @brief - Main function for extracting compressed files
 * @category ALGORITHM
 */
void extract_file(char* file_name) {
  FILE* input_file = fopen(file_name, "rb");
  if (!input_file) {
    log_error("Could not open input file");
    free(file_name);
    return;
  }

  // Read header information
  unsigned int trash_size, tree_size;
  read_header(input_file, &trash_size, &tree_size);

  // Reconstruct Huffman tree
  node_t* root = reconstruct_tree(input_file);
  if (!root) {
    log_error("Could not reconstruct Huffman tree");
    fclose(input_file);
    free(file_name);
    return;
  }

  // Get current position after reading tree
  long data_start_pos = ftell(input_file);

  // Create output file
  char* output_file_name = malloc(strlen(file_name) + 10);
  if (!output_file_name) {
    log_error("Could not allocate memory for output file name");
    destroy_tree(root);
    free(file_name);
    fclose(input_file);
    return;
  }
  strcpy(output_file_name, file_name);

  printf("Enter the target file extension (including the dot, e.g., .txt): ");
  char* extension = ask_file_extension();
  change_file_extension(output_file_name, extension);

  FILE* output_file = fopen(output_file_name, "wb");
  if (!output_file) {
    log_error("Could not create output file");
    destroy_tree(root);
    free(output_file_name);
    free(extension);
    fclose(input_file);
    free(file_name);
    return;
  }

  // Decompress the data
  decompress_data(input_file, output_file, root, trash_size, data_start_pos);

  // Cleanup
  destroy_tree(root);
  free(output_file_name);
  free(extension);
  fclose(input_file);
  fclose(output_file);

  log_info("File extracted successfully");
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
  char* current_code;
  char* compressed_name;

  // Deciding if the user wants to either compress or extract the given file
  display_menu();
  if (scanf("%d", &mode) != 1) {
    log_error("Invalid input");
    return EXIT_FAILURE;
  }

  file_name = get_file_name();

  // Compression mode
  if (mode == 1) {
    log_info("Starting compression process...");

    // Get file content and size
    content = get_file_content(file_name, &file_size);

    // Setting up element frequencies
    frequencies = get_frequencies(content, file_size);

    // Setting up the linked list
    head = create_list(frequencies);

    // Setting up the huffman tree
    root = create_tree(head);

    // Get tree height for code allocation
    tree_height = get_tree_height(root);

    // Code generation
    codes = malloc(ASCII_SIZE * sizeof(char*));
    if (!codes) {
      log_error("Could not allocate memory for codes");
      exit(EXIT_FAILURE);
    }

    for (int i = 0; i < ASCII_SIZE; i++) {
      // Allocate memory for each code string
      // Each code can be at most tree_height bits long + 1 for null terminator
      codes[i] = calloc(tree_height + 2, sizeof(char));
      if (!codes[i]) {
        log_error("Could not allocate memory for code string");
        for (int j = 0; j < i; j++) free(codes[j]);
        free(codes);
        exit(EXIT_FAILURE);
      }
    }

    current_code = calloc(tree_height + 2, sizeof(char));
    if (!current_code) {
      log_error("Could not allocate memory for code generation");
      for (int i = 0; i < ASCII_SIZE; i++) free(codes[i]);
      free(codes);
      exit(EXIT_FAILURE);
    }

    generate_codes(root, codes, current_code, 0);

    // File operations
    compressed_name = malloc(strlen(file_name) + 10);
    if (!compressed_name) {
      log_error("Could not allocate memory for compressed file name");
      exit(EXIT_FAILURE);
    }
    strcpy(compressed_name, file_name);
    change_file_extension(compressed_name, ".huff");

    write_compressed_file(compressed_name, content, file_size, codes, root);

    // Cleanup
    destroy_tree(root);
    for (int i = 0; i < ASCII_SIZE; i++) {
      free(codes[i]);
    }
    free(codes);
    free(current_code);
    free(content);
    free(compressed_name);
    free(file_name);
    free(frequencies);
    log_info("Compression completed successfully");

  } else if (mode == 2) {
    log_info("Starting decompression process...");

    // Decompression process
    extract_file(file_name);

    log_info("Decompression completed successfully");
  } else {
    log_error("Invalid mode selected");
    free(file_name);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}