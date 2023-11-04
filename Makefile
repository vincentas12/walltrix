CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = -lX11 -lm -lXrandr
INSTALL_DIR = /usr/local/bin

SRC = main.c

all: walltrix

walltrix: $(SRC)
	$(CC) $(CFLAGS)  -o $@ $^ $(LDFLAGS) 

install: walltrix
	cp walltrix $(INSTALL_DIR)
update: walltrix
	pkill walltrix
	cp walltrix $(INSTALL_DIR)
	exec walltrix
upgrade: walltrix
	pkill walltrix
	rm -f walltrix
	$(CC) $(CFLAGS)  -o $@ $^ $(LDFLAGS) 
	cp walltrix $(INSTALL_DIR)
	exec walltrix



.PHONY: clean uninstall

clean:
	rm -f walltrix

uninstall:
	rm -f $(INSTALL_DIR)/walltrix

