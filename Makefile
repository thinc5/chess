CC				:= gcc
CFLAGS  		:= -std=gnu11 -O3 -Wall -pedantic -MD
LINKER  		:= gcc
LFLAGS			:=
FORMATTER		:= uncrustify
FORMAT_CONFIG	:= clean.cfg

SRCDIR			:= .
INCDIR			:= .
OBJDIR			:= obj
OUTDIR			:= .

RM				:= rm -rf
MKDIR			:= mkdir -p
FINDC			:= du -a $(SRCDIR) | grep -E '\.(c)$$' | awk '{print $$2}'
FINDH			:= du -a $(INCDIR) | grep -E '\.(h)$$' | awk '{print $$2}'

SOURCES  		:= $(shell $(FINDC))
INCLUDES 		:= $(shell $(FINDH))
FORMAT_TARGETS	:= $(SOURCES) $(INCLUDES)
OBJECTS  		:= $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

TARGET	:= chess

ifeq ($(DEBUG), 1)
    CFLAGS += -g -DDEBUG
endif

default: build

all: build

$(OBJDIR):
	@$(MKDIR) $(OBJDIR)

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	$(info Compiled $<)

$(OUTDIR)/$(TARGET):
	@$(LINKER) $(OBJDIR)/*.o $(LFLAGS) -o $@
	$(info Binary: $@)

build: $(OBJDIR) $(OBJECTS) $(OUTDIR)/$(TARGET)
	$(info $(sed 's/$(INCDIR)/$(OBJDIR)/g'))

rebuild: clean
	$(MAKE) build

.PHONY:	clean format_clean format $(FORMAT_TARGETS)

$(FORMAT_TARGETS):
	@$(FORMATTER) -c $(FORMAT_CONFIG) -q -f $@ -o $@

format_clean:
	@$(RM) *.unc*

format: $(FORMAT_TARGETS) format_clean

clean:
	@$(RM) $(OBJDIR)
	
	@$(RM) $(TARGET)
