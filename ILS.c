#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//#define MODO_DEBUG
#define MAX_VERTICES 148

int num_maquinas = 7;
char nome_instancia[50]="HAHN.IN2";
 char nome_arquivo[] = "resultado.csv";

bool visitado[MAX_VERTICES];

typedef struct {
  int numero_vertices;
  int **grafo;
  int *lista_custos;
  int *solucao;
} Grafo;

void alocar_memoria(Grafo *g, int numero_vertices) {
  g->numero_vertices = numero_vertices;
  g->grafo = (int **)malloc(numero_vertices * sizeof(int *));
  if (!g->grafo) {
    printf("Erro ao alocar memória para o grafo.\n");
    exit(1);
  }
  for (int i = 0; i < numero_vertices; i++) {
    g->grafo[i] = (int *)calloc(numero_vertices, sizeof(int));
    if (!g->grafo[i]) {
      printf("Erro ao alocar memória para as arestas.\n");
      exit(1);
    }
  }
  g->lista_custos = (int *)malloc(numero_vertices * sizeof(int));
  g->solucao = (int *)malloc(numero_vertices * sizeof(int));
  if (!g->lista_custos || !g->solucao) {
    printf("Erro ao alocar memória para listas.\n");
    exit(1);
  }
}

void liberar_memoria(Grafo *g) {
  for (int i = 0; i < g->numero_vertices; i++) {
    free(g->grafo[i]);
  }
  free(g->grafo);
  free(g->lista_custos);
  free(g->solucao);
}

void criar_arestas(Grafo *g, int arestas[][2], int contador_arestas) {
  for (int i = 0; i < contador_arestas; i++) {
    int u = arestas[i][0] - 1;
    int v = arestas[i][1] - 1;
    g->grafo[u][v] = 1;
  }
}

void ordem_topologica(Grafo *g, int *ordenado) {
  int grau_entrada[MAX_VERTICES] = {0};
  int visitado[MAX_VERTICES] = {0};
  int num_tarefas = g->numero_vertices;


  for (int i = 0; i < num_tarefas; i++) {
    for (int j = 0; j < num_tarefas; j++) {
      if (g->grafo[i][j]) {
        grau_entrada[j]++;
      }
    }
  }


  int prontos[MAX_VERTICES], tamanho_prontos = 0;

  for (int i = 0; i < num_tarefas; i++) {
    if (grau_entrada[i] == 0) {
      prontos[tamanho_prontos++] = i;
    }
  }

  int idx = 0;
  while (tamanho_prontos > 0) {
    for (int i = 0; i < tamanho_prontos - 1; i++) {
      for (int j = i + 1; j < tamanho_prontos; j++) {
        if (g->lista_custos[prontos[i]] > g->lista_custos[prontos[j]]) {
          int temp = prontos[i];
          prontos[i] = prontos[j];
          prontos[j] = temp;
        } else if (g->lista_custos[prontos[i]] == g->lista_custos[prontos[j]]) {
          if (rand() % 2 == 0) {
            int temp = prontos[i];
            prontos[i] = prontos[j];
            prontos[j] = temp;
          }
        }
      }
    }
    int atual = prontos[0];

    for (int i = 0; i < tamanho_prontos - 1; i++) {
      prontos[i] = prontos[i + 1];
    }
    tamanho_prontos--;

    ordenado[idx++] = atual;
    visitado[atual] = 1;
    for (int vizinho = 0; vizinho < num_tarefas; vizinho++) {
      if (g->grafo[atual][vizinho]) {
        grau_entrada[vizinho]--;
        if (grau_entrada[vizinho] == 0 && !visitado[vizinho]) {
          prontos[tamanho_prontos++] = vizinho;
        }
      }
    }
  }
}

void distribuir_tarefas_por_custo(Grafo *g, int *ordenado, int num_tarefas, int num_maquinas, int **distribuicao_maquinas, int *fo) {
  int custo_total = 0;
  for (int i = 0; i < num_tarefas; i++) {
    custo_total += g->lista_custos[ordenado[i]];
  }
  int custo_medio = custo_total / num_maquinas;

  int *calculador_fo = (int *)calloc(num_maquinas, sizeof(int));

  for (int i = 0; i < num_maquinas; i++) {
    distribuicao_maquinas[i][0] = 0;
  }

  int maquina_atual = 0;
  int tarefa_atual = 0;

  while (tarefa_atual < num_tarefas) {
    int tarefa = ordenado[tarefa_atual];

    if ((maquina_atual % 2 != 0 && calculador_fo[maquina_atual] + g->lista_custos[tarefa] <= custo_medio) ||
        (maquina_atual % 2 == 0 && calculador_fo[maquina_atual] + g->lista_custos[tarefa] <= custo_medio + g->lista_custos[tarefa])) {
      distribuicao_maquinas[maquina_atual][++distribuicao_maquinas[maquina_atual][0]] = tarefa;
      calculador_fo[maquina_atual] += g->lista_custos[tarefa];
      tarefa_atual++;
    } else {
      maquina_atual++;
      if (maquina_atual >= num_maquinas) {
        break;
      }
    }
  }

  while (tarefa_atual < num_tarefas) {
    int tarefa = ordenado[tarefa_atual++];
    distribuicao_maquinas[num_maquinas - 1][++distribuicao_maquinas[num_maquinas - 1][0]] = tarefa;
    calculador_fo[num_maquinas - 1] += g->lista_custos[tarefa];
  }

  *fo = 0;
  for (int i = 0; i < num_maquinas; i++) {
    if (calculador_fo[i] > *fo) {
      *fo = calculador_fo[i];
    }
  }

  free(calculador_fo);
}

int verificar_precedencia(Grafo *g, int **distribuicao_maquinas, int num_maquinas) {
  int completado[MAX_VERTICES] = {0};

  for (int i = 0; i < num_maquinas; i++) {
    for (int j = 1; j <= distribuicao_maquinas[i][0]; j++) {
      int tarefa = distribuicao_maquinas[i][j];

      for (int k = 0; k < g->numero_vertices; k++) {
        if (g->grafo[k][tarefa] && !completado[k]) {
          return 0;
        }
      }

      completado[tarefa] = 1;
    }
  }
  return 1;
}

void imprimir_ordem_topologica(int *ordenado, int num_tarefas) {
  printf("Ordem Topológica: ");
  for (int i = 0; i < num_tarefas; i++) {
    printf("%d ", ordenado[i] + 1);
  }
  printf("\n");
}

void imprimir_distribuicao_maquinas(int **distribuicao_maquinas, int num_maquinas, int num_tarefas, Grafo *g) {
  int valor = 0;
  printf("Distribuição de Máquinas:\n");
  for (int i = 0; i < num_maquinas; i++) {
    printf("Máquina %d: ", i + 1);
    for (int j = 1; j <= distribuicao_maquinas[i][0]; j++) {
      printf("%d ", distribuicao_maquinas[i][j] + 1);
      valor = valor + g->lista_custos[distribuicao_maquinas[i][j]];
    }

    printf("\n valor da Máquina %d: %d\n", i + 1, valor);
    printf("\n");
    valor = 0;
  }
}

int calcular_fo(Grafo *g, int **distribuicao_maquinas, int num_maquinas) {
  int max_fo = 0;
  for (int i = 0; i < num_maquinas; i++) {
    int soma = 0;
    for (int j = 1; j <= distribuicao_maquinas[i][0]; j++) {
      soma += g->lista_custos[distribuicao_maquinas[i][j]];
    }
    if (soma > max_fo) {
      max_fo = soma;
    }
  }
  return max_fo;
}

void busca_local(Grafo *g, int **distribuicao_maquinas, int *fo, int num_maquinas, int *ordenado_perturbado, int num_tarefas) {
  int melhor_fo = *fo;
  int melhorou = 0;

  do {
    melhorou = 0;
    for (int origem = 0; origem < num_maquinas - 1; origem++) {
      if (distribuicao_maquinas[origem][0] > 0) {
        int indice_ultima_tarefa = distribuicao_maquinas[origem][0];
        int tarefa = distribuicao_maquinas[origem][indice_ultima_tarefa];
        distribuicao_maquinas[origem][0]--;

        for (int j = distribuicao_maquinas[origem + 1][0]; j >= 1; j--) {
          distribuicao_maquinas[origem + 1][j + 1] = distribuicao_maquinas[origem + 1][j];
        }
        distribuicao_maquinas[origem + 1][1] = tarefa;
        distribuicao_maquinas[origem + 1][0]++;

        if (verificar_precedencia(g, distribuicao_maquinas, num_maquinas)) {

          int nova_fo = calcular_fo(g, distribuicao_maquinas, num_maquinas);
          if (nova_fo < melhor_fo) {
            melhor_fo = nova_fo;
            *fo = nova_fo;
            melhorou = 1;
            break;
          }
        }

        for (int j = 1; j < distribuicao_maquinas[origem + 1][0]; j++) {
          distribuicao_maquinas[origem + 1][j] = distribuicao_maquinas[origem + 1][j + 1];
        }
        distribuicao_maquinas[origem + 1][0]--;
        distribuicao_maquinas[origem][++distribuicao_maquinas[origem][0]] = tarefa;
      }
    }

    if (melhorou) {
      continue;
    }

    for (int destino = num_maquinas - 1; destino > 0; destino--) {
      if (distribuicao_maquinas[destino][0] > 0) {
        int primeira_tarefa = distribuicao_maquinas[destino][1];

        for (int j = 1; j < distribuicao_maquinas[destino][0]; j++) {
          distribuicao_maquinas[destino][j] = distribuicao_maquinas[destino][j + 1];
        }
        distribuicao_maquinas[destino][0]--;

        distribuicao_maquinas[destino - 1][++distribuicao_maquinas[destino - 1][0]] = primeira_tarefa;

        if (verificar_precedencia(g, distribuicao_maquinas, num_maquinas)) {
          int nova_fo = calcular_fo(g, distribuicao_maquinas, num_maquinas);
          if (nova_fo < melhor_fo) {
            melhor_fo = nova_fo;
            *fo = nova_fo;
            melhorou = 1;
            break;
          }
        }

        distribuicao_maquinas[destino - 1][distribuicao_maquinas[destino - 1][0]--] = 0;

        for (int j = distribuicao_maquinas[destino][0]; j >= 1; j--) {
          distribuicao_maquinas[destino][j + 1] = distribuicao_maquinas[destino][j];
        }
        distribuicao_maquinas[destino][1] = primeira_tarefa;
        distribuicao_maquinas[destino][0]++;
      }
    }

  } while (melhorou);
}

void perturbacao(Grafo *g, int *ordenado, int num_tarefas, int *fo, int **distribuicao_maquinas) {
  int nova_ordem[num_tarefas];
  bool incluido[MAX_VERTICES] = {false};
  int indice = num_tarefas - 1;

  int ultima_tarefa = ordenado[num_tarefas - 1];
  nova_ordem[indice--] = ultima_tarefa;
  incluido[ultima_tarefa] = true;

  while (indice >= 0) {
    bool adicionado = false;
    int candidatos[MAX_VERTICES];
    int contador_candidatos = 0;

    for (int i = num_tarefas - 1; i >= 0; i--) {
      int candidato = ordenado[i];
      if (!incluido[candidato]) {
        bool todos_sucessores_incluidos = true;

        for (int j = 0; j < num_tarefas; j++) {
          if (g->grafo[candidato][j] && !incluido[j]) {
            todos_sucessores_incluidos = false;
            break;
          }
        }

        if (todos_sucessores_incluidos) {
          candidatos[contador_candidatos++] = candidato;
        }
      }
    }

    if (contador_candidatos > 0) {
      int indice_escolhido = rand() % contador_candidatos;
      int candidato_escolhido = candidatos[indice_escolhido];

      nova_ordem[indice--] = candidato_escolhido;
      incluido[candidato_escolhido] = true;
      adicionado = true;
    }

    if (!adicionado) {
      printf("Erro na perturbação: Não foi possível gerar uma nova ordem "
             "válida.\n");
      return;
    }
  }
  distribuir_tarefas_por_custo(g, nova_ordem, num_tarefas, num_maquinas, distribuicao_maquinas, fo);

  memcpy(ordenado, nova_ordem, num_tarefas * sizeof(int));
}

void salvar_dados(int m, int fo, double t) {

  FILE *arquivo = fopen(nome_arquivo, "a");
  if (arquivo == NULL) {
    printf("Erro ao abrir o arquivo!\n");
    return;
  }

  fprintf(arquivo, "%d;%d;%.3f\n", m, fo, t);

  fclose(arquivo);

  printf("Resultados salvos em %s\n", nome_arquivo);
}

void ILS(Grafo *g, int *ordenado, int num_tarefas, int num_maquinas, int **distribuicao_maquinas, int *fo, int iteracao) {
  int melhor_fo = *fo;
  int melhor_distribuicao[num_maquinas][MAX_VERTICES];

  for (int i = 0; i < num_maquinas; i++) {
    memcpy(melhor_distribuicao[i], distribuicao_maquinas[i], (distribuicao_maquinas[i][0] + 1) * sizeof(int));
  }

  clock_t inicio = clock();
  for (int i = 0; i < iteracao; i++) {

    int ordenado_perturbado[MAX_VERTICES];
    memcpy(ordenado_perturbado, ordenado, num_tarefas * sizeof(int));

    perturbacao(g, ordenado_perturbado, num_tarefas, fo, distribuicao_maquinas);

    busca_local(g, distribuicao_maquinas, fo, num_maquinas, ordenado_perturbado, num_tarefas);

    if (*fo < melhor_fo) {
      melhor_fo = *fo;
      for (int j = 0; j < num_maquinas; j++) {
        memcpy(melhor_distribuicao[j], distribuicao_maquinas[j], (distribuicao_maquinas[j][0] + 1) * sizeof(int));
      }
    } else {
      for (int j = 0; j < num_maquinas; j++) {
        memcpy(distribuicao_maquinas[j], melhor_distribuicao[j], (melhor_distribuicao[j][0] + 1) * sizeof(int));
      }
      *fo = melhor_fo;
    }
  }
#ifdef MODO_DEBUG
  printf("\n");
  imprimir_distribuicao_maquinas(distribuicao_maquinas, num_maquinas, num_tarefas, g);
  printf("Função Final: %d\n", melhor_fo);
#endif
  clock_t fim = clock();
  printf("m: %d\nFO: %d\n t: %.2f \n", num_maquinas, melhor_fo, ((double)(fim - inicio)) / CLOCKS_PER_SEC);

  salvar_dados(num_maquinas, melhor_fo,((double)(fim - inicio)) / CLOCKS_PER_SEC);
}

int main(int argc, char *argv[]) {

  if (argc != 2) {
    printf("Uso: %s <numero>\n", argv[0]);
    return 1;
  }

  num_maquinas = atoi(argv[1]);


  srand(time(NULL));

  FILE *arquivo = fopen(nome_instancia, "r");
  if (!arquivo) {
    printf("Erro ao abrir o arquivo.\n");
    return 1;
  }

  int numero_vertices;
  if (fscanf(arquivo, "%d", &numero_vertices) != 1 || numero_vertices <= 0) {
    printf("Erro ao ler o número de vértices.\n");
    fclose(arquivo);
    return 1;
  }

  Grafo g;
  alocar_memoria(&g, numero_vertices);

  for (int i = 0; i < numero_vertices; i++) {
    if (fscanf(arquivo, "%d", &g.lista_custos[i]) != 1) {
      printf("Erro ao ler a lista de custos.\n");
      liberar_memoria(&g);
      fclose(arquivo);
      return 1;
    }
  }

  int arestas[numero_vertices * numero_vertices][2], contador_arestas = 0;
  while (1) {
    int u, v;
    if (fscanf(arquivo, "%d,%d", &u, &v) != 2) {
      printf("Erro ao ler as arestas.\n");
      liberar_memoria(&g);
      fclose(arquivo);
      return 1;
    }
    if (u == -1 && v == -1) {
      break;
    }
    arestas[contador_arestas][0] = u;
    arestas[contador_arestas][1] = v;
    contador_arestas++;
  }
  fclose(arquivo);

  criar_arestas(&g, arestas, contador_arestas);

  int ordenado[MAX_VERTICES];
  ordem_topologica(&g, ordenado);
#ifdef MODO_DEBUG
  imprimir_ordem_topologica(ordenado, numero_vertices);
#endif
  int **distribuicao_maquinas = (int **)malloc(num_maquinas * sizeof(int *));
  for (int i = 0; i < num_maquinas; i++) {
    distribuicao_maquinas[i] = (int *)malloc((numero_vertices + 1) * sizeof(int));
  }

  int fo;
  printf("\n");
  distribuir_tarefas_por_custo(&g, ordenado, numero_vertices, num_maquinas, distribuicao_maquinas, &fo);

#ifdef MODO_DEBUG
  imprimir_distribuicao_maquinas(distribuicao_maquinas, num_maquinas, numero_vertices, &g);
#endif

  busca_local(&g, distribuicao_maquinas, &fo, num_maquinas, ordenado, numero_vertices);

#ifdef MODO_DEBUG
  printf("\n");

  imprimir_distribuicao_maquinas(distribuicao_maquinas, num_maquinas, numero_vertices, &g);

  printf("Função após busca local: %d\n", fo);
#endif

  ILS(&g, ordenado, numero_vertices, num_maquinas, distribuicao_maquinas, &fo,200000);

  printf("\n");

  for (int i = 0; i < num_maquinas; i++) {
    free(distribuicao_maquinas[i]);
  }
  free(distribuicao_maquinas);
  liberar_memoria(&g);

  return 0;
}
