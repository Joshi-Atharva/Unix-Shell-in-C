CC       := gcc
CFLAGS   := -Wall -Wextra -std=gnu11 -Iinclude -Isrc -g
FLEX     := flex
BISON    := bison

SRCS     := src/main.c src/command.c src/executor.c src/builtins.c
PARSER_Y := src/parser.y
LEXER_L  := src/lexer.l

PARSER_C := src/parser.tab.c
PARSER_H := src/parser.tab.h
LEXER_C  := src/lexer.yy.c

OBJS     := $(SRCS:.c=.o) src/parser.tab.o src/lexer.yy.o
TARGET   := myshell

.PHONY: all clean

all: $(TARGET) shell

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

shell: $(TARGET)
	ln -sf $(TARGET) shell

$(PARSER_C) $(PARSER_H): $(PARSER_Y)
	$(BISON) -d -o $(PARSER_C) $<

$(LEXER_C): $(LEXER_L) $(PARSER_H)
	$(FLEX) -o $(LEXER_C) $<

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

src/parser.tab.o: $(PARSER_C)
	$(CC) $(CFLAGS) -c $< -o $@

src/lexer.yy.o: $(LEXER_C)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) shell src/*.o $(PARSER_C) $(PARSER_H) $(LEXER_C)
