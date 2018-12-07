####################################################################
# Definitions                                                      #
####################################################################

# uniq is a function which removes duplicate elements from a list
uniq = $(strip $(if $1,$(firstword $1) \
       $(call uniq,$(filter-out $(firstword $1),$1))))

PROJECTNAME = clientserver

OBJ_DIR = build
EXE_DIR = exe

####################################################################
# Flags                                                            #
####################################################################

INCLUDEPATHS += \
-I./inc

CFLAGS = 

LDFLAGS = -lWs2_32

####################################################################
# Files                                                            #
####################################################################

C_SRC +=  \
src/server.c \
src/client.c \
main.c 

LIBS = 

####################################################################
# Rules                                                            #
####################################################################

C_FILES = $(notdir $(C_SRC) )
C_PATHS = $(call uniq, $(dir $(C_SRC) ) )
C_OBJS = $(addprefix $(OBJ_DIR)/, $(C_FILES:.c=.o))

$(shell mkdir $(OBJ_DIR)>NUL 2>&1)
$(shell mkdir $(EXE_DIR)>NUL 2>&1)

vpath %.c $(C_PATHS)

# Create objects from C SRC files
$(OBJ_DIR)/%.o: %.c
	@echo -e "\033[1;32m[Building $@]\033[0m"
	gcc $(CFLAGS) $(INCLUDEPATHS) -c -o $@ $<
	@echo -e  "\033[1;32m[Finished $@]\033[0m"

# Link
$(EXE_DIR)/$(PROJECTNAME): $(C_OBJS) $(LIBS)
	@echo -e "\033[1;32m[Linking $@]\033[0m"
	gcc $^ $(LDFLAGS) -o $@
	@echo -e  "\033[1;32m[Finished $@]\033[0m"
