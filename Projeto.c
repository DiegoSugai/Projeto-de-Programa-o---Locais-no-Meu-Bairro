/*
 * PROJETO DE TEORIA DOS GRAFOS - PARTE 2
 * MODELAGEM DE BAIRRO E CALCULO DE PASSEIO
 *
 * Diego Spagnuolo Sugai - RA 10417329
 * Kaue Henrique Matias Alves - RA: 10417894
 * Victor Maki tarcha - RA 10419861
 * Marcos Arambasic - RA 10443260
 * * Para compilar e rodar:
 * gcc Projeto.c -o programa -ansi -pedantic
 * ./programa
 * */

/* Bibliotecas */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h> 

#define BRANCO 0
#define CINZA  1
#define PRETO  2

/* Estrutura de dados para representar grafos */
typedef struct a{ /* Celula de uma lista de arestas */
    int    extremo2;
    int    peso;     /* Distancia em metros */
    struct a *prox;
} Arest;

typedef struct v{  /* Cada vertice tem um ponteiro para uma lista de arestas incidentes nele */
    int nome;
    int cor;
    Arest *prim;
} Vert;

/* Estrutura de Localidades */
typedef struct {
    const char* nome; /* Nome do local */
    int vert1;        /* Um dos vertices da rua (esquina 1) */
    int vert2;        /* O outro vertice da rua (esquina 2) */
    int dist_vert1;   /* Distancia em metros do local ate o vert1 */
    int dist_vert2;   /* Distancia em metros do local ate o vert2 */
} Localidades;

/* Prototipos */
void criaGrafo(Vert **G, int ordem);
void destroiGrafo(Vert **G, int ordem);
int  acrescentaAresta(Vert G[], int ordem, int vert1, int vert2, int peso);
void imprimeGrafo(Vert G[], int ordem);

int calculaDistanciaModificado(Vert G[], int ordem, int origem, int destino, int **caminho_vertices, int *tamanho_caminho);
int getDistanciaEntrePOIs(Vert G[], int ordem, Localidades poi_A, Localidades poi_B, int **melhor_caminho, int *tam_melhor_caminho);
void calcularPasseio(Vert G[], int ordem, Localidades minha_casa, Localidades locais[], int num_locais);
/* Funcao auxiliar agora aceita o 'modo' para saber como imprimir */
void encontrarPOIsNoCaminho(int v1, int v2, Localidades locais_todos[], int num_locais_todos, const char* nome_destino, int modo);

/*
 * Implementacao das Funcoes (Basicas)
 */

void criaGrafo(Vert **G, int ordem){
    int i;
    *G = (Vert*) malloc(sizeof(Vert) * ordem);  
    if (*G == NULL) {
        perror("Erro ao alocar memoria para o grafo");
        exit(1);
    }
    for(i=0; i<ordem; i++){
        (*G)[i].nome = i;
        (*G)[i].cor = BRANCO;
        (*G)[i].prim = NULL;
    }
}

void destroiGrafo(Vert **G, int ordem){
    int i;
    Arest *a, *n;
    for(i=0; i<ordem; i++){
        a = (*G)[i].prim;
        while (a!= NULL){
            n = a->prox;
            free(a);
            a = n;
        }
    }
    free(*G);
}

int acrescentaAresta(Vert G[], int ordem, int vert1, int vert2, int peso){
    Arest *A1, *A2;
    if (vert1<0 || vert1 >= ordem || vert2<0 || vert2 >= ordem) return 0;

    A1 = (Arest *) malloc(sizeof(Arest));
    A1->extremo2 = vert2;
    A1->peso = peso;
    A1->prox = G[vert1].prim;
    G[vert1].prim = A1;

    if (vert1 == vert2) return 1;

    A2 = (Arest *) malloc(sizeof(Arest));
    A2->extremo2 = vert1;
    A2->peso = peso;
    A2->prox = G[vert2].prim;
    G[vert2].prim = A2;
    
    return 1;
}

void imprimeGrafo(Vert G[], int ordem){
    int i;
    Arest *aux;

    printf("\nOrdem:   %d",ordem);
    printf("\nLista de Adjacencia:\n");

    for (i=0; i<ordem; i++){
        printf("\n    v%d: ", i);
        aux = G[i].prim;
        for( ; aux != NULL; aux = aux->prox) {
            /* Formatacao de float para mostrar a distancia */
            printf("  v%d(%.2fm)", aux->extremo2, (float)aux->peso);
        }
    }
    printf("\n\n");
}

/* --- PARTE 2 --- */
/* Dijkstra modificado para retornar o caminho (sequencia de vertices) e a distancia. */
int calculaDistanciaModificado(Vert G[], int ordem, int origem, int destino, int **caminho_vertices, int *tamanho_caminho) {
    int *dist = (int *) malloc(ordem * sizeof(int));
    int *visitado = (int *) malloc(ordem * sizeof(int));
    int *predecessor = (int *) malloc(ordem * sizeof(int)); 
    
    /* Variaveis ANSI C (no topo) */
    int i, u, count;
    int min, min_index;
    int v, peso;
    int distanciaFinal;
    int *caminho_reverso;
    int atual, tam;
    Arest *adj;

    for (i = 0; i < ordem; i++) {
        dist[i] = INT_MAX;
        visitado[i] = 0;
        predecessor[i] = -1; /* -1 indica sem predecessor */
    }
    
    dist[origem] = 0;

    for (count = 0; count < ordem; count++) {
        
        min = INT_MAX; 
        min_index = -1;
        
        for (i = 0; i < ordem; i++) {
            if (visitado[i] == 0 && dist[i] <= min) {
                min = dist[i];
                min_index = i;
            }
        }

        if(min_index == -1) break; 
        
        u = min_index;
        visitado[u] = 1;
        
        if (u == destino) break; /* Encontrou o destino */
        
        adj = G[u].prim;
        
        while (adj != NULL) {
            v = adj->extremo2;
            peso = adj->peso;

            if (!visitado[v] && dist[u] != INT_MAX && dist[u] + peso < dist[v]) {
                dist[v] = dist[u] + peso;
                predecessor[v] = u; /* Guarda o predecessor */
            }
            adj = adj->prox;
        }
    }

    distanciaFinal = dist[destino];
    
    /* --- Reconstrucao do Caminho --- */
    if (distanciaFinal != INT_MAX) {
        caminho_reverso = (int *) malloc(ordem * sizeof(int));
        atual = destino;
        tam = 0;
        while(atual != -1) {
            caminho_reverso[tam++] = atual;
            atual = predecessor[atual];
        }
        
        /* Inverte o caminho para ter a ordem correta (Origem -> Destino) */
        *caminho_vertices = (int *) malloc(tam * sizeof(int));
        for(i = 0; i < tam; i++) {
            (*caminho_vertices)[i] = caminho_reverso[tam - 1 - i];
        }
        *tamanho_caminho = tam;
        
        free(caminho_reverso);
    } else {
        *caminho_vertices = NULL;
        *tamanho_caminho = 0;
    }
    
    free(dist);
    free(visitado);
    free(predecessor); 
    
    return distanciaFinal;
}

/* --- PARTE 2 --- */
/*
 * Calcula a distancia real entre dois locais (POIs),
 * ja incluindo as distancias parciais nas ruas.
 * Testa as 4 combinacoes de vertices e retorna o menor caminho.
 */
int getDistanciaEntrePOIs(Vert G[], int ordem, Localidades poi_A, Localidades poi_B, int **melhor_caminho, int *tam_melhor_caminho) {
    
    int dist_total = INT_MAX;
    /* Variaveis ANSI C (no topo) */
    int *caminho_atual = NULL;
    int tam_caminho_atual = 0;
    int dist_vA1_vB1, dist_c1;
    int dist_vA1_vB2, dist_c2;
    int dist_vA2_vB1, dist_c3;
    int dist_vA2_vB2, dist_c4;

    /* Libera o ponteiro *melhor_caminho* ANTES de usa-lo */
    if(*melhor_caminho) free(*melhor_caminho);
    *melhor_caminho = NULL; 
    *tam_melhor_caminho = 0;

    /* --- Cenario 1: A(via vA1) -> ... -> B(via vB1) --- */
    dist_vA1_vB1 = calculaDistanciaModificado(G, ordem, poi_A.vert1, poi_B.vert1, &caminho_atual, &tam_caminho_atual);
    if (dist_vA1_vB1 != INT_MAX) {
        dist_c1 = poi_A.dist_vert1 + dist_vA1_vB1 + poi_B.dist_vert1;
        if (dist_c1 < dist_total) {
            dist_total = dist_c1;
            if(*melhor_caminho) free(*melhor_caminho); 
            *melhor_caminho = caminho_atual;
            *tam_melhor_caminho = tam_caminho_atual;
        } else {
            if (caminho_atual) free(caminho_atual); 
        }
    }
    caminho_atual = NULL;

    /* --- Cenario 2: A(via vA1) -> ... -> B(via vB2) --- */
    dist_vA1_vB2 = calculaDistanciaModificado(G, ordem, poi_A.vert1, poi_B.vert2, &caminho_atual, &tam_caminho_atual);
     if (dist_vA1_vB2 != INT_MAX) {
        dist_c2 = poi_A.dist_vert1 + dist_vA1_vB2 + poi_B.dist_vert2;
        if (dist_c2 < dist_total) {
            dist_total = dist_c2;
            if(*melhor_caminho) free(*melhor_caminho);
            *melhor_caminho = caminho_atual;
            *tam_melhor_caminho = tam_caminho_atual;
        } else {
            if (caminho_atual) free(caminho_atual);
        }
    }
    caminho_atual = NULL;

    /* --- Cenario 3: A(via vA2) -> ... -> B(via vB1) --- */
    dist_vA2_vB1 = calculaDistanciaModificado(G, ordem, poi_A.vert2, poi_B.vert1, &caminho_atual, &tam_caminho_atual);
     if (dist_vA2_vB1 != INT_MAX) {
        dist_c3 = poi_A.dist_vert2 + dist_vA2_vB1 + poi_B.dist_vert1;
        if (dist_c3 < dist_total) {
            dist_total = dist_c3;
            if(*melhor_caminho) free(*melhor_caminho);
            *melhor_caminho = caminho_atual;
            *tam_melhor_caminho = tam_caminho_atual;
        } else {
            if (caminho_atual) free(caminho_atual);
        }
    }
    caminho_atual = NULL;

    /* --- Cenario 4: A(via vA2) -> ... -> B(via vB2) --- */
    dist_vA2_vB2 = calculaDistanciaModificado(G, ordem, poi_A.vert2, poi_B.vert2, &caminho_atual, &tam_caminho_atual);
     if (dist_vA2_vB2 != INT_MAX) {
        dist_c4 = poi_A.dist_vert2 + dist_vA2_vB2 + poi_B.dist_vert2;
        if (dist_c4 < dist_total) {
            dist_total = dist_c4;
            if(*melhor_caminho) free(*melhor_caminho);
            *melhor_caminho = caminho_atual;
            *tam_melhor_caminho = tam_caminho_atual;
        } else {
            if (caminho_atual) free(caminho_atual);
        }
    }

    return dist_total;
}

/* --- PARTE 2 --- */
/* Funcao para calcular o passeio (TSP) com MENUS de visualizacao e VALIDACAO */
void calcularPasseio(Vert G[], int ordem, Localidades minha_casa, Localidades locais[], int num_locais) {
    
    /* Variaveis ANSI C (no topo) */
    int num_visita = num_locais + 1; 
    Localidades *conjunto_visita;
    int i, j, k;
    int *ordem_visita;
    int *visitados;
    int local_atual_idx;
    int distancia_total_passeio;
    int melhor_dist_trecho;
    int proximo_local_idx;
    int *melhor_caminho_trecho;
    int tam_melhor_caminho_trecho;
    int *caminho_teste;
    int tam_caminho_teste;
    int dist_teste;
    Localidades loc_A, loc_B;
    int u, v;
    int *caminho_final;
    int tam_caminho_final;
    int dist_volta;
    int modo_exibicao;

    /* Aloca memoria */
    conjunto_visita = (Localidades*) malloc(num_visita * sizeof(Localidades));
    if (conjunto_visita == NULL) {
        perror("Erro ao alocar memoria para conjunto_visita");
        return;
    }

    /* Posicao 0 eh sempre "minha casa" */
    conjunto_visita[0] = minha_casa; 
    
    /* Copia os locais */
    for(i = 0; i < num_locais; i++) {
        conjunto_visita[i+1] = locais[i];
    }

    /* --- Exibir menu de escolha COM VALIDACAO --- */
    printf("\n  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("  |            Calculando Passeio (Parte 2)            |\n");
    printf("  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");
    printf("Escolha o modo de visualizacao:\n");
    printf(" 1 - Detalhado (Trecho a trecho, com distancias)\n");
    printf(" 2 - Unificado (Fluxo continuo: Inicio -> v1 -> ... -> Fim)\n");
    
    /* VALIDACAO DE ENTRADA */
    while (1) {
        printf("Opcao: ");
        scanf("%d", &modo_exibicao);
        
        if (modo_exibicao == 1 || modo_exibicao == 2) {
            break; /* Entrada valida */
        } else {
            printf("\nOpcao invalida! Por favor, digite 1 ou 2.\n");
            /* Limpa o buffer de entrada para evitar loop infinito caso digite letra */
            while (getchar() != '\n'); 
        }
    }

    printf("\nIniciando passeio em: %s\n", conjunto_visita[0].nome);
    printf("Visitando os seguintes %d locais...\n", num_visita - 1);
    printf("\n--------------------------------------------------------\n");
    
    /* Se for modo 2 (Unificado), imprime o inicio ja no formato de fluxo */
    if (modo_exibicao == 2) {
        printf("INICIO (%s)", conjunto_visita[0].nome);
    }

    /* Algoritmo TSP (Vizinho Mais Proximo) */
    ordem_visita = (int*) malloc(num_visita * sizeof(int)); 
    visitados = (int*) calloc(num_visita, sizeof(int)); 
    
    local_atual_idx = 0; 
    ordem_visita[0] = local_atual_idx;
    visitados[local_atual_idx] = 1;
    
    distancia_total_passeio = 0;

    for (i = 1; i < num_visita; i++) { 
        
        melhor_dist_trecho = INT_MAX;
        proximo_local_idx = -1;
        melhor_caminho_trecho = NULL;
        tam_melhor_caminho_trecho = 0;

        /* Busca vizinho mais proximo */
        for (k = 0; k < num_visita; k++) {
            if (visitados[k] == 0) { 
                caminho_teste = NULL;
                tam_caminho_teste = 0;
                
                dist_teste = getDistanciaEntrePOIs(G, ordem, 
                                    conjunto_visita[local_atual_idx], 
                                    conjunto_visita[k], 
                                    &caminho_teste, &tam_caminho_teste);
                                    
                if (dist_teste < melhor_dist_trecho) {
                    melhor_dist_trecho = dist_teste;
                    proximo_local_idx = k;
                    
                    melhor_caminho_trecho = caminho_teste;
                    tam_melhor_caminho_trecho = tam_caminho_teste;
                } else {
                    if (caminho_teste) free(caminho_teste); 
                }
            }
        }
        
        if (proximo_local_idx == -1) {
            printf("\n\nERRO: Caminho interrompido.\n");
            if (melhor_caminho_trecho) free(melhor_caminho_trecho);
            break; 
        }
        
        loc_A = conjunto_visita[local_atual_idx];
        loc_B = conjunto_visita[proximo_local_idx];

        /* --- IMPRESSAO --- */
        if (modo_exibicao == 1) {
            /* MODO DETALHADO */
            printf("\n%d. Trecho: %s -> %s\n", i, loc_A.nome, loc_B.nome);
            printf("   - Distancia: %d metros\n", melhor_dist_trecho);
            /* Formato Start -> v... -> End */
            printf("   - Caminho: %s", loc_A.nome); 
            for(j = 0; j < tam_melhor_caminho_trecho; j++) {
                printf(" -> v%d", melhor_caminho_trecho[j]);
                if (j < tam_melhor_caminho_trecho - 1) {
                    encontrarPOIsNoCaminho(melhor_caminho_trecho[j], melhor_caminho_trecho[j+1], locais, num_locais, loc_B.nome, 1);
                }
            }
            printf(" -> %s\n", loc_B.nome);
        } 
        else {
            /* MODO UNIFICADO */
            if (tam_melhor_caminho_trecho > 0) {
                printf(" -> v%d", melhor_caminho_trecho[0]);
            }
            for(j = 0; j < tam_melhor_caminho_trecho - 1; j++) {
                u = melhor_caminho_trecho[j];
                v = melhor_caminho_trecho[j+1];
                encontrarPOIsNoCaminho(u, v, locais, num_locais, "", 2);
                printf(" -> v%d", v);
            }
            printf(" -> %s", loc_B.nome);
        }

        if (melhor_caminho_trecho) free(melhor_caminho_trecho);
        
        distancia_total_passeio += melhor_dist_trecho;
        local_atual_idx = proximo_local_idx;
        ordem_visita[i] = local_atual_idx;
        visitados[local_atual_idx] = 1;
    }
    
    /* --- Volta para casa --- */
    caminho_final = NULL;
    tam_caminho_final = 0;
    dist_volta = getDistanciaEntrePOIs(G, ordem, 
                        conjunto_visita[local_atual_idx], 
                        conjunto_visita[0],               
                        &caminho_final, &tam_caminho_final);

    if (modo_exibicao == 1) {
        printf("\n%d. Trecho (Volta): %s -> %s\n", num_visita, conjunto_visita[local_atual_idx].nome, conjunto_visita[0].nome);
        printf("   - Distancia: %d metros\n", dist_volta);
        printf("   - Caminho: %s", conjunto_visita[local_atual_idx].nome);
        for(j = 0; j < tam_caminho_final; j++) {
            printf(" -> v%d", caminho_final[j]);
            if (j < tam_caminho_final - 1) {
                encontrarPOIsNoCaminho(caminho_final[j], caminho_final[j+1], locais, num_locais, conjunto_visita[0].nome, 1);
            }
        }
        printf(" -> %s\n", conjunto_visita[0].nome);
    } else {
        if (tam_caminho_final > 0) {
            printf(" -> v%d", caminho_final[0]);
        }
        for(j = 0; j < tam_caminho_final - 1; j++) {
            u = caminho_final[j];
            v = caminho_final[j+1];
            encontrarPOIsNoCaminho(u, v, locais, num_locais, "", 2);
            printf(" -> v%d", v);
        }
        printf(" -> FIM (Minha Casa)\n");
    }

    if (caminho_final) free(caminho_final);

    distancia_total_passeio += dist_volta;

    /* --- Exibir Resultado Final --- */
    printf("\n\n--------------------------------------------------------");
    printf("\nDistancia Total Percorrida: %d metros\n", distancia_total_passeio);
    
    if (modo_exibicao == 1) {
        printf("Sequencia de locais visitados:\n");
        for(i = 0; i < num_visita; i++) {
            printf("  %d. %s\n", i+1, conjunto_visita[ordem_visita[i]].nome);
        }
        printf("  %d. %s (Volta)\n", num_visita+1, conjunto_visita[0].nome);
    }
    printf("--------------------------------------------------------\n\n");
    
    free(conjunto_visita); 
    free(ordem_visita);
    free(visitados);
}

/* --- PARTE 2 --- */
/* Funcao auxiliar para encontrar POIs:
   Modo 1: Imprime (Passando por: Nome)
   Modo 2: Imprime -> Nome
*/
void encontrarPOIsNoCaminho(int v1, int v2, Localidades locais_todos[], int num_locais_todos, const char* nome_destino, int modo) {
    int i;
    for (i = 0; i < num_locais_todos; i++) {
        if ((locais_todos[i].vert1 == v1 && locais_todos[i].vert2 == v2) ||
            (locais_todos[i].vert1 == v2 && locais_todos[i].vert2 == v1)) {
            
            if (strcmp(locais_todos[i].nome, nome_destino) != 0) {
                 if (modo == 1) {
                     /* Detalhado */
                     printf("\n     (Passando por: %s)", locais_todos[i].nome);
                 } else {
                     /* Unificado */
                     printf(" -> %s", locais_todos[i].nome);
                 }
            }
        }
    }
}


/*
 * =======================
 * FUNCAO MAIN
 * =======================
 */
int main(int argc, char *argv[]) {
    /* Declaracoes no inicio (ANSI C) */
    Vert *G;
    int ordemG = 50;
    
    /* Variaveis */
    Localidades minha_casa = {"Minha Casa", 22, 34, 15, 150};
    
    Localidades locais[] = {
        {"Ecully Charbon", 1, 2, 30, 90},
        {"Tapecaria Renova", 2, 3, 80, 30},
        {"Gelato Borelli", 5, 6, 100, 15},
        {"We Vets Veterinario", 4, 13, 20, 150},
        {"1900 Pizzeria", 10, 11, 70, 40},
        {"Bacio di Latte", 15, 16, 115, 40},
        {"Petiskin do Bob", 9, 18, 165, 20},
        {"Academia CPN", 10, 19, 75, 90},
        {"Degas Pompeia", 13, 22, 15, 150},
        {"Cabelereiro Edmilson Araujo", 16, 25, 115, 50},
        {"Pizzaria Nogueira", 20, 21, 15, 100},
        {"Colegio Sagrado Coracao de Jesus", 25, 37, 75, 90},
        {"Santiago Padaria", 31, 32, 100, 20},
        {"Cazeco Bar", 32, 33, 25, 90},
        {"Galpao da Pizza", 31, 41, 125, 40},
        {"Hospital Sao Camilo", 34, 44, 100, 65},
        {"St. Marche Perdizes", 35, 45, 95, 70},
        {"Minuto Pao de Acucar", 36, 46, 75, 90},
        {"Bar do Gomes", 41, 42, 20, 90},
        {"Academia Smart Fit", 47, 48, 70, 80}
    };
    
    int num_locais;
    int escolha;
    char buffer_lixo;

    /* Inicio do Codigo */
    criaGrafo(&G, ordemG);
    
    num_locais = sizeof(locais) / sizeof(locais[0]);

    /* Adicionando as arestas do mapa com distancias em METROS */
    acrescentaAresta(G, ordemG, 1, 2, 120);     
    acrescentaAresta(G, ordemG, 2, 3, 110);     
    acrescentaAresta(G, ordemG, 5, 6, 115);     
    acrescentaAresta(G, ordemG, 4, 13, 170);    
    acrescentaAresta(G, ordemG, 10, 11, 110);   
    acrescentaAresta(G, ordemG, 15, 16, 155);   
    acrescentaAresta(G, ordemG, 9, 18, 185);    
    acrescentaAresta(G, ordemG, 10, 19, 165);   
    acrescentaAresta(G, ordemG, 13, 22, 165);   
    acrescentaAresta(G, ordemG, 16, 25, 165);   
    acrescentaAresta(G, ordemG, 20, 21, 115);   
    acrescentaAresta(G, ordemG, 25, 37, 165);   
    acrescentaAresta(G, ordemG, 31, 32, 120);   
    acrescentaAresta(G, ordemG, 32, 33, 135);   
    acrescentaAresta(G, ordemG, 31, 41, 165);   
    acrescentaAresta(G, ordemG, 34, 44, 165);   
    acrescentaAresta(G, ordemG, 35, 45, 165);   
    acrescentaAresta(G, ordemG, 36, 46, 165);   
    acrescentaAresta(G, ordemG, 41, 42, 110);   
    acrescentaAresta(G, ordemG, 47, 48, 150);   
    acrescentaAresta(G, ordemG, 22, 34, 150);   
    
    /* Horizontais */
    acrescentaAresta(G, ordemG, 0, 1, 120); 
    acrescentaAresta(G, ordemG, 3, 4, 120); 
    acrescentaAresta(G, ordemG, 4, 5, 120); 
    acrescentaAresta(G, ordemG, 6, 7, 120); 
    acrescentaAresta(G, ordemG, 7, 8, 120); 
    
    acrescentaAresta(G, ordemG, 9, 10, 120); 
    acrescentaAresta(G, ordemG, 11, 12, 120); 
    acrescentaAresta(G, ordemG, 12, 13, 120); 
    acrescentaAresta(G, ordemG, 13, 14, 120); 
    acrescentaAresta(G, ordemG, 14, 15, 120); 
    acrescentaAresta(G, ordemG, 16, 17, 120); 
    
    acrescentaAresta(G, ordemG, 18, 19, 120); 
    acrescentaAresta(G, ordemG, 19, 20, 120); 
    acrescentaAresta(G, ordemG, 21, 22, 120); 
    acrescentaAresta(G, ordemG, 22, 23, 120); 
    acrescentaAresta(G, ordemG, 23, 24, 120); 
    acrescentaAresta(G, ordemG, 24, 25, 120); 
    acrescentaAresta(G, ordemG, 25, 26, 60); 
    acrescentaAresta(G, ordemG, 26, 27, 60); 
    
    acrescentaAresta(G, ordemG, 28, 29, 120); 
    
    acrescentaAresta(G, ordemG, 30, 31, 120); 
    acrescentaAresta(G, ordemG, 33, 34, 120); 
    acrescentaAresta(G, ordemG, 34, 35, 120); 
    acrescentaAresta(G, ordemG, 35, 36, 120); 
    acrescentaAresta(G, ordemG, 36, 37, 120); 
    acrescentaAresta(G, ordemG, 37, 38, 60); 
    acrescentaAresta(G, ordemG, 38, 39, 60); 
    
    acrescentaAresta(G, ordemG, 40, 41, 120); 
    acrescentaAresta(G, ordemG, 42, 43, 120); 
    acrescentaAresta(G, ordemG, 43, 44, 120); 
    acrescentaAresta(G, ordemG, 44, 45, 120); 
    acrescentaAresta(G, ordemG, 45, 46, 120); 
    acrescentaAresta(G, ordemG, 46, 47, 120); 
    
    /* Verticais */
    acrescentaAresta(G, ordemG, 0, 9, 165); 
    acrescentaAresta(G, ordemG, 18, 28, 80); 
    acrescentaAresta(G, ordemG, 28, 30, 80); 
    acrescentaAresta(G, ordemG, 30, 40, 165); 
    
    acrescentaAresta(G, ordemG, 1, 10, 165); 
    acrescentaAresta(G, ordemG, 19, 29, 80); 
    acrescentaAresta(G, ordemG, 29, 31, 80); 
    
    acrescentaAresta(G, ordemG, 2, 11, 165); 
    acrescentaAresta(G, ordemG, 11, 20, 165); 
    acrescentaAresta(G, ordemG, 20, 32, 165); 
    acrescentaAresta(G, ordemG, 32, 42, 165); 
    
    acrescentaAresta(G, ordemG, 3, 12, 165); 
    acrescentaAresta(G, ordemG, 12, 21, 165); 
    acrescentaAresta(G, ordemG, 21, 33, 165); 
    acrescentaAresta(G, ordemG, 33, 43, 165); 
    
    acrescentaAresta(G, ordemG, 5, 14, 165); 
    acrescentaAresta(G, ordemG, 14, 23, 165); 
    acrescentaAresta(G, ordemG, 23, 35, 165); 
    
    acrescentaAresta(G, ordemG, 6, 15, 165); 
    acrescentaAresta(G, ordemG, 15, 24, 165); 
    acrescentaAresta(G, ordemG, 24, 36, 165); 
    
    acrescentaAresta(G, ordemG, 7, 16, 165); 
    
    acrescentaAresta(G, ordemG, 26, 38, 165); 
    
    acrescentaAresta(G, ordemG, 8, 17, 165); 
    acrescentaAresta(G, ordemG, 17, 27, 165); 
    acrescentaAresta(G, ordemG, 27, 39, 165); 
    acrescentaAresta(G, ordemG, 39, 48, 165); 
    
    
    /* Loop principal do menu */
    while (1) {
        
        printf("\n      ++++++++++++++++++++++++++++++++++++++++\n");
        printf("      |                  MENU                |\n");
        printf("      ++++++++++++++++++++++++++++++++++++++++\n\n");
        
        printf("(1) Localizacao de minha casa\n");
        printf("(2) Calcular passeio (PARTE 2)\n");
        printf("(3) Imprimir Grafo Completo\n"); 
        printf("(0) Fechar programa\n\n");
        
        printf("Digite uma opcao: ");
        scanf("%d", &escolha);
        
        if (escolha == 1){
            printf("\nMinha casa esta entre os vertices V%d e V%d\n", minha_casa.vert1, minha_casa.vert2);
            printf("A %d metros de V%d e %d metros de V%d.\n", minha_casa.dist_vert1, minha_casa.vert1, minha_casa.dist_vert2, minha_casa.vert2);
            printf("\n--------------------------------------------------------\n\n");
        }
        
        else if(escolha == 2){
            /* Chama a funcao principal da Parte 2 */
            calcularPasseio(G, ordemG, minha_casa, locais, num_locais);
        }

        else if (escolha == 3) { 
            printf("\n--- Estrutura do Grafo (Lista de Adjacencia) ---\n");
            imprimeGrafo(G, ordemG);
            printf("------------------------------------------------\n");
        }
        
        else if (escolha == 0){
            printf("\nObrigado por utilizar o programa, fechando programa...\n");
            break;
        }
        
        else {
            printf("\nEscolha invalida, por favor digite novamente\n\n");
        }
        
        /* Limpa o buffer de entrada em caso de digitacao invalida e remove o ENTER do scanf anterior */
        while ((buffer_lixo = getchar()) != '\n' && buffer_lixo != EOF); 
    }
    
    /* Libera toda memoria do grafo antes de encerrar */
    destroiGrafo(&G, ordemG);

    return(0);
}