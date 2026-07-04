#include "env.h"

#ifndef AGENT_H
#define AGENT_H

typedef struct {
    place* onde;
    bool comOuro;
    bool temFlecha;
    int score;
    int h, w;
    /*
        Base de conhecimento do agente: o que ele já visitou e o que
        sentiu em cada local visitado. Não é a mesma coisa que a matriz
        "verdade" do enviroment (buraco/monstro reais) -- o agente só
        pode consultar isto, nunca o grid real, para decidir seus passos.
    */
    bool** visited;
    bool** cheiro;
    bool** vento;
    bool** buracoConhecido;
} agent;

void printSimulation(agent A, enviroment E);
bool move(agent* A, enviroment E, place* target);
agent newAgent(enviroment E);
void delAgent(agent* A);
void sense(agent* A);

/*
    Infere, a partir do conhecimento já coletado pelo agente, se a
    posição (i,j) é garantidamente segura (sem buraco e sem monstro).
*/
bool isSafe(agent A, int i, int j);

/*
    Executa um episódio completo de forma autônoma (sem entrada do
    usuário), usando inferência lógica para escolher os movimentos.
    Retorna o score final e, via 'sucesso', se o agente conseguiu
    escapar com o ouro dentro do limite de movimentos.
*/
int runEpisode(enviroment E, int maxMoves, bool verbose, bool* sucesso);

#endif
