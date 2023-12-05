CC = gcc
CFLAGS = -Wall

all: main
	$(CC) $(CFLAGS) -o download main.o

main: main.c
	$(CC) $(CFLAGS) -c main.c

# Files
# - anonymous:anonymous
#   /pub.txt
#
# - rcom:rcom
#   /pipe.txt
#   /files/archlinux-2023.03.01-x86_64.iso
#   /files/crab.mp4
#   /files/pic1.jpg
#   /files/pic2.png

test_anon: all
	./download ftp://netlab1.fe.up.pt/pub.txt

test_auth: all
	./download ftp://[rcom:rcom@]netlab1.fe.up.pt/pipe.txt

clean:
	rm -f main.o download pub.txt
	rm -f pipe.txt archlinux-2023.03.01-x86_64.iso crab.mp4 pic1.jpg pic2.png
