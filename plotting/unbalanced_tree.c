#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct Node {
  int val;
  struct Node *left, *right;
} Node;

Node* new_node(int v) {
  Node* n = malloc(sizeof(Node));
  if (!n) {
    perror("malloc");
    exit(1);
  }
  n->val = v;
  n->left = n->right = NULL;
  return n;
}

Node* bst_insert(Node* root, int v) {
  if (!root) return new_node(v);
  if (v < root->val)
    root->left = bst_insert(root->left, v);
  else
    root->right = bst_insert(root->right, v);
  return root;
}

int bst_search_count(Node* root, int target) {
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

int cmp_int(const void* a, const void* b) {
  int ia = *(const int*)a;
  int ib = *(const int*)b;
  return (ia > ib) - (ia < ib);
}

int main(void) {
  const char* infile = "random_nums.txt";
  const char* outfile = "unbalanced_tries_sorted.txt";
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
      nums = realloc(nums, cap * sizeof(int));
      if (!nums) {
        perror("realloc");
        fclose(f);
        return 1;
      }
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
  for (size_t i = 0; i < n; ++i) root = bst_insert(root, nums[i]);

  srand((unsigned)time(NULL));
  int* tries = malloc(n * sizeof(int));
  if (!tries) {
    perror("malloc");
    free(nums);
    return 1;
  }

  for (size_t i = 0; i < n; ++i) {
    size_t r = (size_t)(rand() % n);
    int target = nums[r];
    tries[i] = bst_search_count(root, target);
  }

  qsort(tries, n, sizeof(int), cmp_int);

  FILE* out = fopen(outfile, "w");
  if (!out) {
    perror(outfile);
    free(nums);
    free(tries);
    return 1;
  }
  for (size_t i = 0; i < n; ++i) {
    fprintf(out, "%d\n", tries[i]);
  }
  fclose(out);

  free(nums);
  free(tries);
  return 0;
}