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
	.obj/src/devices.o \
	.obj/src/razer-nostromo.o

all: narcissus nartutor

narcissus: .obj/src/narcissus.o $(COMMON_OBJS)
	@echo LINK $@
	@mkdir -p $(dir $@)
	$(hide) gcc -o $@ $^ $(CFLAGS) $(LIBS)

nartutor: .obj/src/nartutor.o $(COMMON_OBJS)
	@echo LINK $@
	@mkdir -p $(dir $@)
	$(hide) gcc -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	@echo CLEAN
	$(hide) rm -rf narcissus nartutor .obj

.obj/%.o: %.c
	@echo COMPILE $@
	@mkdir -p $(dir $@)
	$(hide) gcc -c -o $@ $< $(CFLAGS)

.obj/%.d: %.c
	@echo DEPEND $@
	@mkdir -p $(dir $@)
	$(hide) gcc -MP -MM -MF $@ $< $(CFLAGS)

-include .obj/src/narcissus.d
-include .obj/src/devices.d
-include .obj/src/razer-nostromo.d
-include .obj/src/nartutor.d

