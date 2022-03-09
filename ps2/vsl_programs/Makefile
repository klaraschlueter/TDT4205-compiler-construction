VSLC := ../src/vslc

PS2_EXAMPLES := $(patsubst ps2-parser/%.vsl, ps2-parser/%.ast, $(wildcard ps2-parser/*.vsl))

all: $(PS2_EXAMPLES)
	echo $(PS2_EXAMPLES)

%.ast: %.vsl
	$(VSLC) -t < $^ > $@

clean:
	-rm -r */*.ast