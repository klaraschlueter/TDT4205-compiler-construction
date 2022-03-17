VSLC := ../src/vslc

PS2_EXAMPLES := $(patsubst ps2-parser/%.vsl, ps2-parser/%.ast, $(wildcard ps2-parser/*.vsl))
PS3_EXAMPLES := $(patsubst ps3-simplify/%.vsl, ps3-simplify/%.ast, $(wildcard ps3-simplify/*.vsl))
PS4_EXAMPLES := $(patsubst ps4-symtab/%.vsl, ps4-symtab/%.ast, $(wildcard ps4-symtab/*.vsl))

all: $(PS2_EXAMPLES) $(PS3_EXAMPLES) $(PS4_EXAMPLES)
ps2: $(PS2_EXAMPLES)
ps3: $(PS3_EXAMPLES)
ps4: $(PS4_EXAMPLES)

%.ast: %.vsl
	$(VSLC) -t < $^ > $@

clean:
	-rm -r */*.ast
