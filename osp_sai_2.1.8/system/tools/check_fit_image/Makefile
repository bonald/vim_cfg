

SRCS = fdt.c fdt_ro.c fdt_wip.c fdt_sw.c fdt_rw.c fdt_strerror.c fdt_empty_tree.c fdt_addresses.c fdt_region.c
SRCS += check_fit_image.c
CPPFLAGS += -I.


OBJ = check_fit_image.o fdt.o fdt_ro.o fdt_wip.o fdt_sw.o fdt_rw.o fdt_strerror.o fdt_empty_tree.o fdt_addresses.o fdt_region.o

tool: $(OBJ)
	gcc -o check_fit_image $(OBJ) $(CPPFLAGS)

clean:
	rm check_fit_image $(OBJ)