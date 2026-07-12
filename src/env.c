//  ================================
//  Felipe Rovigatti Delfino
//  Lucas de Souza Silva
//  Mateus Carrinho Joaquim
//  ================================

#include <stdio.h>

#include "env.h"

static const int deltaL[4] = {-1, 1, 0, 0};
static const int deltaC[4] = {0, 0, -1, 1};


//  FUNÇÕES AUXILIARES

bool lugar_valido(const int altura, const int largura, const int l, const int c) {
    return (
        l >= 0 &&
        l < altura &&
        c >= 0 &&
        c < largura
    );
}

bool verifica_vizinho(const int l1, const int c1, const int l2, const int c2) {
    int lDif = abs(l1 - l2);
    int cDif = abs(c1 - c2);

    return (lDif + cDif == 1);
}

static int BFSFlechas(
    Ambiente* am, const int altura, const int largura, const int l1, 
    const int c1, const int l2, const int c2, int flechas
) {
    int tamanho = altura * largura;

    int* maxFlechas = malloc(tamanho * sizeof(int));
    for (int i = 0; i < tamanho; i++)
        maxFlechas[i] = -1;
    
    int capacidade = tamanho * (flechas + 2);
    int* lFila = malloc(capacidade * sizeof(int));
    int* cFila = malloc(capacidade * sizeof(int));
    int* fFila = malloc(capacidade * sizeof(int));
    int inicio = 0;
    int fim = 0;

    lFila[fim] = l1;
    cFila[fim] = c1;
    fFila[fim] = flechas;
    fim++;

    maxFlechas[l1 * largura + c1] = flechas;

    int viz[4][2] = {
        {-1, 0},
        {1, 0},
        {0, -1},
        {0, 1},
    };

    int melhorChegada = -1;

    while (inicio < fim) {
        int lAtual = lFila[inicio];
        int cAtual = cFila[inicio];
        int fAtual = fFila[inicio];
        inicio++;

        if (lAtual == l2 && cAtual == c2) {
            if (fAtual > melhorChegada)
                melhorChegada = fAtual;

            continue;
        }

        for (int i = 0; i < 4; i++) {
            int lProx = lAtual + viz[i][0];
            int cProx = cAtual + viz[i][1];

            if (lugar_valido(altura, largura, lProx, cProx)) {
                int idxProx = lProx * largura + cProx;

                bool buraco = (am->tabuleiro[idxProx].buraco == 1);
                bool monstro = (am->tabuleiro[idxProx].monstro == 1);

                if (buraco) continue;

                int fProx = fAtual;
                if (monstro)
                    fProx--;

                if (fProx >= 0 && fProx > maxFlechas[idxProx]) {
                    maxFlechas[idxProx] = fProx;
                    lFila[fim] = lProx;
                    cFila[fim] = cProx;
                    fFila[fim] = fProx;
                    fim++;
                }
            }
        }
    }

    free(maxFlechas);
    free(lFila);
    free(cFila);
    free(fFila);

    return melhorChegada;
}

static bool lugar_vazio(const Lugar* lg) {
    return (!lg->buraco && !lg->monstro && !lg->ouro);
}

static bool lugar_reservado(const Ambiente* am, const int l, const int c) {
    return (
        (l == 0 && c == 0) ||
        (l == (am->altura - 1) && c == (am->largura - 1))
    );
}

static Lugar* lugar_aleatorio_vazio(const Ambiente* am) {
    int tentativas = 0;
    int limite = am->altura * am->largura * 10;

    while (tentativas < limite) {
        int l = rand() % am->altura;
        int c = rand() % am->largura;

        if (lugar_reservado(am, l, c)) {
            tentativas++;
            continue;
        } 

        Lugar* lg = buscar_lugar(am, l, c);

        if (lugar_vazio(lg))
            return lg;

        tentativas++;
    }

    return NULL;
}

static void propaga_sensacao(Lugar* alvo, const Lugar* vizinho) {
    if (vizinho->buraco)
        alvo->vento = true;
    
    if (vizinho->monstro)
        alvo->cheiro = true;
}


//  FUNÇÕES PRINCIPAIS

Ambiente cria_ambiente(const int altura, const int largura) {
    Ambiente am = {0};

    if (altura <= 0 || largura <= 0) {
        printf("ERRO: dimensoes invalidas!\n");
        return am;
    }

    am.altura = altura;
    am.largura = largura;
    am.tabuleiro = calloc(altura * largura, sizeof(Lugar));

    if (!am.tabuleiro) {
        printf("ERRO: nao alocou memoria!\n");

        am.altura = 0;
        am.largura = 0;

        return am;
    } 

    return am;
}

bool processa_disparo(Ambiente* am, const int lDisparo, const int cDisparo, const int direcao) {
    //  DIREÇÕES: 0 = , 1 = , 2 = , 3 = .
    
    int l = lDisparo + deltaL[direcao];
    int c = cDisparo + deltaC[direcao];

    while (lugar_valido(am->altura, am->largura, l, c)) {
        Lugar* lg = buscar_lugar(am, l, c);

        if (lg->monstro) {
            lg->monstro = false;
            inicializa_sensacoes(am);

            return true;
        }

        l += deltaL[direcao];
        c += deltaC[direcao];
    }

    return false;
}

bool solucionavel(Ambiente* am, const int altura, const int largura, const int lOuro, const int cOuro) {
    int flechasRestantesOuro = BFSFlechas(am, altura, largura, 0, 0, lOuro, cOuro, 1);

    if (flechasRestantesOuro == -1)
        return false;

    int lSaida = altura - 1;
    int cSaida = largura - 1;

    int flechasRestantesSaida = BFSFlechas(am, altura, largura, lOuro, cOuro, lSaida, cSaida, flechasRestantesOuro);

    if (flechasRestantesSaida == -1)
        return false;
    
    return true;
}

Lugar* buscar_lugar(const Ambiente* am, const int l, const int c) {
    if (!lugar_valido(am->altura, am->largura, l, c))
        return NULL;
    
    return &am->tabuleiro[(l * am->largura) + c];
}

void inicializa_ambiente(Ambiente* am, int buracos, int monstros) {
    if (am == NULL || am->tabuleiro == NULL)
        return;

    int reservados = (am->altura == 1 && am->largura == 1) ? 1 : 2;
    int validos = am->altura * am->largura - reservados;

    if (buracos + monstros + 1 > validos) {
        printf("ERRO: numero muito alto de objetos!\n");
        return;
    }

    while (buracos > 0) {
        Lugar* lg = lugar_aleatorio_vazio(am);

        if (lg == NULL) {
            printf("ERRO: nao foi possivel posicionar todos os buracos!\n");
            return;
        }

        lg->buraco = true;
        buracos--;
    }

    while (monstros > 0) {
        Lugar* lg = lugar_aleatorio_vazio(am);

        if (lg == NULL) {
            printf("ERRO: nao foi possivel posicionar todos os monstros!\n");
            return;
        }

        lg->monstro = true;
        monstros--;
    }

    Lugar* lg = lugar_aleatorio_vazio(am);

    if (lg == NULL) {
        printf("ERRO: nao foi possivel posicionar o ouro!\n");
        return;
    }

    lg->ouro = true;


    inicializa_sensacoes(am);
}

void inicializa_sensacoes(Ambiente* am) {
    if (am == NULL || am->tabuleiro == NULL)
        return;
    
    for (int l = 0; l < am->altura; l++) {
        for (int c = 0; c < am->largura; c++) {
            Lugar* atual = buscar_lugar(am, l, c);

            atual->vento = false;
            atual->cheiro = false;

            for (int i = 0; i < 4; i++) {
                int lViz = l + deltaL[i];
                int cViz = c + deltaC[i];

                Lugar* viz = buscar_lugar(am, lViz, cViz);

                if (viz)
                    propaga_sensacao(atual, viz);
            }
        }
    }
}

void remove_ambiente(Ambiente* am) {
    if (!am)
        return;

    free(am->tabuleiro);

    am->tabuleiro = NULL;
    am->altura = 0;
    am->largura = 0;
}


//  FUNÇÕES DE IMPRESSÃO

void imprime_ambiente(const Ambiente* am, const int num) {
    if (am == NULL || am->tabuleiro == NULL)
        return;

    printf("\n===== Ambiente %d =====\n\n", num);
    printf("    ");

    for (int c = 0; c < am->largura; c++)
        printf("%2d ", c);
    
    printf("\n");

    for (int l = 0; l < am->altura; l++) {
        printf("%2d  ", l);

        for (int c = 0; c < am->largura; c++) {
            Lugar* lg = buscar_lugar(am, l, c);

            char simbolo = '.';

            if (l == 0 && c == 0)
                simbolo = 'E';
            else if (l == am->altura - 1 && c == am->largura - 1)
                simbolo = 'S';
            else if (lg->ouro)
                simbolo = 'O';
            else if (lg->buraco)
                simbolo = 'B';
            else if (lg->monstro)
                simbolo = 'M';
            
            printf(" %c ", simbolo);
        }

        printf("\n");
    }

    printf("\n");
}

void imprime_sensacoes(const Ambiente* am, const int num) {
    if (am == NULL || am->tabuleiro == NULL)
        return;

    printf("\n===== Sensacoes %d =====\n\n", num);
    printf("    ");

    for (int c = 0; c < am->largura; c++)
        printf("%2d ", c);
    
    printf("\n");

    for (int l = 0; l < am->altura; l++) {
        printf("%2d  ", l);

        for (int c = 0; c < am->largura; c++) {
            Lugar* lg = buscar_lugar(am, l, c);

            char simbolo = '.';

            if (lg->vento && lg->cheiro)
                simbolo = 'A';
            else if (lg->vento)
                simbolo = 'V';
            else if (lg->cheiro)
                simbolo = 'C';
            
            printf(" %c ", simbolo);
        }

        printf("\n");
    }

    printf("\n");
}