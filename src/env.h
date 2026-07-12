//  ================================
//  Felipe Rovigatti Delfino
//  Lucas de Souza Silva
//  Mateus Carrinho Joaquim
//  ================================

#include <stdbool.h>
#include <stdlib.h>

#ifndef ENV_H
#define ENV_H


typedef struct {
    bool buraco;
    bool monstro;
    bool ouro;

    bool vento;
    bool cheiro;
} Lugar;

typedef struct {
    int altura;
    int largura;
    Lugar* tabuleiro;
} Ambiente;


Ambiente cria_ambiente(const int altura, const int largura);
bool lugar_valido(const int altura, const int largura, const int l, const int c);
bool processa_disparo(Ambiente* am, const int lDisparo, const int cDisparo, const int direcao);
bool solucionavel(Ambiente* am, const int altura, const int largura, const int lOuro, const int cOuro);
bool verifica_vizinho(const int l1, const int c1, const int l2, const int c2);
Lugar* buscar_lugar(const Ambiente* am, const int l, const int c);
void inicializa_ambiente(Ambiente* am, int buracos, int monstros);
void inicializa_sensacoes(Ambiente* am);
void remove_ambiente(Ambiente* am);

void imprime_ambiente(const Ambiente* am, const int num);
void imprime_sensacoes(const Ambiente* am, const int num);


#endif