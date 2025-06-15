# Nome do executável final
TARGET = min_max

# Ficheiros fonte C
SRCS = min_max.c

# Opcões do compilador C
# -Wall: Habilita todos os avisos (warnings)
# -Wextra: Habilita avisos adicionais
# -g: Inclui informações de depuração (para usar com GDB, por exemplo)
# -pthread: Indica que o programa usa threads POSIX e liga com a biblioteca pthread
CFLAGS = -Wall -Wextra -g -pthread

# Opcões do linker (se forem necessárias libs adicionais, aqui)
LDFLAGS = -pthread

# Regra padrão: compila o executável
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(SRCS) $(CFLAGS) -o $@ $(LDFLAGS)

# Regra para limpar os ficheiros gerados
clean:
	$(RM) $(TARGET) *.o

.PHONY: all clean
