"""
@file Robot.py
@brief Implementação de Classe representadora do robô
"""
import numpy as np

class Robot:
    """
    @brief Classe que representará o robô e todos seus atributos inerentes à sua existência.
    """

    # Atributos Comuns a cada Robô
    STEPTIME = 0.02  # Fixed step time
    VISUALSTEP = 0.04  # Fixed visual step time
    SQ_STEPTIME = STEPTIME * STEPTIME
    GRAVITY = np.array([0, 0, -9.81])
    IMU_DECAY = 0.996  # IMU's velocity decay

    # Mapa de juntas
    MAP_PERCEPTOR_TO_INDEX = {"hj1": 0, "hj2": 1, "llj1": 2, "rlj1": 3,
                              "llj2": 4, "rlj2": 5, "llj3": 6, "rlj3": 7,
                              "llj4": 8, "rlj4": 9, "llj5": 10, "rlj5": 11,
                              "llj6": 12, "rlj6": 13, "laj1": 14, "raj1": 15,
                              "laj2": 16, "raj2": 17, "laj3": 18, "raj3": 19,
                              "laj4": 20, "raj4": 21, "llj7": 22, "rlj7": 23}

    # Fix symmetry issues 1a/4 (identification)
    FIX_PERCEPTOR_SET = {'rlj2', 'rlj6', 'raj2', 'laj3', 'laj4'}
    FIX_INDICES_LIST = [5, 13, 17, 18, 20]

    # Alturas recomendadas para unofficial beams
    # os quais são próximos do chão
    BEAM_HEIGHTS = [0.4, 0.43, 0.4, 0.46, 0.4]

    def __init__(self, unum: int, robot_type: int) -> None:
        """
        @brief Construtor de classe inicializando todos os atributos individuais de cada robô
        """

        # Atributos Individuais de cada robô






