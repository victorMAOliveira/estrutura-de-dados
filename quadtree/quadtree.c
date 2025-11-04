#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAPACIDADE 4

typedef struct {
  int x, y;
  char nome[50];
} Ponto;

typedef struct {
  int x, y;
  int w, h;
} Caixa;

typedef struct QT {
  Caixa box;
  Ponto* pts[CAPACIDADE];
  int n;
  int dividido;
  struct QT *nw, *ne, *sw, *se;
} QT;

int dentro(Caixa* b, Ponto* p) {
  return (p->x >= b->x - b->w && p->x <= b->x + b->w && p->y >= b->y - b->h &&
          p->y <= b->y + b->h);
}

QT* novaQT(Caixa box) {
  QT* q = (QT*)malloc(sizeof(QT));
  q->box = box;
  q->n = 0;
  q->dividido = 0;
  q->nw = q->ne = q->sw = q->se = NULL;
  return q;
}

void dividir(QT* q) {
  int x = q->box.x, y = q->box.y;
  int w = q->box.w / 2, h = q->box.h / 2;

  q->nw = novaQT((Caixa){x - w, y + h, w, h});
  q->ne = novaQT((Caixa){x + w, y + h, w, h});
  q->sw = novaQT((Caixa){x - w, y - h, w, h});
  q->se = novaQT((Caixa){x + w, y - h, w, h});
  q->dividido = 1;
}

int add(QT* q, Ponto* p) {
  if (!dentro(&q->box, p)) return 0;
  if (q->n < CAPACIDADE) {
    q->pts[q->n++] = p;
    return 1;
  }
  if (!q->dividido) dividir(q);
  if (add(q->nw, p)) return 1;
  if (add(q->ne, p)) return 1;
  if (add(q->sw, p)) return 1;
  if (add(q->se, p)) return 1;
  return 0;
}

void imprime(QT* q) {
  for (int i = 0; i < q->n; i++)
    printf("(%d,%d) - %s\n", q->pts[i]->x, q->pts[i]->y, q->pts[i]->nome);
  if (q->dividido) {
    imprime(q->nw);
    imprime(q->ne);
    imprime(q->sw);
    imprime(q->se);
  }
}

int emRaio(Ponto* c, Ponto* p, int r) {
  int dx = c->x - p->x, dy = c->y - p->y;
  return dx * dx + dy * dy <= r * r;
}

void busca(QT* q, Ponto* c, int r) {
  if (!dentro(&q->box, c)) return;
  for (int i = 0; i < q->n; i++)
    if (emRaio(c, q->pts[i], r))
      printf("→ (%d,%d) - %s dentro do raio\n", q->pts[i]->x, q->pts[i]->y,
             q->pts[i]->nome);
  if (q->dividido) {
    busca(q->nw, c, r);
    busca(q->ne, c, r);
    busca(q->sw, c, r);
    busca(q->se, c, r);
  }
}

int d2(Ponto* a, Ponto* b) {
  int dx = a->x - b->x, dy = a->y - b->y;
  return dx * dx + dy * dy;
}

void vizinho(QT* q, Ponto* alvo, Ponto** melhor, int* melhorD) {
  if (!dentro(&q->box, alvo)) return;
  for (int i = 0; i < q->n; i++) {
    int dist = d2(alvo, q->pts[i]);
    if (dist < *melhorD) {
      *melhorD = dist;
      *melhor = q->pts[i];
    }
  }
  if (q->dividido) {
    vizinho(q->nw, alvo, melhor, melhorD);
    vizinho(q->ne, alvo, melhor, melhorD);
    vizinho(q->sw, alvo, melhor, melhorD);
    vizinho(q->se, alvo, melhor, melhorD);
  }
}

const char* quad(Caixa* b, Ponto* p) {
  if (p->x < b->x && p->y > b->y) return "NW";
  if (p->x >= b->x && p->y > b->y) return "NE";
  if (p->x < b->x && p->y <= b->y) return "SW";
  if (p->x >= b->x && p->y <= b->y) return "SE";
  return "Fora";
}

void mostraQuad(QT* q, const char* nome) {
  for (int i = 0; i < q->n; i++) {
    if (strcmp(q->pts[i]->nome, nome) == 0) {
      printf("'%s' está no quadrante: %s\n", nome, quad(&q->box, q->pts[i]));
      return;
    }
  }
  if (q->dividido) {
    mostraQuad(q->nw, nome);
    mostraQuad(q->ne, nome);
    mostraQuad(q->sw, nome);
    mostraQuad(q->se, nome);
  }
}

void inserirDeArquivo(QT* q, const char* nomeArquivo) {
  FILE* f = fopen(nomeArquivo, "r");
  if (!f) {
    printf("Erro ao abrir o arquivo '%s'\n", nomeArquivo);
    return;
  }
  int x, y;
  char nome[50];
  while (fscanf(f, "%d %d %49s", &x, &y, nome) == 3) {
    Ponto* p = (Ponto*)malloc(sizeof(Ponto));
    p->x = x;
    p->y = y;
    strcpy(p->nome, nome);
    if (!add(q, p))
      printf("(!) Ponto (%d,%d) - %s fora dos limites\n", x, y, nome);
  }
  fclose(f);
  printf("→ Inserção concluída a partir do arquivo '%s'\n", nomeArquivo);
}

int main() {
  Caixa box;
  printf("Limites (centro: x y | largura altura): ");
  scanf("%d %d %d %d", &box.x, &box.y, &box.w, &box.h);

  QT* q = novaQT(box);

  int op;
  do {
    printf("\n=== QUADTREE ===\n");
    printf(
        "1. Inserir (manual ou arquivo)\n2. Listar\n3. Buscar por raio\n4. "
        "Vizinho mais próximo\n");
    printf("5. Quadrante do ponto\n0. Sair\nOpção: ");
    scanf("%d", &op);

    if (op == 1) {
      int modo;
      printf("1. Inserir manualmente\n2. Ler de arquivo .txt\nEscolha: ");
      scanf("%d", &modo);
      if (modo == 1) {
        Ponto* p = (Ponto*)malloc(sizeof(Ponto));
        printf("X Y Nome: ");
        scanf("%d %d %49s", &p->x, &p->y, p->nome);
        if (!add(q, p)) printf("Fora dos limites!\n");
      } else if (modo == 2) {
        char nomeArquivo[100];
        printf("Nome do arquivo: ");
        scanf("%99s", nomeArquivo);
        inserirDeArquivo(q, nomeArquivo);
      } else {
        printf("Opção inválida.\n");
      }
    } else if (op == 2) {
      imprime(q);
    } else if (op == 3) {
      Ponto c;
      int r;
      printf("Centro X Y e raio: ");
      scanf("%d %d %d", &c.x, &c.y, &r);
      busca(q, &c, r);
    } else if (op == 4) {
      Ponto alvo;
      printf("X Y: ");
      scanf("%d %d", &alvo.x, &alvo.y);
      Ponto* melhor = NULL;
      int md = INT_MAX;
      vizinho(q, &alvo, &melhor, &md);
      if (melhor)
        printf("→ Vizinho: (%d,%d) - %s\n", melhor->x, melhor->y, melhor->nome);
      else
        printf("Nenhum ponto.\n");
    } else if (op == 5) {
      char n[50];
      printf("Nome do ponto: ");
      scanf("%49s", n);
      mostraQuad(q, n);
    } else if (op == 0) {
      printf("Tchau!\n");
    } else {
      printf("Opção inválida.\n");
    }
  } while (op != 0);

  return 0;
}