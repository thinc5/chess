CC				:= gcc
CFLAGS  		:= -std=gnu11 -O3 -Wall -pedantic -fPIC -D_FORTIFY_SOURCE=2 #-MD
ARCHIVER		:= ar
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

TARGET			:= chess
TARGET_CLI		:= $(TARGET)
TARGET_2D		:= $(TARGET)2d
TARGET_ARCHIVE	:= lib$(TARGET).a
TARGET_LIB		:= lib$(TARGET).so

OBJ_CLI			:= $(OBJDIR)/$(TARGET_CLI).o
OBJ_2D			:= $(filter-out $(OBJDIR)/$(TARGET_CLI).o, $(OBJECTS))
OBJ_LIB			:= $(filter-out $(OBJDIR)/$(TARGET_2D).o $(OBJDIR)/$(TARGET_CLI).o, $(OBJECTS))

# OBJ_DIRS 		:= $(OBJDIR) $(shell ls -d $(INCDIR)/*/**.h | awk '"./"{sub("$(INCDIR)", "$(OBJDIR)")} 1' | awk '"./"{sub(".h", "")} 1')
OBJ_DIRS		:= $(OBJDIR) "$(OBJDIR)/core"


ifeq ($(DEBUG), 1)
    CFLAGS += -g -DDEBUG
endif

all: format build_archive build_lib build_cli build_2d

.PHONY:	$(OUTDIR)/$(TARGET_CLI) $(OUTDIR)/$(TARGET_2D) $(OUTDIR)/$(TARGET_ARCHIVE) $(OUTDIR)/$(TARGET_LIB) clean format_clean format $(FORMAT_TARGETS)

# Build objects
$(OBJ_DIRS):
	@$(MKDIR) $(OBJ_DIRS)

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	$(info Compiled $<)

# Build archive.
$(OUTDIR)/$(TARGET_ARCHIVE):
	@$(ARCHIVER) rcs $(TARGET_ARCHIVE) $(OBJ_LIB)
	$(info Binary: $(TARGET_ARCHIVE))

build_archive: $(OBJDIR) $(OBJECTS) $(OUTDIR)/$(TARGET_ARCHIVE)

# Build shared object.
$(OUTDIR)/$(TARGET_LIB):
	@$(LINKER) $(OBJ_LIB) $(LFLAGS) -fPIC -shared -o $(TARGET_LIB)
	$(info Binary: $(TARGET_LIB))

build_lib: $(OBJDIR) $(OBJECTS) $(OUTDIR)/$(TARGET_LIB)

# Build CLI
$(OUTDIR)/$(TARGET_CLI):
	@$(LINKER) $(OBJ_CLI) $(LFLAGS) -L. -l$(TARGET) -o $@
	$(info Binary: $@)

build_cli: $(OBJDIR) $(OUTDIR)/$(TARGET_ARCHIVE) $(OBJECTS) $(OUTDIR)/$(TARGET_CLI)

# Build 2D
$(OUTDIR)/$(TARGET_2D):
	@$(LINKER) $(OBJ_2D) $(LFLAGS) $(LFLAGS_2D) $(LFLAGS) -L. -l$(TARGET) -o $@
	$(info Binary: $@)

build_2d: $(OBJDIR) $(OBJECTS) $(OUTDIR)/$(TARGET_2D)

$(FORMAT_TARGETS):
	@$(FORMATTER) -c $(FORMAT_CONFIG) -q -f $@ -o $@

format_clean:
	@$(RM) **.unc*
	@$(RM) **/**.unc*

format: $(FORMAT_TARGETS) format_clean

clean:
	@$(RM) $(OBJDIR)
	@$(RM) $(TARGET_CLI)
	@$(RM) $(TARGET_2D)
	@$(RM) $(TARGET_ARCHIVE)
	@$(RM) $(TARGET_LIB)
