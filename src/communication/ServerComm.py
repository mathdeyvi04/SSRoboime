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

    def __init__(self, creation_options: list[list[str]]):
        """
        @brief Construtor da classe, inicializando buffers e a conexão de cada agente com servidor.
        """

        # Características da comunicação
        self.BUFFER_SIZE = 8192
        self.BUFFER = bytearray(self.BUFFER_SIZE)
        self.socket = socket.socket(
                                    socket.AF_INET,
                                    socket.SOCK_STREAM # TCP
                                    )

        # Características alheias
        self.num = creation_options[4][1]


        # Fazemos a conexão com servidor
        Printing.print_message(f"Tentando conexão do jogador {self.num}", "info")
        while True:
            try:

                self.socket.connect(
                                   (
                                    creation_options[0][1],# Host
                                    int(creation_options[1][1]) # Porta de Agentes
                                   )
                                   )
                break
            except ConnectionRefusedError:
                sleep(1)
                Printing.print_message(".")

        Printing.print_message("Agente Conectado!", "info")

        robot_type = 1
        # Fazemos o pedido de criação de robô
        self.send_immediate(
            b"(scene rsg/agent/nao/nao_hetero.rsg " + str(robot_type).encode() + b')'
        )
        self.send_immediate(
            b"(init (unum " + str(self.num).encode() + b") (teamname " + "Roboime".encode() + b"))"
        )

        sleep(15)
        self.socket.close()


    def send_immediate(self, message: bytes) -> None:
        """
        @brief Envia uma mensagem instantânea ao servidor, verificando se a conexão continua ativa
        @details
        Coloca-se na frente uma informação de tamanho da mensagem dentro de 4 bytes.
        """

        try:
            self.socket.send(
                            len(message).to_bytes(4, byteorder="big") + message
                            )
        except BrokenPipeError:
            Printing.print_message("Error: Socket foi fechado por rcssserver3d", "error")

    def receive(self) -> None:
        """
        @brief Receberá informações diretamente do servidor.
        """

        while True:
            try:
                # Verificamos se há 4 bytes no cabeçalho e nos preparamos para ler.
                if self.socket.recv_into(
                    self.BUFFER, nbytes=4
                ) != 4:
                    raise ConnectionResetError

                # Lemos o comprimento total da mensagem
                msg_size = int.from_bytes(
                    self.BUFFER[:4], # Garantimos leitura de apenas 4 bytes
                    byteorder="big", # ordem de significativo
                    signed=False # se tem sinal
                )

                # Lemos o restante da mensagem
                if(
                    self.socket.recv_into(
                        self.BUFFER,
                        nbytes=msg_size
                    )
                ) != msg_size:
                    raise ConnectionResetError

            except ConnectionResetError:
                Printing.print_message("\nError: socket foi fechado pelo rcssserver3d.")

            if len(
                select( # Monitora sockets/arquivos para I/O
                    [self.socket], # Lista de sockets/arquivos para verificar leitura
                    [], # Lista vazia para escrita
                    [], # Lista vazia para exceções
                    0.0 # timeout zero (não bloqueante)
                )[0] # Pegamos o primeiro socket para leitura
            ) == 0: # Logo, não há dados disponíveis para leitura
                break

        # Vejamos o recebido do servidor
        print(self.BUFFER)














