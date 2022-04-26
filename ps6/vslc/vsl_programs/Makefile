VSLC := ../src/vslc
AS := gcc

# This updated makefile calls the relevant print for each assignment
# The added target compile will attempt to compile all vsl source files,
# for ps5 and ps6. Other programs are mostly not interesting after the their
# targeted assignment.

# Call `vslc -h` to see the available flags, and call `vslc [flags] < file.vsl`
# to compile a single file.

PS2_EXAMPLES := $(patsubst ps2-parser/%.vsl, ps2-parser/%.ast, $(wildcard ps2-parser/*.vsl))
PS3_EXAMPLES := $(patsubst ps3-simplify/%.vsl, ps3-simplify/%.sast, $(wildcard ps3-simplify/*.vsl))
PS4_EXAMPLES := $(patsubst ps4-symtab/%.vsl, ps4-symtab/%.sym, $(wildcard ps4-symtab/*.vsl))
PS5_EXAMPLES := $(patsubst ps5-codegen1/%.vsl, ps5-codegen1/%.S, $(wildcard ps5-codegen1/*.vsl))
PS6_EXAMPLES := $(patsubst ps6-codegen2/%.vsl, ps6-codegen2/%.S, $(wildcard ps6-codegen2/*.vsl))

PS5_OBJECTS := $(PS5_EXAMPLES:.S=.bin)
PS6_OBJECTS := $(PS6_EXAMPLES:.S=.bin)
# OBJECTS := $(PS5_OBJECTS) $(PS4_EXAMPLES:.sym=.bin) $(PS3_EXAMPLES:.sast=.bin) $(PS2_EXAMPLES:.ast=.bin)
OBJECTS := $(PS5_OBJECTS) $(PS6_OBJECTS)
all: $(PS2_EXAMPLES) $(PS3_EXAMPLES) $(PS4_EXAMPLES) $(PS5_EXAMPLES) $(PS6_EXAMPLES)
ps2: $(PS2_EXAMPLES)
ps3: $(PS3_EXAMPLES)
ps4: $(PS4_EXAMPLES)
ps5: $(PS5_EXAMPLES)

%.ast: %.vsl
	$(VSLC) -t -q < $^ > $@ 2> $@
%.sast: %.vsl
	$(VSLC) -T -q < $^ > $@ 2> $@
%.sym: %.vsl
	$(VSLC) -s -q < $^ > $@ 2> $@
%.S: %.vsl
	$(VSLC) < $^ > $@
# This target is only tested on x86-linux
%.bin: %.S
	$(AS) -no-pie -o $@ $^

ps5-compile: $(PS5_OBJECTS)
ps6-compile: $(PS6_OBJECTS)
compile: $(OBJECTS)

clean:
	-rm -r */*.ast */*.sast */*.sym */*.bin */*.S