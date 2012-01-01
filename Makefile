# - GEGEVENS BOT --------------
NAME    = level
FOLDER  = ./bin/

# - STATIC CONFIG -------------
CPP      = g++
CC       = gcc
RES      = res/resources.res
OBJ      = ConfigFile.o main.o
LINKOBJ  = ConfigFile.o main.o
LIBS     = -m32 -Wl,-rpath,. ./libaw_sdk.51.so -lstdc++
INCS     = 
CXXINCS  = 
CXXFLAGS = $(CXXINCS) -D BINDTOIP -D LINUX -m32
CFLAGS   = $(INCS) -D BINDTOIP -D LINUX -m32
BIN      = $(NAME).exe
RM       = rm -f
CP       = cp

# - ACTIONS -------------------
.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	${RM} $(OBJ) $(BIN)
	
$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o "$(BIN)" $(LIBS)
	$(CP) "$(BIN)" "$(FOLDER)$(NAME)"
	$(CP) "libaw_sdk.51.so" "$(FOLDER)"
	$(RM) "$(BIN)"

main.o: main.cpp
	$(CPP) -c main.cpp -o main.o $(CXXFLAGS)
