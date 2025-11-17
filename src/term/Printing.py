"""
@file Printing.py
@brief Implementação de Interface no terminal
"""
from rich.console import Console, ConsoleRenderable
from rich.table import Table
from rich import box

from select import select
import sys, tty, termios
from typing import Callable

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
    CONSOLE = Console()

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

    @staticmethod
    def print_table(
        columns: list[str],
        dados: list[list],
        # Diversas personalizações
        header_style: str = "bold",
        row_style: dict[int, str] = None,
        width: int = None,
        column_styles: dict[str, str] = None,
        column_justify: dict[str, str] = None,
        column_widths: dict[str, int] = None,
        renderable: bool = False
    ) -> None | ConsoleRenderable:
        """
        @brief Apresentará uma tabela completamente personalizada
        @param columns Lista dos nomes das colunas
        @param data Lista de listas com os valores de linhas
        @details
        Assume os seguintes parâmetros de personalização:
            columns: Lista de nomes das colunas
            data: Lista de listas com dados das linhas
            header_style: Estilo do cabeçalho
            row_styles: Estilos alternados para linhas
            width: Largura fixa da tabela
            column_styles: {nome_coluna: estilo}
            column_justify: {nome_coluna: "left"/"center"/"right"}
            column_widths: {nome_coluna: largura}
        """

        row_style = row_style or {}
        column_styles = column_styles or {}
        column_justify = column_justify or {}
        column_widths = column_widths or {}

        table = Table(
            box=box.ROUNDED,
            header_style=header_style,
            width=width,
            show_lines=True
        )

        for col in columns:
            # noinspection PyTypeChecker
            table.add_column(
                col,
                style=column_styles.get(col, ""),
                justify=column_justify.get(col, "default"),
                width=column_widths.get(col, None)
            )

        for i, row in enumerate(dados):
            table.add_row(*[str(item) for item in row], style=row_style.get(i, ""))

        return table if renderable else Printing.CONSOLE.print(table)

    @staticmethod
    def get_input(
        bytes_to_be_read: int,
        return_type: Callable = str
    ):
        """
        @brief Função complexa que fará leitura de entrada do usuário
        @details
        Tome cuidado com a execução dessa função, pois ela é poderosa
        @param return_type Tipo de entrada a ser retornado
        @param bytes_to_be_read Quantidade de Bytes que serão lidos
        @return Entrada do usuário
        """

        # Obtém o File Descriptor do stdin
        fd = sys.stdin.fileno()

        # Guarda modo original (echo, buffering, etc) para restaurar depois
        old_settings = termios.tcgetattr(fd)

        buffer = ""

        try:
            # - Desativa buffering de linha (não espera Enter)
            # - Desativa echo (não mostra teclas na tela)
            # - Desativa processamento de caracteres especiais (Ctrl+C, etc)
            # - Captura teclas imediatamente
            tty.setraw(fd)

            while len(buffer) < bytes_to_be_read:
                # Verifica se há input disponível (não-bloqueante)
                if select([sys.stdin], [], [], 0.5)[0]:
                    # Adicionamos cada caractere
                    buffer += sys.stdin.read(1)
                    if buffer[-1] in {'\r', '\n'}:
                        break
        finally:
            # Restaura configurações originais do terminal
            # Garante que o terminal volta ao normal mesmo com erros
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)

        try:
            return return_type(buffer)
        except (ValueError, TypeError):
            Printing.print_message("Erro de entrada!", "error")
            return None









