# makefile configuration

TARGET          = dev-sigrok.so
C_SOURCES       = \
	src/dev.c

MAKE            = make
CC              = gcc
AR              = ar
OBJDUMP         = objdump

INCLUDES        = -Isrc
DEFINES         = 

C_CFLAGS        = $(INCLUDES) $(DEFINES) -Wall -fPIC `pkg-config glib-2.0 --cflags`
LDFLAGS         = -shared -lsigrok `pkg-config glib-2.0 --libs`

C_BUILDDIR      = ./build
C_BINDIR        = ../../bin/modules

# define object files from sources
C_OBJECTS       = $(C_SOURCES:%.c=$(C_BUILDDIR)/%.o)

all: $(TARGET)

# create library
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

clean:
	rm -rf $(C_BUILDDIR) $(C_BINDIR)/$(TARGET)