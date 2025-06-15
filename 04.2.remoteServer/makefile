# Makefile para o projeto do servidor

# Compilador C
CC = gcc

# Flags de compilação:
# -Wall: Ativa todos os warnings
# -g: Inclui informações de debug
# -pthread: Linka com a biblioteca de threads POSIX
# -std=c11: Usa o padrão C11
CFLAGS = -Wall -g -pthread -std=c11

# Ficheiros de cabeçalho
HEADERS = ringBuffer.h sharedBuffer.h threadPool.h logger.h socket_utils.h client_handle.h menu_handle.h remote_server.h

# Ficheiros de código fonte
SOURCES = ringBuffer.c sharedBuffer.c threadPool.c logger.c socket_utils.c client_handle.c menu_handle.c remote_server.c

# Ficheiros objeto gerados a partir dos fontes
OBJECTS = $(SOURCES:.c=.o)

# Nome do executável final
TARGET = remote_server

# Regra padrão: compila tudo
all: $(TARGET)

# Regra para compilar o executável principal
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

# Regra para compilar cada ficheiro .c em .o
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para limpar os ficheiros gerados
clean:
	rm -f $(OBJECTS) $(TARGET) server.log /tmp/server_socket_so /tmp/*_XXXXXX
	# O `rm -f /tmp/*_XXXXXX` tenta remover ficheiros temporários seguros criados por mkstemp.
	# Pode ser necessário executar com sudo dependendo das permissões para o socket UNIX.
	# rm -f $(OBJECTS) $(TARGET) server.log /tmp/server_socket_so 

# Regra para executar o servidor (opcional, para conveniência)
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run