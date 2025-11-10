# CFLAGS is not defined by default
CFLAGS=-MMD -Wall -Wextra -Werror -Wno-format-overflow -std=c17 -march=x86-64 -fdiagnostics-color=always

SRC=$(wildcard *.c)
OBJ=$(SRC:%.c=%.o)
DEP=$(OBJ:%.o=%.d)

EXE=f2h

# add any additional libraries here ...
# $(addprefix -l, m pthread GL)
LIBS=$(addprefix -l,)

.PHONY: clean

debug: CFLAGS += -ggdb -Wno-unused-parameter -Wno-unused-variable
debug: $(EXE)

remake: clean debug
.NOTPARALLEL: remake

release: CFLAGS += -Os -s -Wno-unused-result -fno-ident -fno-asynchronous-unwind-tables -faggressive-loop-optimizations
release: clean $(EXE)
.NOTPARALLEL: release

clean:
	$(RM) $(OBJ) $(DEP) $(EXE)

install: release
	sudo cp $(EXE) /usr/local/bin/$(EXE)

$(EXE): $(OBJ) .vscode/launch.json .vscode/tasks.json
	gcc -o $@ $(OBJ) $(LIBS)

-include $(DEP)

