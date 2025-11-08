"""
@file Booting.py
@brief Implementação do Booting do time
"""
import os
import sys
from term.Printing import Printing

class Booting:
    """
    @brief Responsável por inicializar todas as necessidades de execução do time
    @details
    Assume as seguintes responsabilidades:
        - Estabelece um arquivo de configurações default caso já não exista um.
    """

    def __init__(self):
        """
        @brief Responsável por chamar as inicializações mínimas.
        """

        self.options = Booting.get_team_params()


        if getattr(sys, 'frozen', False):
            # Então estamos executando o binário!
            # Devemos forçar que o debug seja 0.
            self.options[8][1] = '0'
            Printing.IF_IN_DEBUG = False

    @staticmethod
    def get_team_params() -> list[list[str | int]]:
        """
        @brief Verifica existência de arquivo de parâmetros de time, caso não exista, usará o default.
        @details
        Faremos em tupla para permitir uso mínimo de memória.
        @return
        """

        if os.path.exists("src/config_team_params.txt"):
            with open(
                    "src/config_team_params.txt",
                    "r"
            ) as file_team_params:
                config_team_params: list[list[str | int]] =  [
                    string_tupla.split(",") for string_tupla in file_team_params.read().split("\n")[:-1]
                ]

                for idx in range(0, len(config_team_params)):
                    # Somente o IP Server e Team Name são palavras
                    if idx not in {0, 3}:
                        config_team_params[idx][1] = int(config_team_params[idx][1])



        config_team_params = [
            ["IP Server",       "localhost"],
            ["Agent Port",      3100], # Onde nos conectaremos com rcssserver3d
            ["Monitor Port",    3200], # Onde nos conectaremos com Roboviz
            ["Team Name",       "RoboIME"],
            ["Uniform Number",  1],
            ["Robot Type",      1],
            ["Penalty Shootout", 0],
            ["MagmaFatProxy",   0],
            ["Debug Mode",      1]
        ]

        # E criamos o arquivo
        with open(
            "src/config_team_params.txt",
            "w+"
        ) as file_team_params:
            for doc, value in config_team_params:
                file_team_params.write(
                    f"{doc},{value}\n"
                )

        return config_team_params

    @staticmethod
    def cpp_builder(self):
        """
        @brief Responsável por buildar os arquivos .cpp presentes na pasta cpp.
        @return Funcionalidades C++ em condições de interoperabilidade.
        """
        # Voltaremos para esta assim que tivermos desenvolvido pelo menos uma pasta cpp
        pass


