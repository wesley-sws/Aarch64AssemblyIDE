CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0 gtksourceview-3.0`
LIBS = `pkg-config --libs gtk+-3.0 gtksourceview-3.0`
OBJDIR = build

# Directories for the additional libraries
# ASSEMBLE_LIB_DIR = assemble_lib
# EMULATE_LIB_DIR = emulate_lib

# Paths to the static libraries
# ASSEMBLE_LIB = $(ASSEMBLE_LIB_DIR)/libassembler.a
# EMULATE_LIB = $(EMULATE_LIB_DIR)/libemulator.a


OBJS = $(OBJDIR)/main.o $(OBJDIR)/auto_complete.o $(OBJDIR)/menu.o $(OBJDIR)/events.o $(OBJDIR)/apply_css.o $(OBJDIR)/error_check.o $(OBJDIR)/tabs.o

all: $(OBJDIR) $(ASSEMBLE_LIB) $(EMULATE_LIB) assembly_ide

$(OBJDIR):
	@mkdir -p $(OBJDIR)

# Ensure libraries are built before main object files
# $(ASSEMBLE_LIB):
# 	$(MAKE) -C $(ASSEMBLE_LIB_DIR)

# $(EMULATE_LIB):
# 	$(MAKE) -C $(EMULATE_LIB_DIR)


$(OBJDIR)/main.o: src/main.c
	$(CC) $(CFLAGS) -c src/main.c -o $(OBJDIR)/main.o

$(OBJDIR)/auto_complete.o: src/auto_complete.c include/auto_complete.h
	$(CC) $(CFLAGS) -c src/auto_complete.c -o $(OBJDIR)/auto_complete.o

$(OBJDIR)/menu.o: src/menu.c include/menu.h
	$(CC) $(CFLAGS) -c src/menu.c -o $(OBJDIR)/menu.o

$(OBJDIR)/events.o: src/events.c include/events.h
	$(CC) $(CFLAGS) -c src/events.c -o $(OBJDIR)/events.o

$(OBJDIR)/apply_css.o: src/apply_css.c include/apply_css.h
	$(CC) $(CFLAGS) -c src/apply_css.c -o $(OBJDIR)/apply_css.o

$(OBJDIR)/tabs.o: src/tabs.c include/tabs.h
	$(CC) $(CFLAGS) -c src/tabs.c -o $(OBJDIR)/tabs.o

$(OBJDIR)/error_check.o: src/error_check.c include/error_check.h
	$(CC) $(CFLAGS) -c src/error_check.c -o $(OBJDIR)/error_check.o

assembly_ide: $(OBJS) # $(ASSEMBLE_LIB) $(EMULATE_LIB)
	$(CC) $(OBJS) $(LIBS) -o assembly_ide
#   $(CC) $(OBJS) $(ASSEMBLE_LIB) $(EMULATE_LIB) $(LIBS) -o assembly_ide

clean:
	rm -rf $(OBJDIR) assembly_ide
	# $(MAKE) -C $(ASSEMBLE_LIB_DIR) clean
	# $(MAKE) -C $(EMULATE_LIB_DIR) clean

