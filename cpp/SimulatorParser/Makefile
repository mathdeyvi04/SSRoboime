CXX := g++
CXXFLAGS := -O3 -Wall -shared -std=c++11 -fPIC
PYBIND11_FLAGS := $(shell python3 -m pybind11 --includes) # Não retira o shell daqui.

MODULE_NAME := simulator_parser
SOURCE_FILE := module_main.cpp

$(MODULE_NAME).so: $(SOURCE_FILE)
	$(CXX) $(CXXFLAGS) $(PYBIND11_FLAGS) $(SOURCE_FILE) -o $@

clean:
	@rm -f *.so

.PHONY: clean