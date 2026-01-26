# Sumário

- [Descrição](#descrição)
- [Como Executar](#como-executar)
- [Padronizações](#padronizações)
  - [Construção da Base](#partir-do-mínimo-possível)
  - [Debugação](#debugação)
  - [Democratização do uso de Heap](#utilização-de-heap)
- [Linhas de Desenvolvimento](#linhas-de-desenvolvimento)
- [Necessidades de Trabalho](#necessidades-de-trabalho)

# Descrição

Neste repositório, marcharemos no progresso de produzir um código proprietário da RoboIME 
para a categoria do **Simulation Soccer**.

Utilizaremos como muleta o repositório da equipe [FCPortugal](https://github.com/m-abr/FCPCodebase) e pouco a pouco construiremos o nosso. Observe que este é o pontapé inicial para anos de pesquisa e avanço.

---
# Como Executar

A seguir, uma sequência mínima de comandos que podem ser inseridos para executar a aplicação:

### `make gdb_threads`

Compila com as flags corretas o arquivo [run_full_threads.cpp](src/run_full_threads.cpp) e executa utilizando o **debugger** gdb.

Neste arquivo, o código geral da equipe é executado por 11 threads diferentes, sendo cada agente controlado por 1 thread.

### `make gdb`

Compila com as flags corretas o arquivo [run_full_team.cpp](src/run_full_team.cpp) e executa utilizando o **debugger** gdb.

Neste arquivo, o código geral da equipe é executado por uma única thread, sendo todos os 11 agentes controlados por ela.

### `make gdb_player`

Compila com as flags corretas o arquivo [run_player.cpp](src/run_player.cpp) e executa utilizando o **debugger** gdb.

Neste arquivo, apenas 1 jogador é instanciado em campo.

### `make debug_vision`

Compila com as flags corretas o arquivo [run_full_team.cpp](src/run_full_team.cpp) e executa utilizando o **debugger** gdb.

Permite a utilização de outro código, [RobotVision.py](src/Utils/RobotVision.py), para que seja possível a visualização interna dos sensores 
do robô.

### Demais

É interessante que, conforme novos avanços forem alcançados, seja acrescentado aqui as possibilidades de execução.

Além de que, ao final do projeto, seja necessário um _ExecutionManager_ para que iniciantes tenham mais facilidade em realizar testes e compreender o projeto.

---
# Padronizações

### Partir do mínimo possível

É fato que existem pontos especialmente complexos, entretanto, devemos utilizar a muleta e caminhar até que
possamos correr, vamos criar versões mais simples e desenvolvê-las.

Seguindo a [Wiki](https://gitlab.com/robocup-sim/SimSpark/-/wikis/home) do Simspark, dentre outras possibilidades, forçaremos nosso caminho desde a base.
Dentro da seção de Issues descreveremos as linhas de avanços alcançadas.

### Debugação

Percebi que para ter acesso mais profundo a como nosso código está funcionando, é essencial que utilizemos
um debugger de verdade e não mais `print` como um amador. 

⚠️ ️Sendo assim, deixo registrado a necessidade de utilização do `debugger python` e `gdb`.

### Utilização de Heap

Sabe-se que utilizar o Heap pode ser perigoso no contexto de velocidade. Entretanto, tenha em
mente que, ao alocar o máximo possível de memória que será utilizada logo no período de inicialização
dos agentes, não teremos nenhuma desvantagem associada ao Heap.

---
# Linhas de Desenvolvimento

Ao chegarmos em um ponto mínimo, como inicializar e alocar os robôs em campo,
nos deparamos uma nova barreira: Como o robô deve **interpretar** e **interagir** sobre o mundo ao redor?

À luz dessas considerações, dividiremos o projeto em duas branches focadas respectivamente nessas duas linhas.

### Como robô interpreta?

Trabalharemos com:

- Visão computacional, processamento de sensor, fusão sensorial, reconhecimento de padrões
- Ruído em sensores, oclusões, iluminação variável, interpretação de contexto, ambiguidades, dados incompletos

### Como o robô interage?

Trabalharemos com:

- Planejamento de trajetória, controle motor, navegação autônoma, manipulação de objetos, tomada de decisão em tempo real
- Colisões, imprecisão mecânica, atrasos de resposta, adaptação a mudanças dinâmicas, segurança operacional

---
# Necessidades de Trabalho

A seguir, linhas de trabalho que necessitam de mais desenvolvimento e pesquisa:

### Código Geral da Equipe

Desenvolver mais o código da equipe é o principal objetivo de todo esse projeto, conforme [explicado](#linhas-de-desenvolvimento).
É interessante notar que sempre haverá algo a ser construído.

### Pesquisando Software

Dentro do código geral, utilizamos algoritmos muito específicos e intensos, como _A*_ para buscar pelos caminhos ou 
sistema de localização em campo.

Dado que avanços geralmente são possíveis, principalmente em um projeto embrião como este, torna-se necessário mais pesquisa.

Experimente iniciar com estes, mas busque por [issues](https://github.com/mathdeyvi04/SSRoboime/issues) com a tag `help wanted`.

- [Localização em Campo](https://github.com/mathdeyvi04/SSRoboime/issues/8)
- [Modelo de Ruído](https://github.com/mathdeyvi04/SSRoboime/issues/10)

### Pesquisando Ferramentas

Para auxiliar verificações interessantes no código geral, o uso de ferramentas auxiliares permite muita criatividade e conveniências
para os desenvolvedores. Por conta disso, torna-se necessário pesquisas de desenvolvimento.

Experimente iniciar com estes, mas busque por [issues](https://github.com/mathdeyvi04/SSRoboime/issues) com nome inicial de `Ferramenta Auxiliar`.

- [Ferramenta Auxiliar de Visualização Interna de Robô](https://github.com/mathdeyvi04/SSRoboime/issues/6)
- [Ferramenta Auxiliar de Formação Tática](https://github.com/mathdeyvi04/SSRoboime/issues/3)

