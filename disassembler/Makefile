TARGET = md_dis
LIBS = -lm
ifndef CC
CC = gcc
endif
CFLAGS += -g0 -Os -Wall -g
ifndef EXE
EXE = $(TARGET)
endif

ifndef ARCH
ARCH = $(shell $(CC) -dumpmachine)
endif

OBJDIR = obj-$(ARCH)

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, $(OBJDIR)/%.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

#create obj dirs
$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: %.c $(HEADERS) | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $(EXE)

clean:
	-rm -f $(OBJECTS)
	-rm -f $(EXE)
