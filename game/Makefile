# Name of the project
PROJ_NAME=candycrash
PROJ_EXE=$(PROJ_NAME).exe

# Allegro inclusion
PATH_ALLEGRO=C:\Users\coutl\Coding\C\allegro-5.0.10-mingw-4.7.0
LIB_ALLEGRO=\lib\liballegro-5.0.10-monolith-mt.a
INCLUDE_ALLEGRO=\include

# .c file
C_SOURCE=$(PROJ_NAME).c

# Object file
OBJ=$(C_SOURCE:.c=.o)

# Compiler
CC=gcc

#
# Compilation and linking
#
all: $(PROJ_EXE)

$(PROJ_EXE): $(OBJ)
	$(CC) -o $@ $^ $(PATH_ALLEGRO)$(LIB_ALLEGRO)

$(OBJ): $(C_SOURCE)
	$(CC) -I $(PATH_ALLEGRO)$(INCLUDE_ALLEGRO) -c $<

clean:
	del *.o *.exe