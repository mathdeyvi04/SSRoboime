"""
@file ServerComm.py
@brief Implementação da Comunicação com Servidor
"""
import socket
from time import sleep
from term.Printing import Printing
from select import select

class ServerComm:
    """
    @brief Responsável pela comunicação com servidor.
    """

    def __init__(self, creation_options: list[list[str]], other_players: list):
        """
        @brief Construtor da classe, inicializando buffers e a conexão de cada agente com servidor.
        @param creation_options Lista de parâmetros de criação, self ainda não foi incluído na lista.
        """

        # Características da comunicação
        self.buffer_size = 4000 # Posteriormente, devemos analisar se realmente vale a pena ter um buffer com este comprimento
        self.buffer = bytearray(self.buffer_size)
        self.socket = socket.socket(
                                    socket.AF_INET,
                                    socket.SOCK_STREAM # TCP
                                    )
        self.socket.settimeout(2)

        # Características alheias
        self.message_queue = []
        self.unum = creation_options[4][1]

        # Fazemos a conexão com servidor
        Printing.print_message(f"Tentando conexão do jogador {self.unum}", "info")
        while True:
            try:

                self.socket.connect(
                                   (
                                    creation_options[0][1],# Host
                                    creation_options[1][1] # Porta de Agentes
                                   )
                                   )
                break
            except ConnectionRefusedError:
                sleep(1)
                Printing.print_message(".")

        Printing.print_message("\tAgente Conectado!\n", "info")

        # Fazemos o pedido de criação de robô
        self.send_immediate(
            f"(scene rsg/agent/nao/nao_hetero.rsg {creation_options[5][1]})".encode()
        )
        self.__receive_async(other_players)
        self.send_immediate(
            f"(init (unum {self.unum}) (teamname {creation_options[3][1]}))".encode()
        )
        self.__receive_async(other_players)

        # Aqui podem ser realizados testes de execução de quaisquer funções do ServerComm


        # self.close()

    # Métodos Mínimos da Classe de Comunicação com servidor
    def send_immediate(self, message: bytes) -> None:
        """
        @brief Envia uma mensagem instantânea ao servidor, verificando se a conexão continua ativa
        @param message String em forma de bytes para ser transmitida
        @details
        Coloca-se na frente uma informação de tamanho da mensagem dentro de 4 bytes.
        """

        try:
            self.socket.send(
                            len(message).to_bytes(4, byteorder="big") + message
                            )
        except BrokenPipeError:
            Printing.print_message("Error: socket foi fechado por rcssserver3d", "error")

    def receive(self) -> None:
        """
        @brief Receberá informações diretamente do servidor, fazendo todas as verificações necessárias.
        """

        while True:
            try:
                # Verificamos se há 4 bytes no cabeçalho e nos preparamos para ler.
                if self.socket.recv_into(
                    self.buffer, nbytes=4
                ) != 4:
                    raise ConnectionResetError

                # Lemos o comprimento total da mensagem
                msg_size = int.from_bytes(
                    self.buffer[:4], # Garantimos leitura de apenas 4 bytes
                    byteorder="big", # ordem de significativo
                    signed=False # se tem sinal
                )

                # Lemos o restante da mensagem
                if(
                    self.socket.recv_into(
                        self.buffer,
                        nbytes=msg_size
                    )
                ) != msg_size:
                    raise ConnectionResetError

            except ConnectionResetError:
                Printing.print_message("\nError: socket foi fechado pelo rcssserver3d.", "error")
                exit()

            except TimeoutError:
                pass

            if len(
                select( # Monitora sockets/arquivos para I/O
                    [self.socket], # Lista de sockets/arquivos para verificar leitura
                    [], # Lista vazia para escrita
                    [], # Lista vazia para exceções
                    0.0 # timeout zero (não bloqueante)
                )[0] # Pegamos o primeiro socket para leitura
            ) == 0: # Logo, não há dados disponíveis para leitura
                break

            # Como há algo para ser lido, devemos aplicar o parser
        print(self.buffer)

    def __receive_async(self, other_players: list) -> None:
        """
        @brief Responsável por esperar resposta do servidor de forma assíncrona, sem impedir fluxo de execução
        @details
        Essa função foi criada com o único próposito de impedir que a espera por resposta
        do servidor interrompa o fluxo de execução. Não deve ser executada posteriormente.
        @param other_players Lista de jogadores de mesmo time presentes na partida
        """


        # Caso não haja ninguém além dele
        if not other_players:
            # Sem isso, um loop infinito existiria
            return self.receive()

        # Desabilitamos o bloqueio do fluxo de execução por espera de dados no socket
        self.socket.setblocking(False)

        while True:
            try:
                Printing.print_message(".")
                self.receive()
                break
            except BlockingIOError:
                pass

            # Força que todos estejam em condições
            for p in other_players:
                p.scom.send_immediate(b"(syn)")

        # Voltamos ao padrão
        self.socket.setblocking(True)
        Printing.print_message(f"Jogador {self.unum} recebeu do servidor assincronamente\n", "info")

        return None

    def commit(self, message: bytes) -> None:
        """
        @brief Responsável por adicionar uma nova mensagem à fila de mensagens
        @param message String em bytes a ser adicionada à fila
        """
        assert isinstance(message, bytes), "Mensagem deve estar em bytes"
        self.message_queue.append(message)

    def close(self) -> None:
        """
        @brief Responsável por fazer o encerramento dos canais de comunicação
        """

        self.socket.close()

    def send(self) -> None:
        """
        @brief Enviará ao servidor todas as mensagens commitadas.
        """
        if len(
            select(
                [self.socket],
                [],
                [],
                0.0
            )[0]
        ) == 0:
            # Se não há nenhum socket para ler neste momento, enviarei
            self.message_queue.append(b"(syn)")
            self.send_immediate(b''.join(self.message_queue))
        else:
            Printing.print_message("Houve sockets de leitura disponíveis enquanto estava enviando a fila de mensagens commitadas.", "warning")
        self.message_queue.clear() # Limpamos buffer

    def clear_queue(self) -> None:
        """
        @brief Limpará a fila de commits.
        """
        self.message_queue.clear() # Assim usamos o mesmo ponteiro

    # Métodos Derivados
    def commit_beam(self, vector_position2d: list, rotation: float):
        """
        @brief Comando de beam oficial do agente
        @param vector_position2d Sequência de dois valores, x e y finais do agente
        @param rotation Valor de rotação a ser dado ao robô
        """
        assert len(vector_position2d) == 2, "O beam oficial permite apenas posições 2D."
        self.commit(
            f"(beam {vector_position2d[0]} {vector_position2d[1]} {rotation})".encode()
        )












