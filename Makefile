CFLAGS = \
	-g \
	-std=c99 \

LIBS = \
	-lfakekey \
	-lX11 \
	-lXi

all: narcissus mapgen

%: %.c
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

