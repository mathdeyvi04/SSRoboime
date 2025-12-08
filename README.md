# Sumário

- [Descrição](#descrição)
- [Padronizações](#padronizações)
  - [Construção da Base](#partir-do-mínimo-possível)
  - [Debugação](#debugação)
- [Linhas de Desenvolvimento](#linhas-de-desenvolvimento)

# Descrição

Neste repositório, marcharemos no progresso de produzir um código proprietário da RoboIME 
para a categoria do **Simulation Soccer**.

Utilizaremos como muleta o repositório da equipe [FCPortugal](https://github.com/m-abr/FCPCodebase) e pouco a pouco construiremos o nosso. Observe que este é o pontapé inicial para anos de pesquisa e avanço.

# Padronizações

### Partir do mínimo possível

É fato que existem pontos especialmente complexos, entretanto, devemos utilizar a muleta e caminhar até que
possamos correr, vamos criar versões mais simples e desenvolvê-las.

Seguindo a [Wiki](https://gitlab.com/robocup-sim/SimSpark/-/wikis/home) do Simspark, dentre outras possibilidades, forçaremos nosso caminho desde a base.
Dentro da seção de Issues descreveremos as linhas de avanços alcançadas.

### Debugação

Percebi que para ter acesso mais profundo a como nosso código está funcionando, é essencial que utilizemos
um debugger de verdade e não mais `print` como um amador. 

Sendo assim, deixo registrado a necessidade de utilização do `debugger python` e `gdb`.

### Utilização de Heap

Sabe-se que utilizar o Heap pode ser perigoso no contexto de velocidade. Entretanto, tenha em
mente que, ao alocar o máximo possível de memória que será utilizada logo no período de inicialização
dos agentes, não teremos nenhuma desvantagem associada ao Heap.

# Linhas de Desenvolvimento

Ao chegarmos no ponto de mínimo, afinal conseguimos inicializar e alocar nossos agentes em campo,
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
