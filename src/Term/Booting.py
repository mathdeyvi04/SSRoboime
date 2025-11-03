"""
@file Booting.py
@brief Implementação do Booting do time
"""
import os

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

        print(Booting.get_team_params())


    @staticmethod
    def get_team_params() -> tuple[tuple[str, ...], ...]:
        """
        @brief Verifica existência de arquivo de parâmetros de time, caso não exista, usará o default.
        @details
        Faremos em tupla para permitir uso mínimo de memória.
        @return
        """

        if os.path.exists("config_team_params.txt"):
            with open(
                    "config_team_params.txt",
                    "r"
            ) as file_team_params:
                return tuple(
                    tuple(
                        string_tupla.split(",")
                    ) for string_tupla in file_team_params.read().split("\n")[:-1]
                )

        config_team_params = (
            ("IP Server",       "localhost"),
            ("Agent Port",      "3100"), # Onde nos conectaremos com rcssserver3d
            ("Monitor Port",    "3200"), # Onde nos conectaremos com Roboviz
            ("Team Name",       "RoboIME"),
            ("Uniform Number",  '1'),
            ("Robot Type",      '1'),
            ("Penalty Shootout",'0'),
            ("MagmaFatProxy",   '0'),
            ("Debug Mode",      '1')
        )

        # E criamos o arquivo
        with open(
            "config_team_params.txt",
            "w+"
        ) as file_team_params:
            for doc, value in config_team_params:
                file_team_params.write(
                    f"{doc},{value}\n"
                )

        return config_team_params





