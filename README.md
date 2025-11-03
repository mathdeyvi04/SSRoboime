# Descrição

Neste repositório, marcharemos no progresso de produzir um código proprietário da RoboIME 
para a categoria do **Simulation Soccer**.

Utilizaremos como muleta o repositório da equipe [FCPortugal](https://github.com/m-abr/FCPCodebase) e pouco a pouco construiremos o nosso. Observe que este é o pontapé inicial para anos de pesquisa e avanço.

# Ideias

### Devemos construir um equilíbrio entre Python e C++

Assim obteremos a desempenho do baixo nível e a legibilidade do alto. Exemplos demonstrativos da capacidade da interoperabilidade:

- [Aplicação](https://github.com/oKermorgant/cpp_python_integration) simples e rápida.
- [Aplicação](https://yanto.fi/2022/09/benchmark-of-python-c-bindings) mais aprofundada e complexa.

Tendo isso em mente, utilizaremos a ferramenta [nanobind](https://github.com/wjakob/nanobind) para garantir essa interação.
Caso haja qualquer dificuldade, usaremos [pybind11](https://github.com/pybind/pybind11) para saná-la.

### Partir do mínimo possível

É fato que existem pontos especialmente complexos, entretanto, devemos utilizar a muleta e caminhar até que
possamos correr.

Seguindo a [Wiki](https://gitlab.com/robocup-sim/SimSpark/-/wikis/home) do Simspark, dentre outras possibilidades, forçaremos nosso caminho desde a base.

### Organização

Diferente do que é feito no repositório muleta, vamos centralizar o gerenciamento de execuções e compilações pelo
Makefile.

Assim, permitiremos um booting comum à qualquer máquina linux, sendo necessário apenas dependências mínimas
como `make`, `python` e `g++`.

Para o booting inicial, basta escrever no terminal: `make init`.
