TARGET := environment.so

# Flags padrão para criação de biblioteca compartilhada
CXXFLAGS := -O3 -Wall -shared -std=c++20 -fPIC

# Pede ao Python o caminho dos headers do sistema (Python.h)
PY_INC ?= $(shell python3 -c "import sysconfig; print(sysconfig.get_path('include'))")
# Pede ao Nanobind onde ele está instalado (nanobind.h)
NB_INC ?= $(shell python3 -c "import nanobind; print(nanobind.include_dir())")
# -- Não sei explicar exatamente essa necessidade, mas é a realidade
NB_ROOT ?= $(shell python3 -c "import os, nanobind; print(os.path.dirname(nanobind.__file__))")
ROBIN_INC ?= $(NB_ROOT)/ext/robin_map/include
# Unimos ambos
INCLUDES = -I$(PY_INC) -I$(NB_INC) -I$(ROBIN_INC)

# Diferente do pybind11, nanobind possui bibliotecas a serem compiladas
NB_SRC ?= $(shell python3 -c "import os, nanobind; print(os.path.join(os.path.dirname(nanobind.__file__), 'src', 'nb_combined.cpp'))")

all:
	$(CXX) $(CXXFLAGS) $(INCLUDES) module_main.cpp $(NB_SRC) -o $(TARGET)

gdb:
	@g++ -g -std=c++20 debug.cc; gdb ./a.out; rm a.out;

clean:
	@rm -rf *.o