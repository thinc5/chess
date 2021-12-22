CC		:= gcc
CFLAGS  := -std=gnu11 -O3 -Wall -pedantic -MD
LINKER  := gcc
LFLAGS	:=
# LFLAGS	+=
TARGET	:= chess

SRCDIR	:= .
INCDIR	:= .
OBJDIR	:= obj
BINDIR	:= .

rm		:= rm -rf
mkdir	:= mkdir -p
findc	:= du -a $(SRCDIR) | grep -E '\.(c)$$' | awk '{print $$2}'
findh	:= du -a $(INCDIR) | grep -E '\.(h)$$' | awk '{print $$2}'

SOURCES  	:= $(shell $(findc))
INCLUDES 	:= $(shell $(findh))
RESOURCES	:= ./$(SRCDIR)/tiles.h ./$(SRCDIR)/font.h

OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

XTRADIR  := $(shell ls -d $(INCDIR)/*/** | grep -v $(OBJDIR) | sed 's/$(INCDIR)/$(OBJDIR)/g')
$(shell $(mkdir) $(OBJDIR) $(XTRADIR))

all: build

debug: CFLAGS += -DDEBUG -g
debug: LFLAGS += -DDEBUG -g
debug: build

build: $(OBJECTS) $(BINDIR)/$(TARGET)
	$(info $(sed 's/$(INCDIR)/$(OBJDIR)/g'))

rebuild: clean
	$(MAKE) build

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	$(info Compiled $<)

$(BINDIR)/$(TARGET): $(OBJECTS)
	@$(LINKER) $(OBJECTS) $(LFLAGS) -o $@
	$(info Binary: $@)

.PHONY:	clean

clean:
	$(rm) $(RESOURCES)
	@$(rm) $(OBJDIR)
	@echo "Cleanup complete!"
	@$(rm) $(BINDIR)/$(TARGET)
	@echo "Executable removed!"
