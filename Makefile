CFLAGS = \
	-g \
	-std=c99 \

LIBS = \
	-lX11 \
	-lXi

all: narcissus

%: %.c
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

