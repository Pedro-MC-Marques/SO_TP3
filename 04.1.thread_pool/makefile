# Define o compilador C
CC = gcc

# Define as flags de compilação
# -Wall: Ativa todos os avisos (warnings)
# -pthread: Linka com a biblioteca pthreads para suporte a threads
# -g: Inclui informações de depuração
CFLAGS = -Wall -pthread -g

# Define o nome do executável final
TARGET = test_pool

# Lista todos os ficheiros fonte (.c)
SRCS = testpool.c threadPool.c sharedBuffer.c ringBuffer.c

# Gera a lista de ficheiros objeto (.o) a partir dos ficheiros fonte
# Substitui a extensão .c por .o
OBJS = $(SRCS:.c=.o)

# Regra padrão: compila tudo
all: $(TARGET)

# Regra para construir o executável final
# Depende de todos os ficheiros objeto
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Regra genérica para compilar ficheiros .c em ficheiros .o
# $<: O primeiro pré-requisito (o ficheiro .c)
# $@: O alvo (o ficheiro .o)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regras de dependência específicas para os ficheiros de cabeçalho
# Garante que, se um ficheiro .h for modificado, os ficheiros .c que o incluem sejam recompilados.
testpool.o: threadpool.h
threadpool.o: threadPool.h sharedBuffer.h
sharedBuffer.o: sharedBuffer.h ringBuffer.h

# Regra para limpar os ficheiros gerados (objetos e executável)
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
