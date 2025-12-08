"""
@file RobotPositionManager.py
@brief Implementação de lógica organizadora de posições iniciais de partida.
@details
Este módulo fornece uma interface gráfica (GUI) baseada em Tkinter para gerenciar
formações táticas de robôs. Ele atua como uma ponte entre a configuração visual
e o código C++ do projeto, lendo e escrevendo diretamente em arquivos .hpp.
"""
import os
import tkinter as tk
from tkinter import ttk, simpledialog, messagebox
from pathlib import Path
import re

class RobotPositionManager(tk.Tk):
    """
    @class RobotPositionManager
    @brief Responsável por permitir ao usuário a criação e edição de diversas formações táticas.
    @details
    Esta classe gerencia um ciclo completo de leitura e escrita de arquivos de cabeçalho C++.
    Focada em experiência do usuário (UX) e customização, ela abstrai a complexidade
    de editar arrays de coordenadas manualmente no código.

    A classe interpreta o arquivo `booting_tactical_formation.hpp` como um dicionário
    de listas, onde cada chave é o nome da formação e o valor é a lista de 11 coordenadas (x, y).
    """

    CONFIG_POSITION_PATH = Path(__file__).resolve().parents[1] / "Booting" / "booting_tactical_formation.hpp"

    def __init__(self):
        """
        @brief Construtor da Classe. Inicializa a GUI, variáveis de estado e constantes do campo.
        @details
        Define as dimensões do campo de futebol simulado, escalas de conversão (pixels por metro)
        e inicializa as estruturas de dados que armazenarão as posições dos jogadores.
        Também carrega as configurações existentes do arquivo C++ ao iniciar.
        """
        # Iniciamos a interface
        super().__init__()
        self.title("RobotPositionManager")
        self.geometry("900x750")

        # Configurações já existentes
        self.config_positions = RobotPositionManager.get_config_positions()
        self.nome_de_config_selecionada = None

        # --- Constantes do Campo ---
        self.FIELD_WIDTH = 30   #: Largura total do campo em metros
        self.FIELD_HEIGHT = 20  #: Altura total do campo em metros
        self.GRID_SCALE = 25    #: Escala de conversão: Pixels por unidade de campo (metro)
        self.MAX_JOGADORES = 11 #: Limite de robôs por time
        self.X_MIN = -self.FIELD_WIDTH / 2
        self.X_MAX = self.FIELD_WIDTH / 2
        self.Y_MIN = -self.FIELD_HEIGHT / 2
        self.Y_MAX = self.FIELD_HEIGHT / 2

        # Variáveis de Estado
        self.posicoes_atuais = [] # Lista de tuplas do grid atual
        self.marcadores_jogadores = [] # Lista para rastrearmos nossos jogadores (IDs do Canvas)

        # Apenas variáveis que serão utilizadas posteriormente
        self.tv_configs = None # Para organizarmos a tabela de configurações
        self.canvas = None
        self.canvas_height = self.FIELD_HEIGHT * self.GRID_SCALE
        self.canvas_width = self.FIELD_WIDTH * self.GRID_SCALE

        # Disporemos as informações de forma inteligente
        self.criar_widgets()
        self.update_table_config()

    # -- Métodos de Ajuda
    @staticmethod
    def get_config_positions() -> dict[str, list[tuple]]:
        """
        @brief Lê e analisa o arquivo C++ para extrair as configurações de posição salvas.
        @details
        Realiza o parsing do arquivo `booting_tactical_formation.hpp`.
        Utiliza Expressões Regulares (Regex) para identificar declarações de arrays C++
        (ex: `float Nome[11][2] = { ... };`) e converte os valores textuais para
        objetos Python (listas de tuplas).

        @return dict[str, list[tuple]] Um dicionário onde as chaves são os nomes das variáveis C++
                e os valores são listas de coordenadas (x, y).
                Retorna um dicionário com valores padrão se o arquivo não existir.
        """

        if os.path.exists(RobotPositionManager.CONFIG_POSITION_PATH):
            conteudo_arquivo = None
            with open(RobotPositionManager.CONFIG_POSITION_PATH, 'r') as f:
                conteudo_arquivo = f.read()

            dados_extraidos = {}

            # 1. Regex para encontrar a declaração da variável completa
            # Procura por: float Nome[...] = { CONTEUDO };
            padrao_bloco = re.compile(
                r"float\s+(?P<nome>\w+)\[\d+]\[\d+]\s*=\s*\{(.*?)};",
                re.DOTALL
            )

            # 2. Regex para encontrar os pares de números dentro do conteúdo
            # Procura por: { numero, numero }
            padrao_linha = re.compile(
                r"\{\s*(-?[\d.]+)[fF]?\s*,\s*(-?[\d.]+)[fF]?\s*}"
            )

            # Itera sobre todas as variáveis encontradas no arquivo (caso haja mais de uma)
            for match in padrao_bloco.finditer(conteudo_arquivo):
                nome_variavel = match.group("nome")
                corpo_matriz = match.group(2)

                lista_final = []

                # Itera sobre todas as linhas {x, y} encontradas dentro da variável
                for linha_match in padrao_linha.finditer(corpo_matriz):
                    try:
                        val_x = float(linha_match.group(1))
                        val_y = float(linha_match.group(2))
                        lista_final.append([val_x, val_y])
                    except ValueError:
                        print(f"Erro ao converter valores na variável {nome_variavel}")

                dados_extraidos[nome_variavel] = lista_final

            return dados_extraidos

        # Logo, não existe
        return {"default": [(1, 2), (2, -3), (5, 4), (2, 2)], "default_1": [(1, 2), (2, 3), (5, 4), (2, 2)]}

    @staticmethod
    def save_config_positions(dados: dict[str, list[tuple]]) -> None:
        """
        @brief Persiste a estrutura de dados Python de volta para o formato de arquivo C++.
        @details
        Reescreve completamente o arquivo `booting_tactical_formation.hpp`.
        Gera o código C++ necessário, incluindo *guards* (`#pragma once`), declaração de *namespace*
        e a formatação correta dos arrays de float (adicionando o sufixo 'f' para literais float).

        @param dados Dicionário contendo as configurações de formação a serem salvas.
        """
        # Header do arquivo (Includes e início do Namespace)
        conteudo = [
            "#pragma once",
            "///< Este código somente será chamado uma vez",
            "namespace TacticalFormation {",
        ]

        for nome_variavel, matriz in dados.items():
            # Declaração da variável array
            conteudo.append(f"\tfloat {nome_variavel}[11][2] = {{")

            # Preenchimento das linhas da matriz
            for linha in matriz:
                x = linha[0]
                y = linha[1]
                # Formatação com 'f' para garantir float literal no C++ (ex: 10.5f)
                conteudo.append(f"\t\t{{{x:}f, {y}f}},")

            conteudo.append("    };")
            conteudo.append("")  # Linha em branco para separar variáveis

        # Fechamento do Namespace
        conteudo.append("};")

        # Escrita no arquivo
        with open(RobotPositionManager.CONFIG_POSITION_PATH, "w", encoding="utf-8") as f:
            f.write("\n".join(conteudo))

    def _field_to_canvas(self, fx_: float, fy_: float) -> tuple:
        """
        @brief Converte coordenadas do mundo real (metros) para coordenadas da tela (pixels).
        @details
        Aplica a escala (`GRID_SCALE`) e ajusta a origem.
        O eixo Y é invertido, pois no Canvas o (0,0) é no topo esquerdo,
        enquanto no campo o Y cresce para cima.

        @param fx_ Coordenada real em X (metros).
        @param fy_ Coordenada real em Y (metros).
        @return tuple (cx, cy) Coordenadas convertidas para o sistema do Canvas.
        """
        return (
            (fx_ - self.X_MIN) * self.GRID_SCALE,
            (self.Y_MAX - fy_) * self.GRID_SCALE
        )

    def _canvas_to_field(self, cx: int, cy: int) -> tuple:
        """
        @brief Converte coordenadas do clique (pixels) para o quadrado do grid mais próximo (metros).
        @details
        Realiza a operação inversa de `_field_to_canvas`, mas com uma etapa adicional de
        arredondamento (snap-to-grid) para passos de 0.5 metros.
        Também aplica *clamping* (limitação) para garantir que o resultado esteja dentro dos limites do campo.

        @param cx Posição X do pixel clicado.
        @param cy Posição Y do pixel clicado.
        @return tuple (fx, fy) Coordenadas reais arredondadas e limitadas ao campo.
        """

        # Converte pixel X para coordenada de campo
        fx_raw = (cx / self.GRID_SCALE) + self.X_MIN

        # Converte pixel Y para coordenada de campo (invertendo a lógica)
        fy_raw = self.Y_MAX - (cy / self.GRID_SCALE)

        # Arredonda para o 0.5 mais próximo
        fx_rounded = round(fx_raw * 2) / 2
        fy_rounded = round(fy_raw * 2) / 2

        # Garante que o clique (mesmo fora) se encaixe nos limites
        return (
            max(self.X_MIN, min(self.X_MAX, fx_rounded)),
            max(self.Y_MIN, min(self.Y_MAX, fy_rounded))
        )

    # -- Métodos de Interface
    def criar_widgets(self):
        """
        @brief Instancia e posiciona todos os elementos visuais (Widgets) da janela.
        @details
        Constrói o layout dividido em:
        1. **Frame Superior**: Contém a tabela (Treeview) de configurações salvas e os botões de ação (Novo, Salvar, Apagar, Limpar).
        2. **Frame Inferior**: Contém o Canvas que desenha o campo de futebol interativo.

        Também configura os *bindings* de eventos, como cliques simples e duplos.
        """

        upper_frame = ttk.Frame(self)
        upper_frame.pack(side="top", fill="x", padx=10, pady=10)

        config_frame = ttk.Frame(upper_frame)
        config_frame.pack(side="left", fill="both", expand=True)

        # Disporemos a tabela
        self.tv_configs = ttk.Treeview(config_frame, columns=("Nome", "Configuração"), show="headings")
        self.tv_configs.heading("Nome", text="Nome")
        self.tv_configs.heading("Configuração", text="Configuração")
        self.tv_configs.column("Nome", width=50, anchor="center")
        self.tv_configs.column("Configuração", width=250)

        self.tv_configs.pack(side="left", fill="both", expand=True)
        self.tv_configs.bind("<Double-1>", self.on_double_click_in_configs)

        frame_botoes = ttk.Frame(upper_frame)
        frame_botoes.pack(side="right", fill="y", padx=10)

        ttk.Button(frame_botoes, text="Nova Configuração", command=self.nova_config).pack(fill="x", pady=2)
        ttk.Button(frame_botoes, text="Salvar Atual", command=self.salvar_config).pack(fill="x", pady=2)
        ttk.Button(frame_botoes, text="Apagar Selecionada", command=self.apagar_config).pack(fill="x", pady=2)
        ttk.Button(frame_botoes, text="Limpar Grade", command=lambda: (self.clear_grid(), self.posicoes_atuais.clear())).pack(fill="x", pady=10)

        # ----- Focando no campo
        frame_grid = ttk.Frame(self)
        frame_grid.pack(side="top", fill="both", expand=True, padx=10, pady=10)

        # Canvas para o campo
        self.canvas = tk.Canvas(
            frame_grid,
            width=self.canvas_width,
            height=self.canvas_height,
            bg="#42f545" # Verde para o campo
        )
        self.canvas.pack()

        # Bind do clique no canvas
        self.canvas.bind("<Button-1>", self.click_on_grid)

        self.clear_grid()

    def draw_player(self, field_x, field_y) -> None:
        """
        @brief Renderiza visualmente um jogador no Canvas.
        @details
        Desenha um círculo amarelo com borda preta na posição especificada.
        Armazena o ID do objeto gráfico criado em `self.marcadores_jogadores`
        para permitir a remoção futura via clique.

        @param field_x Posição real em X (metros).
        @param field_y Posição real em Y (metros).
        """

        # Converte as coordenadas do campo (ex: -14, 0) para pixels
        cx, cy = self._field_to_canvas(field_x, field_y)

        r = self.GRID_SCALE / 3

        oval_id = self.canvas.create_oval(cx - r, cy - r, cx + r, cy + r,
                                          fill="yellow", outline="black", width=2)

        self.marcadores_jogadores.append((oval_id, (field_x, field_y)))

    # -- Métodos de Interação
    def click_on_grid(self, event: tk.Event):
        """
        @brief Callback executado ao clicar no Canvas (Campo).
        @details
        Gerencia a lógica de inserção e remoção de jogadores:
        1. Se clicar em cima de um jogador existente -> Remove-o.
        2. Se clicar em um espaço vazio -> Adiciona um jogador (se não exceder o limite `MAX_JOGADORES`).

        Utiliza `_canvas_to_field` para alinhar o clique à grade (snap).

        @param event Objeto de evento do Tkinter contendo as coordenadas x, y do clique.
        """

        new_pos = self._canvas_to_field(event.x, event.y)

        # Verificamos se clicamos em cima de um jogador
        for i, (oval_id, pos) in enumerate(self.marcadores_jogadores):
            if pos == new_pos:
                self.canvas.delete(oval_id)
                self.marcadores_jogadores.pop(i)
                self.posicoes_atuais.remove(new_pos)
                return

        # Verificamos se o limite de jogadores foi atingido
        if len(self.posicoes_atuais) >= self.MAX_JOGADORES:
            messagebox.showwarning("Limite Atingido",
                                   f"Não é possível adicionar mais de {self.MAX_JOGADORES} jogadores.\n"
                                   "Clique em um jogador existente para removê-lo.")
            return

        # Caso nenhuma das opções anteriores, adicionamos
        self.posicoes_atuais.append(new_pos)
        self.draw_player(*new_pos)

    def on_double_click_in_configs(self, _: tk.Event) -> None:
        """
        @brief Callback executado ao clicar duas vezes em uma linha da tabela de configurações.
        @details
        Carrega a formação selecionada da memória para a área de edição (Canvas).
        Limpa a grade atual e redesenha todos os jogadores da configuração escolhida.

        @param _ Evento do Tkinter (ignorado).
        """

        item_selecionado = self.tv_configs.focus()
        if not item_selecionado:
            return

        nome_config = self.tv_configs.item(item_selecionado, "values")[0]
        if nome_config in self.config_positions:
            self.posicoes_atuais = self.config_positions[nome_config][:]
            self.clear_grid()
            for (fx, fy) in self.posicoes_atuais:
                self.draw_player(fx, fy)
            self.nome_de_config_selecionada = nome_config
        else:
            messagebox.showwarning("Erro", f"Configuração '{nome_config}' não encontrada.")

    def salvar_config(self) -> None:
        """
        @brief Salva a disposição atual dos jogadores no Canvas para a configuração selecionada.
        @details
        Pede confirmação ao usuário antes de sobrescrever a configuração.
        Atualiza o dicionário `self.config_positions` e refaz a tabela visual.
        **Nota**: A gravação em disco só ocorre no encerramento do programa (`destroy`).
        """

        item_selecionado = self.tv_configs.focus()
        if not item_selecionado:
            if not self.nome_de_config_selecionada:
                messagebox.showwarning("Inválido", "Não há selecionado")
                return
            else:
                nome_config = self.nome_de_config_selecionada
        else:
            nome_config = self.tv_configs.item(item_selecionado, "values")[0]

        if messagebox.askyesno(
            "Certeza?",
            f"Realmente deseja salvar a configuração de jogadores presentes na grade em {nome_config}?"
        ):
            # Atualizaremos
            self.config_positions[nome_config] = self.posicoes_atuais.copy()
            self.update_table_config()
            for item in self.tv_configs.get_children():
                if self.tv_configs.item(item, 'values')[0] == nome_config:  # [0] = primeira coluna
                    self.tv_configs.selection_set(item)
                    self.nome_de_config_selecionada = nome_config
                    break

    def clear_grid(self) -> None:
        """
        @brief Reseta o visual do campo.
        @details
        1. Remove todos os elementos desenhados (jogadores e linhas).
        2. Limpa a lista de referências de marcadores.
        3. Redesenha as linhas do campo:
           - Grade de 0.5 em 0.5 metros.
           - Linhas principais (Eixos X e Y) em branco e mais grossas.
           - Áreas dos gols (rectângulos) e círculo central.
        """

        self.canvas.delete("all")
        self.marcadores_jogadores = []

        # Círculo central (usando a conversão de coordenadas)
        cx, cy = self._field_to_canvas(0,0)
        r = self.GRID_SCALE * 4  # Raio de 4 unidades
        self.canvas.create_oval(cx - r, cy - r, cx + r, cy + r, outline="white", width=2)

        # --- Desenhar Linhas da Grade (Quadrados) ---

        # Total de passos de 0.5
        n_steps_x = int(self.FIELD_WIDTH * 2) + 1
        n_steps_y = int(self.FIELD_HEIGHT * 2) + 1

        # Linhas Verticais (eixo X)
        for i in range(n_steps_x):
            fx = self.X_MIN + (i * 0.5)

            # --- Lógica das Cores (Req. 3) ---
            cor = "white" if fx == 0 else "#337033"
            largura = 2 if fx == 0 else 1

            # Converte a coordenada X para pixel
            cx, _ = self._field_to_canvas(fx, 0)

            # Desenha a linha (Req. 2 - todas as linhas são desenhadas)
            self.canvas.create_line(cx, 0, cx, self.canvas_height,
                                    fill=cor, width=largura)

        # Linhas Horizontais (eixo Y)
        for i in range(n_steps_y):
            fy = self.Y_MIN + (i * 0.5)

            # --- Lógica das Cores (Req. 3) ---
            cor = "white" if fy == 0 else "#337033"
            largura = 2 if fy == 0 else 1

            # Converte a coordenada Y para pixel
            _, cy = self._field_to_canvas(0, fy)

            # Desenha a linha (Req. 2)
            self.canvas.create_line(0, cy, self.canvas_width, cy,
                                    fill=cor, width=largura)

            # Caixas do Gol Esquerda (-15 a -13 em X, 3 a -3 em Y)
            coords_gol_esq = (-15, 3, -13, -3)

            # Caixas do Gol Direita (13 a 15 em X, 3 a -3 em Y)
            coords_gol_dir = (13, 3, 15, -3)

            # Converte e desenha o Gol Esquerdo
            x1, y1 = self._field_to_canvas(coords_gol_esq[0], coords_gol_esq[1])
            x2, y2 = self._field_to_canvas(coords_gol_esq[2], coords_gol_esq[3])
            self.canvas.create_rectangle(x1, y1, x2, y2, outline="white", width=3)

            # Converte e desenha o Gol Direito
            x1, y1 = self._field_to_canvas(coords_gol_dir[0], coords_gol_dir[1])
            x2, y2 = self._field_to_canvas(coords_gol_dir[2], coords_gol_dir[3])
            self.canvas.create_rectangle(x1, y1, x2, y2, outline="white", width=3)

    def nova_config(self) -> None:
        """
        @brief Cria uma nova entrada de configuração vazia.
        @details
        Solicita ao usuário um nome único para a nova formação tática.
        Se o nome for válido e não existente, inicializa uma entrada vazia no dicionário
        e atualiza a interface.
        """

        nome = simpledialog.askstring("Nova Configuração", "Digite o nome desejado:")
        if not nome:
            return

        if nome in self.config_positions:
            messagebox.showwarning("Nome Inválido", "Já há uma configuração com este nome")
            return

        # Atualizamos e setamos
        self.config_positions[nome] = []
        self.update_table_config()
        self.clear_grid()
        for item in self.tv_configs.get_children():
            if self.tv_configs.item(item, 'values')[0] == nome:  # [0] = primeira coluna
                self.tv_configs.selection_set(item)
                self.nome_de_config_selecionada = nome
                break

    def apagar_config(self) -> None:
        """
        @brief Remove permanentemente a configuração selecionada da lista.
        @details
        Pede confirmação ao usuário antes de excluir. Se confirmado, remove a chave
        do dicionário `config_positions`, limpa a grade atual e atualiza a tabela.
        """

        item_selecionado = self.tv_configs.focus()
        if not item_selecionado:
            if not self.nome_de_config_selecionada:
                messagebox.showwarning("Inválido", "Não há nada para ser adicionado")
                return
            else:
                nome_config = self.nome_de_config_selecionada
        else:
            nome_config = self.tv_configs.item(item_selecionado, "values")[0]

        if messagebox.askyesno("Confirmar", f"Tem certeza que deseja apagar a configuração '{nome_config}'?"):
            if nome_config in self.config_positions:
                self.nome_de_config_selecionada = None
                del self.config_positions[nome_config]
                self.update_table_config()
                self.clear_grid()
                self.posicoes_atuais.clear()
                messagebox.showinfo("Apagado", f"Configuração '{nome_config}' foi apagada.")

    def update_table_config(self) -> None:
        """
        @brief Atualiza os dados exibidos na Treeview (Tabela) de configurações.
        @details
        Limpa todos os itens visuais da tabela e a repovoa iterando sobre
        as chaves e valores atuais do dicionário `self.config_positions`.
        """
        for i in self.tv_configs.get_children():
            self.tv_configs.delete(i)

        for chave, value in self.config_positions.items():
            self.tv_configs.insert("", "end", values=(chave, value))

    # -- Métodos de Overload
    def destroy(self):
        """
        @brief Sobrescrita do método de destruição da janela (Ao fechar).
        @details
        Garante que as alterações feitas no dicionário `config_positions` sejam salvas
        no arquivo C++ (`.hpp`) antes de encerrar a aplicação Tkinter.
        """
        RobotPositionManager.save_config_positions(self.config_positions)
        super().destroy()

if __name__ == '__main__':
    root = RobotPositionManager()
    root.mainloop()