#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct Node {
  int val;
  struct Node *left, *right;
  int height;
} Node;

int node_height(Node* n) { return n ? n->height : 0; }

static void update_height(Node* n) {
  int hl = node_height(n->left);
  int hr = node_height(n->right);
  n->height = (hl > hr ? hl : hr) + 1;
}

int balance_factor(Node* n) {
  return n ? node_height(n->left) - node_height(n->right) : 0;
}

Node* new_node(int v) {
  Node* n = malloc(sizeof(Node));
  if (!n) {
    perror("malloc");
    exit(1);
  }
  n->val = v;
  n->left = n->right = NULL;
  n->height = 1;
  return n;
}

Node* right_rotate(Node* y) {
  Node* x = y->left;
  Node* T2 = x->right;
  x->right = y;
  y->left = T2;
  update_height(y);
  update_height(x);
  return x;
}

Node* left_rotate(Node* x) {
  Node* y = x->right;
  Node* T2 = y->left;
  y->left = x;
  x->right = T2;
  update_height(x);
  update_height(y);
  return y;
}

/* AVL insert (duplicates go to the right) */
Node* avl_insert(Node* node, int v) {
  if (!node) return new_node(v);
  if (v < node->val)
    node->left = avl_insert(node->left, v);
  else
    node->right = avl_insert(node->right, v);

  update_height(node);
  int bf = balance_factor(node);

  if (bf > 1 && v < node->left->val) return right_rotate(node);
  if (bf > 1 && v > node->left->val) {
    node->left = left_rotate(node->left);
    return right_rotate(node);
  }
  if (bf < -1 && v > node->right->val) return left_rotate(node);
  if (bf < -1 && v < node->right->val) {
    node->right = right_rotate(node->right);
    return left_rotate(node);
  }
  return node;
}

/* Count node comparisons to find target (iterative) */
int avl_search_count(Node* root, int target) {
  int count = 0;
  Node* cur = root;
  while (cur) {
    count++;
    if (target == cur->val) return count;
    if (target < cur->val)
      cur = cur->left;
    else
      cur = cur->right;
  }
  return count;
}

void free_tree(Node* n) {
  if (!n) return;
  free_tree(n->left);
  free_tree(n->right);
  free(n);
}

int cmp_int(const void* a, const void* b) {
  int ia = *(const int*)a;
  int ib = *(const int*)b;
  return (ia > ib) - (ia < ib);
}

int main(void) {
  const char* infile = "random_nums.txt";
  const char* outfile = "avl_tries_sorted.txt";
  FILE* f = fopen(infile, "r");
  if (!f) {
    perror(infile);
    return 1;
  }

  size_t cap = 4096, n = 0;
  int* nums = malloc(cap * sizeof(int));
  if (!nums) {
    perror("malloc");
    fclose(f);
    return 1;
  }

  while (1) {
    int x;
    if (fscanf(f, "%d", &x) != 1) break;
    if (n >= cap) {
      cap *= 2;
      int* tmp = realloc(nums, cap * sizeof(int));
      if (!tmp) {
        perror("realloc");
        free(nums);
        fclose(f);
        return 1;
      }
      nums = tmp;
    }
    nums[n++] = x;
  }
  fclose(f);
  if (n == 0) {
    fprintf(stderr, "no numbers found in %s\n", infile);
    free(nums);
    return 1;
  }

  Node* root = NULL;
  for (size_t i = 0; i < n; ++i) root = avl_insert(root, nums[i]);

  srand((unsigned)time(NULL));
  int* tries = malloc(n * sizeof(int));
  if (!tries) {
    perror("malloc");
    free(nums);
    free_tree(root);
    return 1;
  }

  for (size_t i = 0; i < n; ++i) {
    size_t r = (size_t)(rand() % n);
    int target = nums[r];
    tries[i] = avl_search_count(root, target);
  }

  qsort(tries, n, sizeof(int), cmp_int);

  FILE* out = fopen(outfile, "w");
  if (!out) {
    perror(outfile);
    free(nums);
    free(tries);
    free_tree(root);
    return 1;
  }
  for (size_t i = 0; i < n; ++i) fprintf(out, "%d\n", tries[i]);
  fclose(out);

  free(nums);
  free(tries);
  free_tree(root);
  return 0;
}