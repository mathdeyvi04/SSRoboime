"""
@file BaseAgent.py
@brief Implementação da classe de jogador base, que deve ser comum a todos os agentes.
"""
from abc import ABC  # para conseguirmos criar classes abstratas em Python
from communication.ServerComm import ServerComm
from pathlib import Path
import pickle

class BaseAgent(ABC):
    """
    @brief Classe que agrupará todas as funcionalidades comuns a qualquer agente.
    """

    AGENTS_IN_THE_MATCH = []
    INITIAL_POSITION = []

    def __init__(self, creation_options: list[list[str | int]]):
        """
        @brief Construtor da classe base de agente, chamando todos os construtores de outras
        classes mínimas para cada agente.
        @param creation_options Lista de Parâmetros de Criação de Agente
        """

        # --- Importações do C++ ---
        from cpp.logger.logger import Logger
        from cpp.environment.environment import Environment

        self.logger = Logger.get() # Todos os jogadores utilizarão o único
        self.env = Environment(self.logger)
        self.scom = ServerComm(
            creation_options,
            self.env,
            # Passamos o ponteiro da lista de jogadores
            # Conforme eles são inseridos, teremos novos na partida
            BaseAgent.AGENTS_IN_THE_MATCH
        )
        # Chamaremos os construtores mínimos conforme formos criando-os

        self.unum = creation_options[4][1]
        # Note que colocamos apenas por último
        BaseAgent.AGENTS_IN_THE_MATCH.append(self)

        # Garantimos que as posições são existentes
        # E executamos apenas uma vez
        if not BaseAgent.INITIAL_POSITION:
            with open(
                    Path(__file__).resolve().parent / "tactical_formation.pkl",
                    "rb"
            ) as f:
                BaseAgent.INITIAL_POSITION = pickle.load(f)["default"]

        self.init_position = BaseAgent.INITIAL_POSITION[self.unum - 1]

    def beam(self) -> None:
        """
        @brief Responsável por gerenciar o teletransporte dos jogadores
        """

        self.scom.commit_beam(self.init_position, 0)


