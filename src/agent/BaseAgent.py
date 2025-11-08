"""
@file BaseAgent.py
@brief Implementação da classe de jogador base, que deve ser comum a todos os agentes.
"""
from abc import ABC, abstractmethod # para conseguirmos criar classes abstratas em Python
from communication.ServerComm import ServerComm
from environment.World import World

class BaseAgent(ABC):
    """
    @brief Classe que agrupará todas as funcionalidades comuns a qualquer agente.
    """

    def __init__(self, creation_options: list[list[str]]):
        """
        @brief Construtor da classe base de agente, chamando todos os construtores de outras
        classes mínimas para cada agente.
        @param creation_options Lista de Parâmetros de Criação de Agente
        """
        self.world = World(creation_options)
        self.scom = ServerComm(creation_options)
        # Chamaremos os construtores mínimos conforme formos criando-os


