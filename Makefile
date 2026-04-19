ARDUINO_CLI ?= arduino-cli
FQBN ?= arduino:renesas_uno:minima
WARNINGS ?= all
TCP1819_SRC ?= ../TCP1819

.PHONY: compile examples ci clean

compile: examples

examples:
	FQBN="$(FQBN)" WARNINGS="$(WARNINGS)" ARDUINO_CLI="$(ARDUINO_CLI)" TCP1819_SRC="$(TCP1819_SRC)" \
	./tools/compile-examples.sh

ci: examples

clean:
	rm -rf build