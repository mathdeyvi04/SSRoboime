"""
@file Agent.py
@brief Implementação de Lógica de Agente de Campo
"""
from agent.BaseAgent import BaseAgent

class Agent(BaseAgent):
    """
    @brief Classe que representará os agentes de campo, possuindo métodos correspondentes.
    """
    def __init__(self, creation_options: list[list[str | int]]):
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

        creation_options[5][1] = (0,1,1,1,2,3,3,3,4,4,4)[creation_options[4][1] - 1]

        super().__init__(creation_options)