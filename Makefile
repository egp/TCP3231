ARDUINO_CLI ?= arduino-cli
FQBN ?= arduino:renesas_uno:minima
WARNINGS ?= all

.PHONY: compile examples ci clean

compile: examples

examples:
	FQBN="$(FQBN)" WARNINGS="$(WARNINGS)" ARDUINO_CLI="$(ARDUINO_CLI)" \
	./tools/compile-examples.sh

ci: examples

clean:
	rm -rf build