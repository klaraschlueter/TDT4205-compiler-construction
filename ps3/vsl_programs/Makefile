VSLC := ../src/vslc

PS2_EXAMPLES := $(patsubst ps2-parser/%.vsl, ps2-parser/%.ast, $(wildcard ps2-parser/*.vsl))
PS3_EXAMPLES := $(patsubst ps3-simplify/%.vsl, ps3-simplify/%.ast, $(wildcard ps3-simplify/*.vsl))

all: $(PS2_EXAMPLES) $(PS3_EXAMPLES)
ps2: $(PS2_EXAMPLES)
ps3: $(PS3_EXAMPLES)

%.ast: %.vsl
	$(VSLC) -t < $^ > $@

clean:
	-rm -r */*.ast