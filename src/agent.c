//  ================================
//  Felipe Rovigatti Delfino
//  Lucas de Souza Silva
//  Mateus Carrinho Joaquim
//  ================================

#include <stdio.h>
#include <limits.h>
#include <math.h>

#include "agent.h"


//  FUNÇÕES AUXILIARES

static int manhattan(const int l1, const int c1, const int l2, const int c2) {
    return abs(l1 - l2) + abs(c1 - c2);
}

static int indice(const int largura, const int l, const int c) {
    return (l * largura) + c;
}


//  FUNÇÕES DE INFERÊNCIA

static void inferir_local(Agente* ag) {
    int atual = indice(ag->largura, ag->linha, ag->coluna);

    int viz[4][2] = {
        {ag->linha - 1, ag->coluna},
        {ag->linha + 1, ag->coluna},
        {ag->linha, ag->coluna - 1},
        {ag->linha, ag->coluna + 1},
    };

    for (int i = 0; i < 4; i++) {
        int l = viz[i][0];
        int c = viz[i][1];

        if (!lugar_valido(ag->altura, ag->largura, l, c))
            continue;

        int idx = indice(ag->largura, l, c);

        if (ag->vento[atual] == AUSENTE)
            ag->buraco[idx] = AUSENTE;
        else if (ag->vento[atual] == PRESENTE && ag->buraco[idx] == DESCONHECIDO)
            ag->buraco[idx] = SUSPEITO;
        
        if (ag->cheiro[atual] == AUSENTE && ag->monstro[idx] != MORTO)
            ag->monstro[idx] = AUSENTE;
        else if (ag->cheiro[atual] == PRESENTE && ag->monstro[idx] == DESCONHECIDO)
            ag->monstro[idx] = SUSPEITO;
    }
}

static bool inferir_exclusao(Agente* ag) {
    bool mudou = false;

    for (int l = 0; l < ag->altura; l++) {
        for (int c = 0; c < ag->largura; c++) {
            int idx = indice(ag->largura, l, c);

            if (!ag->visitado[idx])
                continue;
            
            int viz[4][2] = {
                {l - 1, c},
                {l + 1, c},
                {l, c - 1},
                {l, c + 1},
            };

            if (ag->vento[idx] == PRESENTE) {
                int duvidas = 0;
                int suspeito = -1;
                bool explicado = false;

                for (int i = 0; i < 4; i++) {
                    int lViz = viz[i][0];
                    int cViz = viz[i][1];

                    if (lugar_valido(ag->altura, ag->largura, lViz, cViz)) {
                        int idxViz = indice(ag->largura, lViz, cViz);

                        if (ag->buraco[idxViz] == PRESENTE) {
                            explicado = true;
                            break;
                        }
                        else if (ag->buraco[idxViz] == SUSPEITO || ag->buraco[idxViz] == DESCONHECIDO) {
                            duvidas++;
                            suspeito = idxViz;
                        }
                    }
                }

                if (!explicado && duvidas == 1 && suspeito != -1) {
                    ag->buraco[suspeito] = PRESENTE;
                    ag->buracosEncontrados++;
                    mudou = true;
                }
            }

            if (ag->cheiro[idx] == PRESENTE) {
                int duvidas = 0;
                int suspeito = -1;
                bool explicado = false;

                for (int i = 0; i < 4; i++) {
                    int lViz = viz[i][0];
                    int cViz = viz[i][1];

                    if (lugar_valido(ag->altura, ag->largura, lViz, cViz)) {
                        int idxViz = indice(ag->largura, lViz, cViz);

                        if (ag->monstro[idxViz] == PRESENTE || ag->monstro[idxViz] == MORTO) {
                            explicado = true;
                            break;
                        }
                        else if (ag->monstro[idxViz] == SUSPEITO || ag->monstro[idxViz] == DESCONHECIDO) {
                            duvidas++;
                            suspeito = idxViz;
                        }
                    }
                }

                if (!explicado && duvidas == 1 && suspeito != -1) {
                    ag->monstro[suspeito] = PRESENTE;
                    mudou = true;
                }
            }
        }
    }
    
    return mudou;
}

static bool inferir_contagem(Agente* ag) {
    int tamanho = ag->altura * ag->largura;
    bool mudou = false;

    if (ag->buracosEncontrados == ag->numBuraco) {
        for (int i = 0; i < tamanho; i++) {
            if (ag->buraco[i] == SUSPEITO || ag->buraco[i] == DESCONHECIDO) {
                ag->buraco[i] = AUSENTE;
                mudou = true;
            }
        }
    }

    ag->monstrosEncontrados = 0;

    for (int l = 0; l < ag->altura; l++) {
        for (int c = 0; c < ag->largura; c++) {
            int idx = indice(ag->largura, l ,c);

            if (ag->monstro[idx] == MORTO || ag->monstro[idx] == PRESENTE)
                ag->monstrosEncontrados++;
        }
    }

    if (ag->monstrosEncontrados == ag->numMonstro) {
        for (int i = 0; i < tamanho; i++) {
            if (ag->monstro[i] == SUSPEITO || ag->monstro[i] == DESCONHECIDO) {
                ag->monstro[i] = AUSENTE;
                mudou = true;
            }
        }
    }

    return mudou;
}


//  FUNÇÕES PRINCIPAIS

Agente cria_agente(Ambiente* am, const int buracos, const int monstros) {
    Agente ag = {0};

    ag.posicao = buscar_lugar(am, 0, 0);
    ag.linha = 0;
    ag.coluna = 0;
    ag.altura = am->altura;
    ag.largura = am->largura;

    ag.ouro = false;
    ag.flecha = true;
    ag.vivo = true;

    ag.score = 0;

    ag.numBuraco = buracos;
    ag.numMonstro = monstros;
    ag.buracosEncontrados = 0;
    ag.monstrosEncontrados = 0;

    int tamanho = ag.altura * ag.largura;

    ag.visitado = calloc(tamanho, sizeof(bool));
    ag.vento = calloc(tamanho, sizeof(Status));
    ag.cheiro = calloc(tamanho, sizeof(Status));
    ag.buraco = calloc(tamanho, sizeof(Status));
    ag.monstro = calloc(tamanho, sizeof(Status));

    if (
        !ag.visitado || !ag.vento || !ag.cheiro ||
        !ag.buraco || !ag.monstro
    ) {
        printf("ERRO: nao alocou memoria!\n");

        remove_agente(&ag);

        Agente erro = {0};
        return erro;
    }

    int idxSaida = indice(ag.largura, ag.altura - 1, ag.largura - 1);
    ag.buraco[idxSaida] = AUSENTE;
    ag.monstro[idxSaida] = AUSENTE;

    return ag;
}

bool decidir_movimentar(Agente* ag, Ambiente* am, const bool verbose) {
    int tamanho = ag->altura * ag->largura;
    int* caminho = malloc(tamanho * sizeof(int));

    //  CASO 1: Tem o ouro e sabe sair.
    if (ag->ouro) {
        if (verbose) printf("Agente pegou o ouro! Planejando caminho seguro para a saída.\n");

        int passos = BFS(ag, ag->linha, ag->coluna, ag->altura - 1, ag->largura - 1, caminho);

        if (passos > 0) {
            for (int i = 0; i < passos; i++) {
                executar_passo(ag, am, caminho[i], false);
            }

            free(caminho);
            return true;
        } else
            if (verbose) printf("Saida bloqueada!\n");
        
    }

    //  CASO 2: explora casas seguras e não visitadas.
    int lMelhor = -1;
    int cMelhor = -1;
    int distMenor = INT_MAX;
    int* caminhoTeste2 = malloc(tamanho * sizeof(int));

    for (int l = 0; l < ag->altura; l++) {
        for (int c = 0; c < ag->largura; c++) {
            int idx = indice(ag->largura, l, c);

            if (!ag->visitado[idx] && ag->buraco[idx] == AUSENTE && (ag->monstro[idx] == AUSENTE || ag->monstro[idx] == MORTO)) {
                int dist = manhattan(ag->linha, ag->coluna, l, c);

                if (dist >= distMenor)
                    continue;

                int passosTeste = BFS(ag, ag->linha, ag->coluna, l, c, caminhoTeste2);
                if (passosTeste > 0) {
                    distMenor = dist;
                    lMelhor = l;
                    cMelhor = c;
                }
            }
        }
    }

    free(caminhoTeste2);

    if (lMelhor != -1) {
        int passos = BFS(ag, ag->linha, ag->coluna, lMelhor, cMelhor, caminho);

        if (passos > 0) {
            if (verbose) printf("Movendo ate a casa segura inexplorada em (%d, %d)!\n",
                lMelhor, cMelhor);

            for (int i = 0; i < passos; i++)
                executar_passo(ag, am, caminho[i], false);
            
            free(caminho);
            return true;
        }
    }

    //  CASO 3: escolhe a posição menos arriscada.
    if (verbose) printf("Escolhendo fronteira menos arriscada para explorar!\n");

    int lChute = -1;
    int cChute = -1;
    int menorRisco = INT_MAX;
    int passosMenor = INT_MAX;
    int* caminhoTeste = malloc(tamanho * sizeof(int));

    int locaisVisitados = 0;
    for (int i = 0; i < tamanho; i++)
        if (ag->visitado[i]) locaisVisitados++;
    
    int buracosRestantes = ag->numBuraco - ag->buracosEncontrados;
    int monstrosRestantes = ag->numMonstro - ag->monstrosEncontrados;

    for (int l = 0; l < ag->altura; l++) {
        for (int c = 0; c < ag->largura; c++) {
            int idx = indice(ag->largura, l, c);

            if (ag->visitado[idx])
                continue;
             
            bool fronteira = false;
            int vizVisitados = 0;

            int viz[4][2] = {
                {l - 1, c},
                {l + 1, c},
                {l, c - 1},
                {l, c + 1}
            };

            for (int i = 0; i < 4; i++) {
                if (lugar_valido(ag->altura, ag->largura, viz[i][0], viz[i][1])) {
                    if (ag->visitado[indice(ag->largura, viz[i][0], viz[i][1])]) {
                        fronteira = true;
                        vizVisitados++;
                    }
                }
            }

            if (!fronteira)
                continue;
            
            int risco = 0;

            if (ag->buraco[idx] == PRESENTE) continue;
            if (ag->monstro[idx] == PRESENTE && !ag->flecha) continue;

            if (ag->buraco[idx] == SUSPEITO) {
                if (buracosRestantes > 0) {
                    risco += ((20 * buracosRestantes) / ag->numBuraco) + (vizVisitados * 20) + 1;
                }
            }

            if (ag->monstro[idx] == SUSPEITO) {
                if (monstrosRestantes > 0) {
                    if (ag->flecha)
                        risco += ((2 * monstrosRestantes) / ag->numMonstro) - (vizVisitados * 20) + 1;
                    else
                        risco += ((20 * monstrosRestantes) / ag->numMonstro) + (vizVisitados * 20) + 1;
                }
            }
            if (ag->monstro[idx] == PRESENTE && ag->flecha)
                risco -= 10000;

            int passosTeste = BFS(ag, ag->linha, ag->coluna, l, c, caminhoTeste);
            if (passosTeste > 0) {
                if (risco < menorRisco) {
                    passosMenor = passosTeste;
                    menorRisco = risco;
                    lChute = l;
                    cChute = c;
                } else if (risco == menorRisco && passosTeste < passosMenor) {
                    passosMenor = passosTeste;
                    lChute = l;
                    cChute = c;
                }
            }
        }
    }

    if (lChute != -1) {
        int passos = BFS(ag, ag->linha, ag->coluna, lChute, cChute, caminho);

        if (passos >= 0) {
            int idxProx = caminho[0];
            int lProx = idxProx / ag->largura;
            int cProx = idxProx % ag->largura;

            if ((ag->monstro[idxProx] == SUSPEITO || ag->monstro[idxProx] == PRESENTE) && ag->flecha) {
                if (verbose) printf("Arriscando um disparo!\n");
                bool acertou = disparar(ag, am, lProx, cProx, verbose);

                if (!acertou && ag->buraco[idxProx] != AUSENTE) {
                    free(caminho);
                    free(caminhoTeste);
                    return true;
                }
            }

            executar_passo(ag, am, idxProx, false);

            free(caminho);
            free(caminhoTeste);
            return true;
        }
    }

    free(caminho);
    free(caminhoTeste);
    return false;
}

bool disparar(Agente* ag, Ambiente* am, const int lAlvo, const int cAlvo, const bool verbose) {
    if (!ag->flecha)
        return false;
    
    ag->flecha = false;
    ag->score -= 10;

    int direcao = -1;
    if (lAlvo < ag->linha) direcao = 0;
    else if (lAlvo > ag->linha) direcao = 1;
    else if (cAlvo < ag->coluna) direcao = 2;
    else if (cAlvo > ag->coluna) direcao = 3;

    if (verbose) printf("O agente dispara...\n");

    bool grito = processa_disparo(am, ag->linha, ag->coluna, direcao);
    if (grito) {
        if (verbose) printf("FAAAAAH! Um grito ecoou pelo ambiente: o monstro morreu!\n");

        int idxAlvo = indice(ag->largura, lAlvo, cAlvo);
        ag->monstro[idxAlvo] = MORTO;
        ag->buraco[idxAlvo] = AUSENTE;
        ag->score += 20;

        return true;
    } else {
        if (verbose) printf("e erra...\n");

        int idxAlvo = indice(ag->largura, lAlvo, cAlvo);
        ag->monstro[idxAlvo] = AUSENTE;
    }

    return false;
}

int BFS(Agente* ag, const int l1, const int c1, const int l2, const int c2, int* caminho) {
    int tamanho = ag->altura * ag->largura;

    if (l1 == l2 && c1 == c2)
        return 0;
    
    int* fila = malloc(tamanho * sizeof(int));
    int* pais = malloc(tamanho * sizeof(int));
    bool* visitado = calloc(tamanho, sizeof(bool));

    for (int i = 0; i < tamanho; i++)
        pais[i] = -1;

    int idxA = indice(ag->largura, l1, c1);
    int idxB = indice(ag->largura, l2, c2);

    int head = 0;
    int tail = 0;

    fila[tail++] = idxA;
    visitado[idxA] = true;

    bool achou = false;

    int deltaL[4] = {-1, 1, 0, 0};
    int deltaC[4] = {0, 0, -1, 1};

    while (head < tail) {
        int idx = fila[head++];

        if (idx == idxB) {
            achou = true;
            break;
        }

        int l = idx / ag->largura;
        int c = idx % ag->largura;

        for (int i = 0; i < 4; i++) {
            int lViz = l + deltaL[i];
            int cViz = c + deltaC[i];

            if (lugar_valido(ag->altura, ag->largura, lViz, cViz)) {
                int idxViz = indice(ag->largura, lViz, cViz);

                if (!visitado[idxViz]) {
                    bool caminhoSeguro = ag->visitado[idxViz] ||
                                         ag->buraco[idxViz] == AUSENTE &&
                                         (ag->monstro[idxViz] == AUSENTE ||
                                         ag->monstro[idxViz] == MORTO);
                    bool alvoSeguro = idxViz == idxB;

                    if (caminhoSeguro || alvoSeguro) {
                        visitado[idxViz] = true;
                        pais[idxViz] = idx;
                        fila[tail++] = idxViz;
                    }
                }
            }
        }
    }

    int passos = -1;

    if (achou) {
        int* ahlip = malloc(tamanho * sizeof(int));
        int contador = 0;
        int atual = idxB;

        while (atual != idxA) {
            ahlip[contador++] = atual;
            atual = pais[atual];
        }

        for (int i = 0; i < contador; i++)
            caminho[i] = ahlip[contador - i - 1];
        
        passos = contador;
        free(ahlip);
    }

    free(fila);
    free(pais);
    free(visitado);
    
    return passos;
}

void executar_passo(Agente* ag, const Ambiente* am, const int proximo, const bool verbose) {
    if (!ag->vivo) return;

    int l = proximo / ag->largura;
    int c = proximo % ag->largura;

    ag->linha = l;
    ag->coluna = c;
    ag->posicao = buscar_lugar(am, l, c);
    ag->score--;

    if (ag->posicao->ouro && !ag->ouro) {
        ag->ouro = true;
        ag->posicao->ouro = false;
        ag->score += 200;
        if (verbose) printf("Pegou o ouro em (%d, %d)!\n", l, c);
    }

    if (ag->posicao->buraco) {
        ag->vivo = false;
        ag->score -= 1000;
        if (verbose) printf("FIM DE JOGO: o agente caiu no buraco em (%d, %d)...\n", l, c);
        return;
    }

    if (ag->posicao->monstro) {
        ag->vivo = false;
        ag->score -= 1000;
        if (verbose) printf("FIM DE JOGO: o agente encontro o monstro em (%d, %d)...\n", l, c);
        return;
    }

    observar(ag);
    inferir(ag);
}

void inferir(Agente* ag) {
    inferir_local(ag);

    bool mudou;
    do {
        mudou = false;

        if (inferir_exclusao(ag))
            mudou = true;

        if (inferir_contagem(ag))
            mudou = true;
    } while (mudou);
    
}

void observar(Agente* ag) {
    int idx = indice(ag->largura, ag->linha, ag->coluna);

    ag->visitado[idx] = true;

    //  Se o agente está observando, ele está vivo.
    ag->buraco[idx] = AUSENTE;
    if (ag->monstro[idx] != MORTO)
        ag->monstro[idx] = AUSENTE;

    ag->vento[idx] = (ag->posicao->vento == 1) ? PRESENTE : AUSENTE;
    ag->cheiro[idx] = (ag->posicao->cheiro == 1) ? PRESENTE : AUSENTE;
}

void remove_agente(Agente* ag) {
    if (!ag)
        return;

    free(ag->visitado);
    free(ag->vento);
    free(ag->cheiro);
    free(ag->buraco);
    free(ag->monstro);

    *ag = (Agente){0};
}