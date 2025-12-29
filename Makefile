CC = gcc
CFLAGS = -O2 -Wall -Wextra -pthread
LDFLAGS = -lssl -lcrypto

SRC = main.c \
      core/tcp.c \
      core/http.c \
      core/downloader.c \
      core/progress.c \
      core/util.c

OUT = out/downloader

all: $(OUT)

out:
	mkdir -p out

$(OUT): out $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

clean:
	rm -rf out
