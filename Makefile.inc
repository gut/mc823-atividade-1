# Generic fucking Makefile

DEBUG := 1
STATIC := 0

# Define the C compiler to use
CC := gcc
AR := ar
RANLIB := ranlib

# Some definitions.
# Add more with -DDEF, e.g, -DUSE_LINUX
DEFS +=

# Define compile-time flags
CFLAGS += -Wall -Werror -g
CFLAGS += $(DEFS)

# Define libraries to link into executable
# Add more with -llib, e.g, -lm
LIBS +=

# Define library paths in addition to /usr/lib
# Use: -Lpath, e.g, -L/mydir/lib
LFLAGS +=

# Define dynamic library to link into executable
LDFLAGS +=

LIBDIR := $(shell pwd)/lib

# Define directories containing header files other than /usr/include
# Use: -Ipath, e.g, -I/mydir/include
INCLUDES +=

.PHONY: default

default: all

LIB_TARGET_SO := $(foreach L,$(TARGET_LIBS),lib/lib$(L).so)
LIB_TARGET_A := $(foreach L,$(TARGET_LIBS),lib/lib$(L).a)
BIN_TARGET := $(foreach B,$(TARGET_BINS),bin/$(B))

ALL_TARGET :=
ALL_TARGET += $(LIB_TARGET_SO)
ALL_TARGET += $(LIB_TARGET_A)
ALL_TARGET += $(BIN_TARGET)

BINMODS := $(foreach B,$(TARGET_BINS),$(MODS_$(B)))
LIBMODS := $(foreach L,$(TARGET_LIBS),$(MODS_$(L)))
ALLMODS := $(BINMODS) $(LIBMODS)

# Compile library modules with -fpic (required for AMD64 and benefitial on other
# systems too)
$(LIBMODS:%=obj/%.o) : CFLAGS += -fpic

.PHONY: all
all: $(ALL_TARGET)
	@echo "Everything is up to date"

# Delete files if/when updating them fails.
.DELETE_ON_ERROR:

# Update stuff based on whether we have DEBUG=0 or DEBUG=1
ifneq ($(DEBUG),0)
	DEFS += -DDEBUG
	CFLAGS += -g -ggdb
	LDFLAGS += -g -ggdb
	OPTIM = -O0
else
	DEFS += -DNDEBUG
	OPTIM = -O2
endif

# Are we compiling static libraries?
ifneq ($(STATIC),0)
	$(BIN_TARGET) : LDFLAGS += -static
endif

# Add optimization flags to CFLAGS it them exist
ifneq ($(OPTIM),)
	CFLAGS += $(OPTIM)
endif

# Use rpath in order to execute binaries in-place.
ifneq ($(LIBDIR),)
	LDFLAGS += -Wl,-rpath,$(LIBDIR)
endif

##################################################
# How to build binaries/libraries

$(LIB_TARGET_SO): | lib/.d
	$(CC) $(LDFLAGS) -shared -o $@ \
		$(foreach L,$(LIBS) $(LIBS_$(patsubst lib/lib%.so,%,$@)),l$(L)) \
		$(foreach M,$(MODS_$(patsubst lib/lib%.so,%,$@)),obj/$(M).o)

$(LIB_TARGET_A): | lib/.d
	$(AR) cru $@ $(foreach M,$(MDOS_$(patsubst lib/lib%.a,%,$@)),obj/$(M).o)
	$(RANLIB) $@

$(BIN_TARGET): | bin/.d
	$(CC) $(LDFLAGS) -o $@ \
		$(foreach L,$(LIBS) $(LIBS_$(patsubst bin/%,%,$@)),-l$(L)) \
		$(foreach M,$(MODS_$(notdir $(basename $@))),obj/$(M).o)

################################
# Pattern matched build targets.

obj/%.o : src/%.c Makefile
	$(CC) -x c $(CFLAGS) -o $@ -c $<

obj/%.o : src/%.C Makefile
	$(CC) $(CFLAGS) -o $@ -c $<

deps/%.d : src/%.c Makefile | deps/.d
	$(CC) -x c $(CFLAGS) -o $@ -MM $<
	sed -e 's,$(notdir $*.o):,$@ obj/$*.o:,' -i $@

deps/%.d : src/%.C Makefile | deps/.d
	$(CC) $(CFLAGS) -o $@ -MM $<
	sed -e 's,$(notdir $*.o):,$@ obj/$*.o:,' -i $@

.PRECIOUS: %/.d
%/.d :
	mkdir -p $(dir $@)
	@touch $@

################################
# Dependancy files.

depfiles := deps/Makefile
deps/Makefile : Makefile | deps/.d
	@echo "Bringing dependancy Makefile up to date ..."
	@>$@
	@echo "# Each binary and library needs to depend on it's modules." >>$@
	@$(foreach B,$(TARGET_BINS),echo "bin/$(B) : $(patsubst %,obj/%.o,$(MODS_$(B)))" >>$@;)
	@$(foreach L,$(TARGET_LIBS),echo "lib/lib$(L).so lib/lib$(L).a : $(patsubst %,obj/%.o,$(MODS_$(L)))" >> $@;)
	@echo "# Inter-target dependencies." >> $@
	@$(if $(filter $(LIBS),$(TARGET_LIBS)),echo "$(strip $(LIB_TARGET_SO) $(BIN_TARGET)) : $(foreach L,$(filter $(LIBS),$(TARGET_LIBS)),lib/lib$(L).so lib/lib$(L).a)" >> $@;,)
	@$(foreach B,$(TARGET_BINS),$(if $(filter $(LIBS_$(B)),$(TARGET_LIBS)),echo "bin/$(B) : $(foreach L,$(filter $(LIBS),$(TARGET_LIBS)),lib/lib$(L).so lib/lib$(L).a)" >> $@;,))
	@$(foreach L,$(TARGET_LIBS),$(if $(filter $(LIBS_$(L)),$(TARGET_LIBS)),echo "lib/lib$(L).so : $(foreach L,$(filter $(LIBS_$(L)),$(TARGET_LIBS)),lib/lib$(L).so)" >> $@;,))
	@echo "# object file requires that it's directory exists before it is made." >>$@
	$(foreach M,$(ALLMODS),echo "obj/$(M).o : | $(dir obj/$(M)).d" >>$@;)

depfiles += $(patsubst %,deps/%.d,$(ALLMODS))
deps/.d : $(filter-out deps/.d,$(addsuffix .d,$(dir $(depfiles))))

.PHONY: clean distclean cleanbins cleandoc cleandeps cleanobjs

distclean: clean cleanbins cleandoc

clean : cleandeps cleanobjs
	$(RM) *~ *.swp

cleandeps:
	$(RM) -r deps

cleanobjs:
	$(RM) -r obj

cleanbins:
	$(RM) -r $(TARGET_BINS)

cleandoc:
	$(RM) -r doc/*.pdf

# Determine whether we need the depfiles or not.
NODEP_TARGETS := clean distclean
depinc := 1
ifneq (,$(fileter $(NODEP_TARGETS),$(MAKECMDGOALS)))
	depinc := 0
endif
ifneq (,$(filter-out $(NODEP_TARGETS),$(MAKECMDGOALS)))
	depinc := 1
endif

ifeq ($(depinc),1)
	-include $(depfiles)
endif

