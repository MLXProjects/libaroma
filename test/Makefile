# main variables
AR = ar
CC = gcc
RM = rm -rf
MKDIR = mkdir -p
RANLIB = ranlib
LIB_CFLAGS = -fPIC -Wall -Wextra -O3 -DMLX_LIBVER=2.0
LIB_CFLAGS+= -I./include -I./src
LIB_CFLAGS+= -I./src/plat/linux
LIB_INCLUDE= -I./examples/libtest

# paths to be used
OUTDIR=./lib
OBJDIR=$(OUTDIR)/obj

#target files
LIB_STATIC = libmlx.a
LIB_SHARED = libmlx.so
TEST_SHARED= test
TEST_STATIC= test-static

#cflags for lib linking
TEST_CFLAGS = -L$(OUTDIR) -lmlx

# paths for library source/objects
LIB_DIR = examples/libtest
LIB_SUBDIR = af
DIR_OBJ = ./obj

#source, headers & target objects list
LIB_SRC = $(wildcard $(LIB_DIR)/*.c $(foreach fd, $(LIB_SUBDIR), $(LIB_DIR)/$(fd)/*.c))
LIB_HDR = $(wildcard $(LIB_DIR)/*.h)
LIB_OBJ = $(addprefix $(OBJDIR)/, $(LIB_SRC:.c=.o))

#log paths
$(info LIB_SRC is $(LIB_SRC))
$(info LIB_HDR is $(LIB_HDR))
$(info LIB_OBJ is $(LIB_OBJ))

#paths for test program
TEST_SRC = $(wildcard examples/test/*.c)
TEST_HDR = $(wildcard examples/test/*.h)

.PHONY: all
#build shared library and test by default
all: shared
shared: $(LIB_SHARED) $(TEST_SHARED)
static: $(TEST_STATIC) $(LIB_STATIC)

#build shared library from objects list
$(LIB_SHARED): $(LIB_OBJ) $(LIB_HDR)
	$(info Building $@)
	$(CC) -shared -o $(OUTDIR)/$(LIB_SHARED) $(LIB_INCLUDE) $(LIB_CFLAGS) $^

#build static library from objects list
$(LIB_STATIC): $(LIB_OBJ) $(LIB_HDR)
	$(info Building $@)
	$(AR) -r -c -s $(OUTDIR)/$@ $^
	$(RANLIB) $(OUTDIR)/$@

#build test program
$(TEST_SHARED): $(TEST_SRC) $(TEST_HDR) $(LIB_SHARED)
	$(info Building $@)
	$(CC) -o $(OUTDIR)/$@ $(TEST_SRC) $(LIB_INCLUDE) $(TEST_CFLAGS) -Wl,-rpath=.

#build static test program
$(TEST_STATIC): $(TEST_SRC) $(TEST_HDR) $(LIB_STATIC)
	$(info Building $@)
	$(CC) -o $(OUTDIR)/$@ $(TEST_SRC) $(LIB_INCLUDE) $(TEST_CFLAGS) -static

#build required object from c file
$(OBJDIR)/%.o: %.c $(LIB_HDR)
	#$(info Building $@ from $^)
	@$(MKDIR) $(@D)
	$(CC) -c -o $@ $< $(LIB_CFLAGS) $(LIB_INCLUDE)

.PHONY: clean
clean:
	$(RM) $(OUTDIR)/*