"""
@file World.py
@brief Implementação da Lógica de interpretação do Robô com o mundo ao seu redor
"""

from environment.Robot import Robot

class World:
    """
    @brief Responsável por agrupar o conjunto de lógicas de assimilação.
    """

    ### Atributos Inerentes ao Mundo
    STEPTIME = 0.02  # Fixed step time
    STEPTIME_MS = 20  # Fixed step time in milliseconds
    VISUALSTEP = 0.04  # Fixed visual step time
    VISUALSTEP_MS = 40  # Fixed visual step time in milliseconds

    # Modos de Jogo a favor do nosso time
    M_OUR_KICKOFF = 0
    M_OUR_KICK_IN = 1
    M_OUR_CORNER_KICK = 2
    M_OUR_GOAL_KICK = 3
    M_OUR_FREE_KICK = 4
    M_OUR_PASS = 5
    M_OUR_DIR_FREE_KICK = 6
    M_OUR_GOAL = 7
    M_OUR_OFFSIDE = 8

    # Modos de jogo a favor deles
    M_THEIR_KICKOFF = 9
    M_THEIR_KICK_IN = 10
    M_THEIR_CORNER_KICK = 11
    M_THEIR_GOAL_KICK = 12
    M_THEIR_FREE_KICK = 13
    M_THEIR_PASS = 14
    M_THEIR_DIR_FREE_KICK = 15
    M_THEIR_GOAL = 16
    M_THEIR_OFFSIDE = 17

    # Modos de jogo neutros
    M_BEFORE_KICKOFF = 18
    M_GAME_OVER = 19
    M_PLAY_ON = 20

    # Modos de jogo de grupo
    MG_OUR_KICK = 0
    MG_THEIR_KICK = 1
    MG_ACTIVE_BEAM = 2
    MG_PASSIVE_BEAM = 3
    MG_OTHER = 4  # play on, game over

    # Posições de Pontos Específicos no Jogo
    FLAGS_CORNERS_POS = ((-15, -10, 0), (-15, +10, 0), (+15, -10, 0), (+15, +10, 0))
    FLAGS_POSTS_POS = ((-15, -1.05, 0.8), (-15, +1.05, 0.8), (+15, -1.05, 0.8), (+15, +1.05, 0.8))

    def __init__(self, creation_options: list[list[str]]):
        """
        @brief Construtor de Classe inicializando sensores interpretativos
        """

        # Atributos Inerentes à interpretação do robô para com o mundo

        # Há muitas definições aqui, como time_server, time_game

        # Entretanto, há peças importantes

        self.robot = Robot(
                          creation_options[4][1], # Uniforme do Robô
                          creation_options[5][1]  # Tipo do robô
                          )












