//  ================================
//  Felipe Rovigatti Delfino
//  Lucas de Souza Silva
//  Mateus Carrinho Joaquim
//  ================================

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "env.h"
#include "agent.h"

void mapamental(Agente* ag, Ambiente* am) {
    FILE* ff = fopen("caminho.txt", "a");

    fprintf(ff, "\n=== SCORE: %d | POS: (%d,%d) ===\n", 
           ag->score, ag->linha, ag->coluna);
    fprintf(ff, "Status: Flecha: %s | Ouro coletado: %s\n", 
           ag->flecha ? "SIM" : "NAO", ag->ouro ? "SIM" : "NAO");
    
    for (int c = 0; c < am->largura; c++) fprintf(ff,"+---");
    fprintf(ff,"+\n");

    for (int l = 0; l < am->altura; l++) {
        for (int c = 0; c < am->largura; c++) {
            int idx = (am->largura * l) + c;

            fprintf(ff,"|");
            
            if (ag->linha == l && ag->coluna == c) {
                fprintf(ff," A ");
            }
            else if (ag->buraco[idx] == PRESENTE) {
                fprintf(ff,"B! ");
            }
            else if (ag->monstro[idx] == PRESENTE) {
                fprintf(ff,"M! ");
            }
            else if (ag->buraco[idx] == AUSENTE && ag->monstro[idx] == AUSENTE) {
                fprintf(ff," . ");
            }
            else if (ag->buraco[idx] == SUSPEITO && ag->monstro[idx] == SUSPEITO){
                fprintf(ff," 2?");
            }
            else if (ag->buraco[idx] == SUSPEITO) {
                fprintf(ff,"b? ");
            } 
            else if (ag->monstro[idx] == SUSPEITO) {
                fprintf(ff,"m? ");
            }
            else if (ag->monstro[idx] == MORTO) {
                fprintf(ff, "xx ");
            }
            else {
                fprintf(ff," ? ");
            }
        }
        fprintf(ff,"|\n");
        
        for (int c = 0; c < am->largura; c++) fprintf(ff,"+---");
        fprintf(ff,"+\n");
    }
    
    fprintf(ff,"Pistas locais -> Brisa: %s | Cheiro: %s\n",
           am->tabuleiro[ag->linha * am->largura + ag->coluna].vento ? "SIM" : "NAO",
           am->tabuleiro[ag->linha * am->largura + ag->coluna].cheiro ? "SIM" : "NAO");
    fprintf(ff,"=============================================\n");
}

void rodar_experimento(FILE* f, int altura, int largura, int buracos, int monstros, int testes){
    int vitorias = 0;
    int mortesBuraco = 0;
    int mortesMonstro = 0;
    int travado = 0;

    double scoreAcumulado = 0;
    double quadradoScoreAcumulado = 0;

    double* historico = malloc(testes * sizeof(double));
    int maxTurnos = altura * largura * 10;

    for (int i = 0; i < testes; i++) {
        Ambiente am;

        int tentativaMapa = 0;
        int maxMapa = 10000;
        while (tentativaMapa < maxMapa) {
            am = cria_ambiente(altura, largura);
            inicializa_ambiente(&am, buracos, monstros);

            int lOuro = -1;
            int cOuro = -1;
            int tamanho = largura * altura;

            for (int i = 0; i < tamanho; i++) {
                if (am.tabuleiro[i].ouro == 1) {
                    lOuro = i / largura;
                    cOuro = i % largura;
                    break;
                }
            }

            if (solucionavel(&am, altura, largura, lOuro, cOuro))
                break;

            remove_ambiente(&am);
            tentativaMapa++;
        }

        if (tentativaMapa == maxMapa)
            continue;

        Agente ag = cria_agente(&am, buracos, monstros);
        observar(&ag);
        inferir(&ag);
        
        int turnos = 0;
        char resultado[20] = "TRAVOU";

        while (ag.vivo && turnos < maxTurnos) {
            turnos++;

            if (ag.ouro && ag.linha == am.altura - 1 && ag.coluna == am.largura - 1) {
                vitorias++;
                ag.score += 1000;
                strcpy(resultado, "VITORIA");
                break;
            }

            bool agiu = decidir_movimentar(&ag, &am, false);
            if (!agiu) {
                travado++;
                break;
            }
        }

        if (!ag.vivo && turnos <= 2) {
            remove_agente(&ag);
            remove_ambiente(&am);
            i--;
            continue;
        } 

        if (!ag.vivo) {
            if (ag.posicao->buraco) {
                mortesBuraco++;
                strcpy(resultado, "BURACO");
            } else if (ag.posicao->monstro) {
                mortesMonstro++;
                strcpy(resultado, "MONSTRO");
            }
        }

        scoreAcumulado += ag.score;
        historico[i] = (double)ag.score;

        remove_agente(&ag);
        remove_ambiente(&am);
    }

    double txVitoria = (vitorias / (float)testes) * 100;
    double txBuraco = (mortesBuraco/ (float)testes) * 100;
    double txMonstro = (mortesMonstro / (float)testes) * 100;
    double txTravado = (travado / (float)testes) * 100;
    double mediaScore = (scoreAcumulado / (float)testes);
    double somaVariancia = 0;
    for (int i = 0; i < testes; i++) {
        somaVariancia += pow(historico[i] - mediaScore, 2);
    }
    double desvio = sqrt(somaVariancia / (testes - 1));

    fprintf(f, "%d,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
            altura, buracos, monstros, txVitoria, txBuraco, txMonstro, txTravado, mediaScore, desvio);

    free(historico);
}

int main() {
    srand(time(NULL));


    printf("[1] Individual \nou [2] Multiplo?\n");
    
    int opcao;
    scanf(" %d", &opcao);
    switch (opcao) {
        case 1:
            remove("caminho.txt");
            printf("\nCaminho salvo em 'caminho.txt'!\n");

            Ambiente am;

            int tentativaMapa = 0;
            int maxMapa = 10000;
            while (tentativaMapa < maxMapa) {
                am = cria_ambiente(6, 6);
                inicializa_ambiente(&am, 3, 4);

                int lOuro = -1;
                int cOuro = -1;
                int tamanho = 6 * 6;

                for (int i = 0; i < tamanho; i++) {
                    if (am.tabuleiro[i].ouro == 1) {
                        lOuro = i / 6;
                        cOuro = i % 6;
                        break;
                    }
                }

                if (solucionavel(&am, 6, 6, lOuro, cOuro))
                    break;

                remove_ambiente(&am);
                tentativaMapa++;
            }

            if (tentativaMapa == maxMapa)
                return 0;

            imprime_ambiente(&am, 1);

            Agente ag = cria_agente(&am, 3, 4);
            observar(&ag);
            inferir(&ag);
            
            int turnos = 0;
            while (ag.vivo && turnos < 1000) {
                mapamental(&ag, &am);

                if (ag.ouro && ag.linha == am.altura - 1 && ag.coluna == am.largura - 1) {
                    ag.score += 1000;
                    break;
                }

                bool agiu = decidir_movimentar(&ag, &am, false);
                if (!agiu) {
                    return 0;
                }
            }

            remove_agente(&ag);
            remove_ambiente(&am);

            return 0;
        case 2:
             FILE *csv = fopen("results/dungeon.csv", "w");

            fprintf(csv, "dimensao,buracos,monstros,taxa_vitoria,morte_buraco,morte_monstro,travamento,score_medio,desvio_padrao\n");

            int numTestes = 1000;
            int tamanhos[] = {4, 5, 6, 8, 10};
            int qtdTamanhos = 5;

            printf("Iniciando bateria de experimentos...\n");

            for (int t = 0; t < qtdTamanhos; t++) {
                int dim = tamanhos[t];
                int maxPerigos = (((dim * dim) - 2) * 25) / 100; 

                printf("Processando salas %dx%d (Max perigos: %d)...\n", dim, dim, maxPerigos);

                for (int b = 1; b <= maxPerigos; b++) {
                    for (int m = 1; m <= maxPerigos; m++) {
                        rodar_experimento(csv, dim, dim, b, m, numTestes);
                    }
                }
            }

            fclose(csv);
            printf("Bateria concluida! Arquivo 'results/dungeon.csv' gerado com sucesso.\n");
            return 0;
    }
}