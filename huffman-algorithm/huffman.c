#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME_SIZE 200
#define ASCII_SIZE 256

typedef struct node {
  void* element;
  size_t frequency;
  struct node* next;
  struct node* left;
  struct node* right;
} node_t;

void log_info(const char* message) { printf("%s\n", message); }
void log_error(const char* message) { fprintf(stderr, "Error: %s\n", message); }

char* get_file_name() {
  char* file_name = malloc(FILE_NAME_SIZE * sizeof(char));
  if (file_name == NULL) {
    log_error("Could not allocate memory for file name");
    exit(EXIT_FAILURE);
  }

  printf("Enter the file name: ");
  scanf(" %s", file_name);

  return file_name;
}

unsigned char* get_file_content(const char* file_name, size_t* file_size) {
  FILE* file = fopen(file_name, "rb");
  if (file == NULL) {
    log_error("Could not open file");
    exit(EXIT_FAILURE);
  }

  fseek(file, 0, SEEK_END);
  *file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  unsigned char* content = malloc((*file_size + 1) * sizeof(unsigned char));
  if (!content) {
    log_error("Could not allocate memory for file content");
    fclose(file);
    exit(EXIT_FAILURE);
  }

  size_t bytes_read = fread(content, sizeof(unsigned char), *file_size, file);
  if (bytes_read != *file_size) {
    log_error("Could not read entire file");
    free(content);
    fclose(file);
    exit(EXIT_FAILURE);
  }

  content[*file_size] = '\0';

  fclose(file);
  return content;
}

size_t* get_frequencies(const unsigned char* content, const size_t file_size) {
  size_t* frequencies = calloc(ASCII_SIZE, sizeof(size_t));
  if (frequencies == NULL) {
    log_error("Could not allocate memory for frequencies table");
    exit(EXIT_FAILURE);
  }

  for (size_t byte = 0; byte < file_size; byte++) {
    frequencies[content[byte]]++;
  }

  return frequencies;
}

node_t* add_to_list_ordered(node_t* head, node_t* new_node) {
  if (head == NULL) {
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

  return head;
}

node_t* create_list(const size_t* frequencies) {
  node_t* head = NULL;

  for (unsigned int index = 0; index < ASCII_SIZE; index++) {
    if (frequencies[index] > 0) {
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

      head = add_to_list_ordered(head, new_node);
    }
  }

  return head;
}

node_t* fuse_nodes(node_t* head) {
  if (!head || !head->next) {
    log_error("List too short to fuse nodes");
    return head;
  }

  node_t* first = head;
  node_t* second = head->next;
  node_t* rest = second->next;

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
  *(unsigned char*)new_node->element = '*';
  new_node->frequency = first->frequency + second->frequency;
  new_node->left = first;
  new_node->right = second;
  new_node->next = NULL;

  first->next = NULL;
  second->next = NULL;

  head = add_to_list_ordered(rest, new_node);
  return head;
}

node_t* create_tree(node_t* head) {
  if (head == NULL) {
    log_error("Empty list - cannot create tree");
    exit(EXIT_FAILURE);
  }

  while (head->next != NULL) {
    head = fuse_nodes(head);
  }

  return head;
}

void destroy_tree(node_t* root) {
  if (root == NULL) return;
  destroy_tree(root->left);
  destroy_tree(root->right);
  if (root->element) free(root->element);
  free(root);
}

int get_tree_height(node_t* root) {
  if (root == NULL) return -1;
  if (root->left == NULL && root->right == NULL) return 0;

  int left_height = get_tree_height(root->left);
  int right_height = get_tree_height(root->right);

  return (left_height > right_height ? left_height : right_height) + 1;
}

void generate_codes(node_t* root, char** codes, char* current_code, int depth) {
  if (root == NULL) return;

  if (root->left == NULL && root->right == NULL) {
    current_code[depth] = '\0';
    unsigned char element = *(unsigned char*)root->element;
    strcpy(codes[element], current_code);
    return;
  }

  if (root->left != NULL) {
    current_code[depth] = '0';
    generate_codes(root->left, codes, current_code, depth + 1);
  }

  if (root->right != NULL) {
    current_code[depth] = '1';
    generate_codes(root->right, codes, current_code, depth + 1);
  }
}

void change_file_extension(char* file_name, const char* new_extension) {
  char* dot_position = strrchr(file_name, '.');
  if (dot_position != NULL) {
    *dot_position = '\0';
  }
  strcat(file_name, new_extension);
}

void write_trash_and_size(FILE* file, unsigned int trash, unsigned int size) {
  unsigned short header = (trash << 13) | (size & 0x1FFF);
  fwrite(&header, sizeof(unsigned short), 1, file);
}

void write_tree(FILE* file, node_t* root) {
  if (root == NULL) return;

  if (root->left == NULL && root->right == NULL) {
    unsigned char element = *(unsigned char*)root->element;
    if (element == '*' || element == '\\') {
      unsigned char escape = '\\';
      fwrite(&escape, sizeof(unsigned char), 1, file);
    }
    fwrite(&element, sizeof(unsigned char), 1, file);
  } else {
    unsigned char marker = '*';
    fwrite(&marker, sizeof(unsigned char), 1, file);
    write_tree(file, root->left);
    write_tree(file, root->right);
  }
}

unsigned int calculate_trash_size(const unsigned char* content,
                                  size_t file_size, char** codes) {
  size_t total_bits = 0;

  for (size_t i = 0; i < file_size; i++) {
    unsigned char byte = content[i];
    total_bits += strlen(codes[byte]);
  }

  unsigned int trash_size = (8 - (total_bits % 8)) % 8;
  return trash_size;
}

unsigned int calculate_tree_size(node_t* root) {
  if (root == NULL) return 0;

  if (root->left == NULL && root->right == NULL) {
    unsigned char element = *(unsigned char*)root->element;
    if (element == '*' || element == '\\') {
      return 2;
    }
    return 1;
  }

  return 1 + calculate_tree_size(root->left) + calculate_tree_size(root->right);
}

void write_compressed_file(const char* file_name, const unsigned char* content,
                           size_t file_size, char** codes, node_t* root) {
  FILE* file = fopen(file_name, "wb");
  if (file == NULL) {
    log_error("Could not open file for writing");
    exit(EXIT_FAILURE);
  }

  unsigned int trash_size = calculate_trash_size(content, file_size, codes);
  unsigned int tree_size = calculate_tree_size(root);

  write_trash_and_size(file, trash_size, tree_size);
  write_tree(file, root);

  unsigned char buffer = 0;
  int bit_count = 0;
  for (size_t i = 0; i < file_size; i++) {
    unsigned char byte = content[i];
    char* code = codes[byte];

    for (size_t j = 0; j < strlen(code); j++) {
      buffer <<= 1;
      if (code[j] == '1') {
        buffer |= 1;
      }
      bit_count++;

      if (bit_count == 8) {
        fwrite(&buffer, sizeof(unsigned char), 1, file);
        buffer = 0;
        bit_count = 0;
      }
    }
  }

  if (bit_count > 0) {
    buffer <<= (8 - bit_count);
    fwrite(&buffer, sizeof(unsigned char), 1, file);
  }

  fclose(file);
}

char* ask_file_extension() {
  char* file_extension = malloc(10 * sizeof(char));
  if (file_extension == NULL) {
    log_error("Could not allocate memory for file extension");
    exit(EXIT_FAILURE);
  }

  scanf(" %s", file_extension);

  return file_extension;
}

void display_menu() {
  printf("Select operation mode:\n");
  printf("1. Compress file\n");
  printf("2. Extract file\n");
  printf("Enter your choice: ");
}

void read_header(FILE* file, unsigned int* trash_size,
                 unsigned int* tree_size) {
  unsigned short header;
  if (fread(&header, sizeof(unsigned short), 1, file) != 1) {
    log_error("Could not read header from file");
    exit(EXIT_FAILURE);
  }
  *trash_size = (header >> 13) & 0x07;
  *tree_size = header & 0x1FFF;
}

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

void decompress_data(FILE* input_file, FILE* output_file, node_t* root,
                     unsigned int trash_size, long data_start_pos) {
  node_t* current = root;
  unsigned char byte;

  fseek(input_file, 0, SEEK_END);
  long file_size = ftell(input_file);
  fseek(input_file, data_start_pos, SEEK_SET);

  long data_size = file_size - data_start_pos;
  size_t bytes_read = 0;

  while (fread(&byte, sizeof(unsigned char), 1, input_file)) {
    bytes_read++;
    int bits_to_read = (bytes_read == data_size) ? 8 - trash_size : 8;

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

void extract_file(char* file_name) {
  FILE* input_file = fopen(file_name, "rb");
  if (!input_file) {
    log_error("Could not open input file");
    free(file_name);
    return;
  }

  unsigned int trash_size, tree_size;
  read_header(input_file, &trash_size, &tree_size);

  node_t* root = reconstruct_tree(input_file);
  if (!root) {
    log_error("Could not reconstruct Huffman tree");
    fclose(input_file);
    free(file_name);
    return;
  }

  long data_start_pos = ftell(input_file);

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

  decompress_data(input_file, output_file, root, trash_size, data_start_pos);

  destroy_tree(root);
  free(output_file_name);
  free(extension);
  fclose(input_file);
  fclose(output_file);

  log_info("File extracted successfully");
}

int main() {
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

  display_menu();
  if (scanf("%d", &mode) != 1) {
    log_error("Invalid input");
    return EXIT_FAILURE;
  }

  file_name = get_file_name();

  if (mode == 1) {
    log_info("Starting compression process...");

    content = get_file_content(file_name, &file_size);
    frequencies = get_frequencies(content, file_size);
    head = create_list(frequencies);
    root = create_tree(head);
    tree_height = get_tree_height(root);

    codes = malloc(ASCII_SIZE * sizeof(char*));
    if (!codes) {
      log_error("Could not allocate memory for codes");
      exit(EXIT_FAILURE);
    }

    for (int i = 0; i < ASCII_SIZE; i++) {
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

    compressed_name = malloc(strlen(file_name) + 10);
    if (!compressed_name) {
      log_error("Could not allocate memory for compressed file name");
      exit(EXIT_FAILURE);
    }
    strcpy(compressed_name, file_name);
    change_file_extension(compressed_name, ".huff");

    write_compressed_file(compressed_name, content, file_size, codes, root);

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

    extract_file(file_name);

    log_info("Decompression completed successfully");
  } else {
    log_error("Invalid mode selected");
    free(file_name);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}