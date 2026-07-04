#include "env.h"

#ifndef AGENT_H
#define AGENT_H

typedef struct {
    place* onde;
    bool comOuro;
    bool temFlecha;
    int score;
    int h, w;
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

bool isSafe(agent A, int i, int j);

int runEpisode(enviroment E, int maxMoves, bool verbose, bool* sucesso);

#endif
