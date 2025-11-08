"""
@file Agent.py
@brief Implementação de Lógica de Agente de Campo
"""
from agent.BaseAgent import BaseAgent

class Agent(BaseAgent):
    """
    @brief Classe que representará os agentes de campo, possuindo métodos correspondentes.
    """
    def __init__(self, creation_options: list[list[str]]):
        """
        @brief Construtor da classe agente de campo, inicializando informações gerais.
        @param creation_options Lista de Parâmetros de Criação de Agente
        @details
        Parâmetros presentes em `creation_options`:
            - IP Server
            - Porta de Agente
            - Porta de Monitor
            - Nome do time
            - Número de Uniforme
            - Tipo de Robô
            - Tiro livre Penâlti
            - Proxy
            - Modo de Debug
        """

        super().__init__(creation_options)