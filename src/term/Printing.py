"""
@file Printing.py
@brief Implementação de Interface no terminal
"""

class Printing:
    """
    @brief Responsável pela comunicação usuário - terminal
    """
    IF_IN_DEBUG = True
    TABLE_COLORS = {
        "info": "\033[1;36m",
        "warning": "\033[1;33m",
        "error": "\033[1;31m"
    }

    @staticmethod
    def print_message(message: str, role: str=None) -> None:
        """
        @brief Apresentará uma mensagem estilizada de forma específica
        @param message Mensagem a ser apresentada
        @param role String indicando qual o motivo da mensagem
        @details
        Há uma quantidade específica de roles possíveis:
            - info
            - warning
            - error
        Caso nenhuma dessas seja inserida, há a possibilidade de inserir
        o comando ASCII de uma vez.
        """

        if not Printing.IF_IN_DEBUG:
            return

        if role is None:
            print(message, end="", flush=True)
            return

        if role in Printing.TABLE_COLORS:
            print(f"{Printing.TABLE_COLORS[role]}", end="", flush=True)
        else:
            if role.startswith("\\033["):
                print(f"{role}", end="", flush=True)
            else:
                Printing.print_message("Erro: `role` não específicada.", "error")
                return

        print(message, end="", flush=True)
        print("\033[0m", flush=True, end="")













