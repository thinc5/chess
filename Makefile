CC				:= gcc
CFLAGS  		:= -std=gnu11 -O3 -Wall -pedantic -MD
LINKER  		:= gcc
LFLAGS			:=
FORMATTER		:= uncrustify
FORMAT_CONFIG	:= clean.cfg

LFLAGS_2D		:= -lm -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

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

TARGET_CLI	:= chess
TARGET_2D	:= chess2d
TARGET_LIB	:= chess.so

OBJ_CLI = $(filter-out $(OBJDIR)/$(TARGET_2D).o, $(OBJECTS))
OBJ_2D := $(filter-out $(OBJDIR)/$(TARGET_CLI).o, $(OBJECTS))
OBJ_LIB := $(filter-out $(OBJDIR)/$(TARGET_CLI)*.o, $(OBJECTS))

ifeq ($(DEBUG), 1)
    CFLAGS += -g -DDEBUG
endif

all: format build_cli build_2d # build_lib

.PHONY:	$(OUTDIR)/$(TARGET_CLI) $(OUTDIR)/$(TARGET_2D) $(OUTDIR)/$(TARGET_LIB) clean format_clean format $(FORMAT_TARGETS)

# Build objects
$(OBJDIR):
	@$(MKDIR) $(OBJDIR)

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	$(info Compiled $<)

# Build CLI
$(OUTDIR)/$(TARGET_CLI):
	@$(LINKER) $(OBJ_CLI) $(LFLAGS) -o $@
	$(info Binary: $@)

build_cli: $(OBJDIR) $(OBJECTS) $(OUTDIR)/$(TARGET_CLI)

# Build 2D
$(OUTDIR)/$(TARGET_2D):
	@$(LINKER) $(OBJ_2D) $(LFLAGS) $(LFLAGS_2D) -o $@
	$(info Binary: $@)

build_2d: $(OBJDIR) $(OBJECTS) $(OUTDIR)/$(TARGET_2D)

# Build shared object.
$(OUTDIR)/$(TARGET_LIB):
	@$(LINKER) $(OBJDIR)/*.o $(LFLAGS) -fPIC -shared -o $(TARGET_LIB)
	$(info $(TARGET_LIB))

build_lib: $(OBJDIR) $(OBJECTS) $(OUTDIR)/$(TARGET_LIB)

$(FORMAT_TARGETS):
	@$(FORMATTER) -c $(FORMAT_CONFIG) -q -f $@ -o $@

format_clean:
	@$(RM) *.unc*

format: $(FORMAT_TARGETS) format_clean

clean:
	@$(RM) $(OBJDIR)
	@$(RM) $(TARGET_CLI)
	@$(RM) $(TARGET_2D)
	@$(RM) $(TARGET_LIB)
