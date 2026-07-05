//  Felipe Rovigatti Delfino
//  Lucas de Souza Silva
//  Mateus Carrinho Joaquim

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include "agent.h"
#include "env.h"

#define NUM_EXECUCOES 15

typedef struct {
    int h, w;
    int numBuraco, numMonstro;
} configuracaoSala;

static void demonstracao(void){
    printf("=== Demonstracao de uma execucao autonoma (sala 5x5) ===\n\n");
    enviroment E = newEnviroment(5,5);
    initEnviroment(E,3,1);

    bool sucesso;
    int score = runEpisode(E, 10*5*5, true, &sucesso);

    printf("\nResultado: %s. Score final: %d\n\n",
        sucesso ? "escapou com o ouro" : "nao conseguiu escapar a tempo", score);

    delEnviroment(&E);
}

int main(){
    // Semente única para todo o programa: initEnviroment não deve mais
    // chamar srand() sozinha, senão execuções consecutivas na mesma
    // fração de segundo gerariam salas idênticas.
    srand((unsigned int)time(NULL));

    demonstracao();

    configuracaoSala configs[] = {
        {4, 4, 1, 1},
        {5, 5, 1, 1},
        {6, 6, 3, 1},
        {7, 7, 5, 2},
        {8, 8, 7, 3},
    };
    int numConfigs = sizeof(configs)/sizeof(configs[0]);
    int c, r;

    FILE* csv = fopen("resultados.csv", "w");
    if (csv) fprintf(csv, "altura,largura,buracos,monstros,execucao,score,sucesso\n");

    printf("=== Avaliacao do agente em diferentes tamanhos de sala ===\n\n");
    printf("%-8s %-9s %-10s %-11s %-10s %-10s %-10s\n",
        "Sala", "Buracos", "Monstros", "Execucoes", "Media", "DesvPad", "Sucesso%");

    for (c=0; c<numConfigs; c++){
        int h = configs[c].h, w = configs[c].w;
        int nb = configs[c].numBuraco, nm = configs[c].numMonstro;
        int maxMoves = 10*h*w;

        int scores[NUM_EXECUCOES];
        int sucessos = 0;

        for (r=0; r<NUM_EXECUCOES; r++){
            enviroment E = newEnviroment(h,w);
            initEnviroment(E, nb, nm);

            bool sucesso;
            int score = runEpisode(E, maxMoves, false, &sucesso);
            scores[r] = score;
            if (sucesso) sucessos++;

            if (csv){
                fprintf(csv, "%d,%d,%d,%d,%d,%d,%d\n",
                    h, w, nb, nm, r, score, sucesso ? 1 : 0);
            }

            delEnviroment(&E);
        }

        double soma = 0;
        for (r=0; r<NUM_EXECUCOES; r++) soma += scores[r];
        double media = soma/NUM_EXECUCOES;

        double somaSqDiff = 0;
        for (r=0; r<NUM_EXECUCOES; r++){
            double diff = scores[r]-media;
            somaSqDiff += diff*diff;
        }
        double desvio = sqrt(somaSqDiff/(NUM_EXECUCOES-1));

        char salaStr[16];
        snprintf(salaStr, sizeof(salaStr), "%dx%d", h, w);

        printf("%-8s %-9d %-10d %-11d %-10.2f %-10.2f %-10.1f\n",
            salaStr, nb, nm, NUM_EXECUCOES, media, desvio,
            100.0*sucessos/NUM_EXECUCOES);
    }

    if (csv){
        fclose(csv);
        printf("\nResultados detalhados de cada execucao salvos em resultados.csv\n");
    }

    return 0;
}
