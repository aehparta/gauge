# makefile configuration

TARGET          = simple_capture
C_SOURCES       = \
	simple_capture.c

MAKE            = make
CC              = gcc
OBJDUMP         = objdump

INCLUDES        = -I../src
DEFINES         = 

C_CFLAGS        = $(INCLUDES) $(DEFINES) -Wall -Wno-unused-label
LDFLAGS         = ../bin/libscope.a -ldl -lpthread

C_BUILDDIR      = ./build
C_BINDIR        = ../bin

# define object files from sources
C_OBJECTS       = $(C_SOURCES:%.c=$(C_BUILDDIR)/%.o)

all: $(TARGET)

# link target into binary
$(TARGET): $(C_OBJECTS)
	@echo
	@echo "Linking $@..."
	@mkdir -p $(C_BINDIR)
	@$(CC) $(C_OBJECTS) -o $(C_BINDIR)/$@ $(LDFLAGS)
	@echo "Build complete."
	@echo

-include $(C_OBJECTS:.o=.d)

# build target C-code
$(C_BUILDDIR)/%.o : %.c
	@echo "Compiling $<..."
	@mkdir -p `dirname $@`
	@$(CC) $(C_CFLAGS) -MMD -c $< -o $@

modules_compile:
	@for dir in $(MODULES); do \
		$(MAKE) -C $$dir; \
	done

modules_clean:
	@for dir in $(MODULES); do \
		$(MAKE) -C $$dir clean; \
	done

clean: modules_clean
	rm -rf $(C_BUILDDIR) $(C_BINDIR)
