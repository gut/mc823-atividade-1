# Declare here all information needed to compile the project
# Full declaration should be the form:
# 	TARGET_LIBS = lib<n>... # where lib<n> is the nth library name
# 	TARGET_BINS = bin<n>... # where bin<n> is the nth binary name
#
# 	MODS_lib<n> = mod<n>...
# 	MODS_bin<n> = bin<n>...
#
#	LIBS = includelib<n> ...
#	LIBS_bin<n> = lib<n> ...

TARGET_LIBS =
TARGET_BINS = servidor cliente

LIBS_servidor =
LIBS_cliente =

MODS_servidor = servidor
MODS_cliente = cliente

include $(shell pwd)/Makefile.inc
