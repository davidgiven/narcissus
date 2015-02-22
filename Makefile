hide = @

CFLAGS = \
	-g \
	-Os \
	-std=c99 \
	-D_BSD_SOURCE

LIBS = \
	-lfakekey \
	-lX11 \
	-lXi

COMMON_OBJS = \
	.obj/devices.o \
	.obj/razer-nostromo.o

all: narcissus mapgen statanal nartutor

narcissus: .obj/narcissus.o $(COMMON_OBJS)
	@echo LINK $@
	@mkdir -p .obj
	$(hide) gcc -o $@ $^ $(CFLAGS) $(LIBS)

nartutor: nartutor.o $(COMMON_OBJS)
	@echo LINK $@
	@mkdir -p .obj
	$(hide) gcc -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	@echo CLEAN
	$(hide) rm -f narcissus mapgen statanal nartutor .obj/*.o .obj/*.d

.obj/%.o: %.c
	@echo COMPILE $@
	$(hide) gcc -c -o $@ $< $(CFLAGS)

.obj/%.d: %.c
	@echo DEPEND $@
	@mkdir -p .obj
	$(hide) gcc -MP -MM -MF $@ $< $(CFLAGS)

%: %.c
	@echo COMPILELINK $@
	$(hide) gcc -o $@ $< $(CFLAGS) $(LIBS)

-include .obj/narcissus.d
-include .obj/devices.d
-include .obj/razer-nostromo.d
-include .obj/nartutor.d

