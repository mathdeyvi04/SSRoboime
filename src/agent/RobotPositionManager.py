"""
@file RobotPositionManager.py
@brief Implementação de lógica organizadora de posições iniciais de partida.
"""
import os
import pickle

from rich.console import ConsoleRenderable
from term.Printing import Printing
from rich.live import Live

class RobotPositionManager:
    """
    @brief Responsável por permitir ao usuário a criação de diversas configurações
    de posições iniciais de partida.
    @details
    Focada em diversão e customização, gerencia um binário que é a representação de
    dicionário de listas que contém as 11 posições.
    Por ter esse objetivo, não faz sentido que haja essa função na lógica geral dos agentes.
    """

    CONFIG_POSITION_PATH = "./config_positions.pkl"


    def __init__(self):
        """
        @brief Construtor da Classe, inicializa variáveis importantes, como o próprio dicionário.
        """

        # Configurações já existentes
        self.config_positions = RobotPositionManager.get_config_positions()

        # 0 - Tabela de Configurações Existentes
        # 1 - Mapa de Posição
        self.select = False

        # Para percorremos a tabela
        self.idx = 0

        # Para percorrermos o mapa
        self.x = 0
        self.y = 0

    @staticmethod
    def get_config_positions() -> dict[str, list[tuple]]:
        """
        @brief Verificará existência do arquivo binário correspondente ao dicionário.
        @return Caso exista, o retornará restaurado. Caso não, retornará um dicionário vazio.
        """

        if os.path.exists(RobotPositionManager.CONFIG_POSITION_PATH):
            # Caso exista, então devemos apenas restaurar
            with open(RobotPositionManager.CONFIG_POSITION_PATH, "rb") as f:
                return pickle.load(f)

        # Logo, não existe
        return {"default": [(1, 2), (2, -3), (5, 4), (2, 2)], "default_1": [(1, 2), (2, 3), (5, 4), (2, 2)]}

    def generate_table(self, renderable=False) -> ConsoleRenderable:
        """
        @brief Apresentará a tabela de configurações posicionais existentes
        """

        return Printing.print_table(
            columns=["Config", "Valores"],
            dados=[
                [
                    key,
                    value
                ] for key, value in self.config_positions.items()
            ],
            row_style={
                self.idx: "bright_cyan"
            },
            renderable=renderable
        )

    def print_grid(self) -> None:
        """
        @brief Apresentaremos a grid da configuração existente
        """

        locations = self.config_positions[list(self.config_positions.keys())[self.idx]]
        decay = 0.5

        x = -15.0
        y = 10.0
        while y != (-10 - decay):

            while x != (15 + decay):

                # Devemos verificar se estamos no mesmo x e mesmo y
                if (x, y) in locations:
                    print("\033[1;45m[X]\033[0m", end="")
                else:
                    if x == 0:
                        print("\033[1;37m[0]\033[0m", end="")
                    elif y == 0:
                        print("\033[1;37m[0]\033[0m", end="")
                    else:
                        print("\033[1;32m[0]\033[0m", end="")
                x = round(x + decay, 2)

            print()
            x = -15.0
            y = round(y - decay, 2)



    def mainloop(self) -> None:
        """
        @brief Apresentará interface
        """

        os.system("clear")
        with Live(refresh_per_second=10, transient=True) as live:
            while True:
                if not self.select:
                    self.generate_table()
                else:
                    self.print_grid()

                key = Printing.get_input(1)

                if key == 's':
                    if not self.select:
                        self.idx = self.idx + 1 if self.idx < len(self.config_positions) - 1 else 0

                elif key == 'w':
                    if not self.select:
                        self.idx = self.idx - 1 if self.idx > 0 else len(self.config_positions) - 1

                elif key == ' ':
                    # Então devemos entrar nessa configuração
                    self.select = not self.select
                    self.print_grid()




                elif key == 'q':
                    break

                os.system("clear")


