#  ================================
#  Felipe Rovigatti Delfino
#  Lucas de Souza Silva
#  Mateus Carrinho Joaquim
#  ================================

CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lm \
		  -Isrc
		  
# Arquivos fonte
SRC = main.c src/env.c src/agent.c
	
# Executável
TARGET = dungeon.exe

all:
	$(CC) $(SRC) -o $(TARGET) $(LDFLAGS)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)