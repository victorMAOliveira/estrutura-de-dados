#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAM_LINHA 256
#define TAM_LEITURA 5
#define TAM_INICIAL 0

typedef struct {
  int** clausulas;
  int* tamanhos;
  int qtdClausulas;
  int qtdVariaveis;
} sat_cnf_t;

typedef struct sat_no {
  int indice_variavel;
  struct sat_no* esquerda;
  struct sat_no* direita;
} sat_no_t;

void lerCNF(const char* nomeArquivo, sat_cnf_t* cnf) {
  FILE* arquivo = fopen(nomeArquivo, "r");
  if (arquivo == NULL) {
    perror("Erro ao abrir o arquivo");
    return;
  }

  char linha[TAM_LINHA];
  int clausulaAtual = 0;

  while (fgets(linha, sizeof(linha), arquivo)) {
    if (linha[0] == 'c') continue;

    if (strncmp(linha, "p cnf", TAM_LEITURA) == 0) {
      sscanf(linha, "p cnf %d %d", &cnf->qtdVariaveis, &cnf->qtdClausulas);
      cnf->clausulas = malloc(cnf->qtdClausulas * sizeof(int*));
      cnf->tamanhos = malloc(cnf->qtdClausulas * sizeof(int));
      continue;
    }

    int literal, tamanho = 0;
    int* clausula = malloc(TAM_LINHA * sizeof(int));
    char* token = strtok(linha, " ");

    while (token != NULL) {
      literal = atoi(token);
      if (literal == 0) break;
      clausula[tamanho++] = literal;
      token = strtok(NULL, " ");
    }

    cnf->clausulas[clausulaAtual] = malloc(tamanho * sizeof(int));

    memcpy(cnf->clausulas[clausulaAtual], clausula, tamanho * sizeof(int));

    cnf->tamanhos[clausulaAtual] = tamanho;

    clausulaAtual++;

    free(clausula);
  }
  fclose(arquivo);
}

bool satisfazClausula(int* clausula, int tamanho, bool* valores) {
  for (int i = 0; i < tamanho; i++) {
    int literal = clausula[i];
    int var = abs(literal) - 1;
    bool valor = valores[var];

    if ((literal > 0 && valor) || (literal < 0 && !valor)) return true;
  }
  return false;
}

bool verificaCNF(sat_cnf_t* cnf, bool* valores) {
  for (int i = 0; i < cnf->qtdClausulas; i++) {
    if (!satisfazClausula(cnf->clausulas[i], cnf->tamanhos[i], valores))
      return false;
  }
  return true;
}

sat_no_t* construirArvore(int nivel, int qtdVariaveis) {
  if (nivel == qtdVariaveis) return NULL;

  sat_no_t* no = malloc(sizeof(sat_no_t));

  no->indice_variavel = nivel;
  no->esquerda = construirArvore(nivel + 1, qtdVariaveis);
  no->direita = construirArvore(nivel + 1, qtdVariaveis);

  return no;
}

bool avaliarArvore(sat_no_t* raiz, sat_cnf_t* cnf, bool* valores) {
  if (raiz == NULL) return verificaCNF(cnf, valores);

  valores[raiz->indice_variavel] = true;

  if (avaliarArvore(raiz->esquerda, cnf, valores)) return true;

  valores[raiz->indice_variavel] = false;

  if (avaliarArvore(raiz->direita, cnf, valores)) return true;

  return false;
}

void liberarArvore(sat_no_t* raiz) {
  if (raiz == NULL) return;

  liberarArvore(raiz->esquerda);
  liberarArvore(raiz->direita);

  free(raiz);
}

void liberarCNF(sat_cnf_t* cnf) {
  for (int i = 0; i < cnf->qtdClausulas; i++) {
    free(cnf->clausulas[i]);
  }

  free(cnf->clausulas);
  free(cnf->tamanhos);
}

int main() {
  sat_cnf_t cnf;
  lerCNF("exemplo.cnf", &cnf);

  bool* valores = calloc(cnf.qtdVariaveis, sizeof(bool));
  sat_no_t* raiz = construirArvore(0, cnf.qtdVariaveis);

  if (avaliarArvore(raiz, &cnf, valores)) {
    printf("\nSAT\n");
  } else {
    printf("\nUNSAT\n");
  }

  liberarArvore(raiz);
  free(valores);
  liberarCNF(&cnf);
  return 0;
}