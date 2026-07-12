//  ================================
//  Felipe Rovigatti Delfino
//  Lucas de Souza Silva
//  Mateus Carrinho Joaquim
//  ================================

#include "env.h"

#ifndef AGENT_H
#define AGENT_H


typedef enum {
    DESCONHECIDO,
    SUSPEITO,
    AUSENTE,
    PRESENTE,
    MORTO
} Status;

typedef struct {
    Lugar* posicao;
    int linha;
    int coluna;
    int altura;
    int largura;
    
    bool ouro;
    bool flecha;
    bool vivo;

    int score;

    bool* visitado;
    Status* vento;
    Status* cheiro;
    Status* buraco;
    Status* monstro;

    int numBuraco;
    int numMonstro;
    int buracosEncontrados;
    int monstrosEncontrados;
} Agente;


Agente cria_agente(Ambiente* am, const int buracos, const int monstros);
bool decidir_movimentar(Agente* ag, Ambiente* am, const bool verbose);
bool disparar(Agente* ag, Ambiente* am, const int lAlvo, const int cAlvo, const bool verbose);
int BFS(Agente* ag, const int l1, const int c1, const int l2, const int c2, int* caminho);
void executar_passo(Agente* ag, const Ambiente* am, const int proximo, const bool verbose);
void inferir(Agente* ag);
void observar(Agente* ag);
void remove_agente(Agente* ag);


#endif