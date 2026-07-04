
#include <stdbool.h>
#include <stdlib.h>

#ifndef ENV_H
#define ENV_H

typedef struct {
    bool cheiro;
    bool vento;
} sensation;

//Propriedades de cada lugar do ambiente
typedef struct{
    int row, col; //indice do lugar em um grid
    bool buraco; //se tem buraco
    bool monstro; //se tem monstro
    bool ouro; //se tem ouro
    sensation S; //Sensações que podem ser percebidas pelo agente
} place;

//O ambiente é uma matriz de lugares (grid)
typedef struct{
    int h, w;
    place** grid;
} enviroment;

enviroment newEnviroment(int h, int w);
void delEnviroment(enviroment* E);
void initEnviroment(enviroment E, int numBuraco, int numMonstro);
bool isNeighbor(place p, place q);
place* getPlace(enviroment E, int i, int j);
void initSensations(enviroment E);

#endif
