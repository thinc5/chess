CC				:= gcc
CFLAGS  		:= -std=gnu11 -lm -O3 -Wall -pedantic -fPIC -D_FORTIFY_SOURCE=2 -MD
ARCHIVER		:= ar
LINKER  		:= gcc
LFLAGS			:= -lm
FORMATTER		:= uncrustify
FORMAT_CONFIG	:= clean.cfg

LFLAGS_SDL		:= -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

WASM_CC			:= 

SRCDIR			:= .
INCDIR			:= .
OBJDIR			:= obj
OUTDIR			:= .
OBJDIR_2D		:= $(OBJDIR)/2d
OBJDIR_3D		:= $(OBJDIR)/3d

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
TARGET_3D		:= $(TARGET)3d
TARGET_ARCHIVE	:= lib$(TARGET).a
TARGET_LIB		:= lib$(TARGET).so

OBJ_MAIN		:= $(filter $(OBJDIR)/$(TARGET)%.o, $(OBJECTS))
OBJ_2D_SPECIFIC := $(filter $(OBJDIR_2D)/%.o, $(OBJECTS))
OBJ_3D_SPECIFIC := $(filter $(OBJDIR_3D)/%.o, $(OBJECTS))

OBJ_CLI			:= $(filter-out						\
					$(OBJDIR)/$(TARGET_2D).o		\
					$(OBJDIR)/$(TARGET_3D).o		\
					$(OBJ_2D_SPECIFIC)				\
					$(OBJ_3D_SPECIFIC),				\
					$(OBJECTS)						\
				)
OBJ_2D			:= $(filter-out						\
					$(OBJDIR)/$(TARGET_CLI).o		\
					$(OBJDIR)/$(TARGET_3D).o		\
					$(OBJ_3D_SPECIFIC),				\
					$(OBJECTS)						\
				)
OBJ_3D			:= $(filter-out						\
					$(OBJDIR)/$(TARGET_CLI).o		\
					$(OBJDIR)/$(TARGET_2D).o		\
					$(OBJ_2D_SPECIFIC),				\
					$(OBJECTS)						\
				)
OBJ_LIB			:= $(filter-out						\
					$(OBJ_MAIN)						\
					$(OBJ_2D_SPECIFIC)				\
					$(OBJ_3D_SPECIFIC),				\
					$(OBJECTS)						\
				)

# OBJ_DIRS 		:= $(OBJDIR) $(shell ls -d $(INCDIR)/*/**.h | awk '"./"{sub("$(INCDIR)", "$(OBJDIR)")} 1' | awk '"./"{sub(".h", "")} 1')
OBJ_DIRS		:= $(OBJDIR) "$(OBJDIR)/core" "$(OBJDIR_2D)" "$(OBJDIR_3D)"

ifeq ($(DEBUG), 1)
    CFLAGS += -g -DDEBUG
endif

all: format build_archive build_lib build_cli build_2d build_3d

.PHONY:	$(OUTDIR)/$(TARGET_CLI) $(OUTDIR)/$(TARGET_2D) $(OUTDIR)/$(TARGET_3D) $(OUTDIR)/$(TARGET_ARCHIVE) $(OUTDIR)/$(TARGET_LIB) clean format_clean format $(FORMAT_TARGETS)

# Build objects
$(OBJ_DIRS):
	@$(MKDIR) $(OBJ_DIRS)

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(info Compiling $<)
	@$(CC) $(CFLAGS) -c $< -o $@
	$(info Compiled $<)

# Build archive.
$(OUTDIR)/$(TARGET_ARCHIVE):
	$(info Compiling: $@)
	@$(ARCHIVER) rcs $(TARGET_ARCHIVE) $(OBJ_LIB)
	$(info Binary: $@)

build_archive: $(OBJDIR) $(OBJECTS) $(OUTDIR)/$(TARGET_ARCHIVE)

# Build shared object.
$(OUTDIR)/$(TARGET_LIB):
	$(info Compiling: $@)
	@$(LINKER) $(OBJ_LIB) $(LFLAGS) -fPIC -shared -o $(TARGET_LIB)
	$(info Binary: $@)

build_lib: $(OBJDIR) $(OBJECTS) $(OUTDIR)/$(TARGET_LIB)

# Build CLI
$(OUTDIR)/$(TARGET_CLI):
	$(info Compiling: $@)
	@$(LINKER) $(OBJ_CLI) $(LFLAGS) -L. -l$(TARGET) -o $@
	$(info Binary: $@)

build_cli: $(OBJDIR) $(OUTDIR)/$(TARGET_ARCHIVE) $(OBJECTS) $(OUTDIR)/$(TARGET_CLI)

# Build 2D
$(OUTDIR)/$(TARGET_2D):
	$(info Compiling: $@)
	@$(LINKER) $(OBJ_2D) $(LFLAGS) $(LFLAGS_SDL) $(LFLAGS) -L. -l$(TARGET) -o $@
	$(info Binary: $@)

build_2d: $(OBJDIR) $(OBJECTS) $(OUTDIR)/$(TARGET_2D)

# Build 3D
$(OUTDIR)/$(TARGET_3D):
	$(info Compiling: $@)
	@$(LINKER) $(OBJ_3D) $(LFLAGS) $(LFLAGS_SDL) $(LFLAGS) -L. -l$(TARGET) -o $@
	$(info Binary: $@)

build_3d: $(OBJDIR) $(OBJECTS) $(OUTDIR)/$(TARGET_3D)

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
