/**
 *   @file SATsolver.h
 *   @brief Implementacao do algoritimo de logica booleana SAT Solver
 *   @author Lucitânia Lopes, Lorenzo Holanda, Victor Amaral
 *   @date 2025-10-10
 *   @version 1.0
 */

#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAM_LINHA 200
#define TAM_LEITURA 5
#define TAM_INICIAL 0

/**
 *    @brief Define o que e necessario para compor a CNF (Conjuctive normal
 * form)
 *    @param clausulas  Clausula e o cujunto de condicoes ligadas pelo operador
 * logico "OU". Armazena um vetor de enderecos
 *    @param variaveis Armazena o tamanho das clausulas em um vetor
 *    @param qtdClausulas Armazena a quantidade de clausulas que serao testadas
 *    @param qtdVariaveis a quantidade de variaveis booleanas (literais) a serem
 * testadas
 *    @note Compoe as clausulas, seus respectivos variaveis, a quantidade delas
 * e quantos literais haverao
 *    @note CNF e a forma padrao de escrever funcoes logicas
 */
typedef struct {
  int** clausulas;
  int* tamanho;
  int qtdVariaveis;
  int qtdClausulas;
} sat_cnf_t;

/**
 *  @brief Representa Um no da arvore binaria.
 *  @param  indice_variavel Indicie da variavel (comeca em 0)
 *  @param  esquerda Ponteiro que testa arvore como verdade (true)
 *  @param  direita Ponteiro que testa a arvore como falso (false)
 */
typedef struct sat_no {
  int indice_variavel;
  struct sat_no* esquerda;
  struct sat_no* direita;
} sat_no_t;

/**
 *  @brief Funcao para ler o arquivo CNF.
 *   Associa os dados do texto a struct "CNF"
 *  @note Aloca os valores da clausula e os variaveis do array na struct CNF
 */
void lerCNF(const char* arquivoCNF, sat_cnf_t* cnf) {
  FILE* arquivo = fopen(arquivoCNF, "r");
  if (arquivo == NULL) {
    fprintf(stderr, "ERRO: Não foi possível abrir o arquivo %s\n", arquivoCNF);
    return;
  }

  char linha[TAM_LINHA];
  int clausulaAtual = 0;

  while (fgets(linha, sizeof(linha), arquivo)) {
    if (linha[0] == 'c') continue;

    if (strncmp(linha, "p cnf", TAM_LEITURA) == 0) {
      sscanf(linha, "p cnf %d %d", &cnf->qtdVariaveis, &cnf->qtdClausulas);
      cnf->clausulas = malloc(cnf->qtdClausulas * sizeof(int*));
      cnf->tamanho = malloc(cnf->qtdClausulas * sizeof(int));

      continue;
    }
  }
  printf("CNF detectado com %d variaveis e %d clausulas\n", cnf->qtdVariaveis,
         cnf->qtdClausulas);

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
  printf("Clausula %d lida: ", clausulaAtual + 1);

  cnf->tamanho[clausulaAtual] = tamanho;
  clausulaAtual++;

  free(clausula);

  printf("\n");
  fclose(arquivo);
}

/**
 * @brief verifica se a clausula e satisfeita, logando cada literal avaliado
 * @param clausula ponteiro para os literais da clausula
 * @param tamanho quantidade de literais na clausula
 * @param valores atribuicoes atuais (x1 em valores[0], x2 em valores[1], ...)
 */

bool satisfazClausula(int* clausula, int tamanho, bool* valores) {
  for (int i = 0; i < tamanho; i++) {
    int lit = clausula[i];
    int var = abs(lit) - 1;
    bool valor_var = valores[var];
    bool valor_lit = (lit > 0) ? valor_var : !valor_var;

    // Ex.: "literal -3 => true (x3 = false)"
    printf("literal %d => %s (x%d = %s)\n", lit, valor_lit ? "true" : "false",
           abs(lit), valor_var ? "true" : "false");

    if (valor_lit == 1) {
      printf("-> clausula satisfeita por %d\n\n", lit);
      return true;
    }
  }
  printf("-> clausula NAO satisfeita\n");
  return false;
}

bool verificaCNF(sat_cnf_t* cnf, bool* valores) {
  printf("Verificando CNF (%d clausulas, %d variaveis)\n", cnf->qtdClausulas,
         cnf->qtdVariaveis);
  printf("\n");

  for (int i = 0; i < cnf->qtdClausulas; i++) {
    if (!satisfazClausula(cnf->clausulas[i], cnf->tamanho[i], valores)) {
      printf(
          "Resultado parcial: CNF NAO satisfeita (falhou na clausula %d)\n\n",
          i + 1);
      return false;
    }
  }

  printf("Resultado: CNF satisfeita\n");
  return true;
}

/**
 * @brief Constroi a arvore binaria (com log de direcao)
 * @param direcao rótulo textual para saber se o nó veio da esquerda/TRUE ou
 * direita/FALSE
 */
sat_no_t* construirArvore(int nivel, int qtdVariaveis) {
  if (nivel == qtdVariaveis) {
    return NULL;
  }

  sat_no_t* no = (sat_no_t*)malloc(sizeof(sat_no_t));
  if (no == NULL) {
    printf("Erro ao alocar memória\n");
    return NULL;
  }

  no->indice_variavel = nivel;
  no->esquerda = construirArvore(nivel + 1, qtdVariaveis);
  no->direita = construirArvore(nivel + 1, qtdVariaveis);

  return no;
}

/**
 * @brief Percorre a arvore e testa todas as atribuicoes com logs de caminho
 */
bool avaliarArvore(sat_no_t* raiz, sat_cnf_t* cnf, bool* valores) {
  if (raiz == NULL) {
    printf("Opa! cheguei na folha bora verificar B>\n\n");
    return verificaCNF(cnf, valores);
  }

  printf("Atribuindo x%d (nivel %d)\n", raiz->indice_variavel + 1,
         raiz->indice_variavel);
  valores[raiz->indice_variavel] = true;
  printf("x%d = true (esquerda)\n", raiz->indice_variavel + 1);
  if (avaliarArvore(raiz->esquerda, cnf, valores)) return true;

  valores[raiz->indice_variavel] = false;
  printf("Reatribuindo x%d (nivel %d)\nx%d = false (direita)\n",
         raiz->indice_variavel + 1, raiz->indice_variavel,
         raiz->indice_variavel + 1);
  if (avaliarArvore(raiz->direita, cnf, valores)) return true;

  return false;
}

/**
 *  @brief Libera memoria da arvore
 */
void liberarArvore(sat_no_t* raiz) {
  if (raiz == NULL) return;

  liberarArvore(raiz->esquerda);
  liberarArvore(raiz->direita);
  free(raiz);
}

/**
 *  @brief Libera memoria da estrutura CNF
 */
void liberarCNF(sat_cnf_t* cnf) {
  if (!cnf) return;
  if (cnf->clausulas) {
    for (int i = 0; i < cnf->qtdClausulas; i++) {
      free(cnf->clausulas[i]);
    }
    free(cnf->clausulas);
  }
  if (cnf->tamanho) free(cnf->tamanho);
}

int main() {
  sat_cnf_t cnf;                // struct que armazena a CNF lida do arquivo
  lerCNF("exemplo.cnf", &cnf);  //

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