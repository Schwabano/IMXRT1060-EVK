###############################################################################
# Standard rules.
###############################################################################

DEPFLAGS = -MT $@ -MMD -MP -MF $(basename $@).d

CCRUN_STD  = $(CC)  -c $(DEPFLAGS) $(CFLAGS) $(INCLUDES) $(DEFINES) -o $@ $<
CPPRUN_STD = $(CPP) -c $(DEPFLAGS) $(CFLAGS) $(INCLUDES) $(DEFINES) -o $@ $< -D__cplusplus


DEPENDENCY_FILES = $(wildcard Makefile*) utilities.mk rules.mk ../../GeneratedCode/ewfiles.inc | precompile

$(OBJ_PATH)/%.o: %.c $(DEPENDENCY_FILES)
	@echo Compiling $<
#	@echo $(CCRUN_STD)
	$(CCRUN_STD)

$(OBJ_PATH)/%.o: %.cpp $(DEPENDENCY_FILES)
	@echo Compiling $<
#	@echo $(CPPRUN_STD)
	$(CPPRUN_STD)

$(OBJ_PATH)/%.d: ;
.PRECIOUS: $(OBJ_PATH)/%.d

