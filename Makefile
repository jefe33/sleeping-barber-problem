SEM = semaphore-mutex/main.c
COND = conditional-variables/main.c

all: sem cond

sem: $(SEM)
	gcc -pthread -o sem $(SEM)

cond: $(COND)
	gcc -pthread -o cond $(COND)

.PHONY: clean
clean:
	rm sem cond
