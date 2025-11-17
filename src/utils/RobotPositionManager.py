"""
@file RobotPositionManager.py
@brief Implementação de lógica organizadora de posições iniciais de partida.
"""
import os
import pickle
import tkinter as tk
from tkinter import ttk, simpledialog, messagebox

class RobotPositionManager(tk.Tk):
    """
    @brief Responsável por permitir ao usuário a criação de diversas configurações
    de posições iniciais de partida.
    @details
    Focada em diversão e customização, gerencia um binário que é a representação de
    dicionário de listas que contém as 11 posições.
    Por ter esse objetivo, não faz sentido que haja essa função na lógica geral dos agentes.
    """

    CONFIG_POSITION_PATH = "../agent/config_positions.pkl"


    def __init__(self):
        """
        @brief Construtor da Classe, inicializa variáveis importantes, como o próprio dicionário.
        """
        # Iniciamos a interface
        super().__init__()
        self.title("RobotPositionManager")
        self.geometry("900x750")

        # Configurações já existentes
        self.config_positions = RobotPositionManager.get_config_positions()
        self.nome_de_config_selecionada = None

        # --- Constantes do Campo ---
        self.FIELD_WIDTH = 30
        self.FIELD_HEIGHT = 20
        self.GRID_SCALE = 25  # Pixels por unidade de campo
        self.MAX_JOGADORES = 11
        self.X_MIN = -self.FIELD_WIDTH / 2
        self.X_MAX = self.FIELD_WIDTH / 2
        self.Y_MIN = -self.FIELD_HEIGHT / 2
        self.Y_MAX = self.FIELD_HEIGHT / 2

        # Variáveis de Estado
        self.posicoes_atuais = [] # Lista de tuplas do grid atual
        self.marcadores_jogadores = [] # Lista para rastrearmos nossos jogadores

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
        @brief Verificará existência do arquivo binário correspondente ao dicionário.
        @return Caso exista, o retornará restaurado. Caso não, retornará um dicionário vazio.
        """

        if os.path.exists(RobotPositionManager.CONFIG_POSITION_PATH):
            # Caso exista, então devemos apenas restaurar
            with open(RobotPositionManager.CONFIG_POSITION_PATH, "rb") as f:
                return pickle.load(f)

        # Logo, não existe
        return {"default": [(1, 2), (2, -3), (5, 4), (2, 2)], "default_1": [(1, 2), (2, 3), (5, 4), (2, 2)]}

    @staticmethod
    def save_config_positions(dados: dict[str, list[tuple]]) -> None:
        """
        @brief Responsável por salvar uma estrutura de dados em arquivo binário
        @param dados Estrutura de dados a ser salva
        """

        with open(
            RobotPositionManager.CONFIG_POSITION_PATH,
            "wb"
        ) as f:
            # Colocamos esse comentário já que estava dando erro no interpretador da IDE
            pickle.dump(dados, f) # type: ignore

    def _field_to_canvas(self, fx_: float, fy_: float) -> tuple:
        """
        @brief Responsável por converter coordenadas do campo para pixels no canvas
        @param fx_ Coordenada real em x
        @param fy_ Coordenada real em y
        @return Coordenadas corrigidas para o grid
        """
        return (
            (fx_ - self.X_MIN) * self.GRID_SCALE,
            (self.Y_MAX - fy_) * self.GRID_SCALE
        )

    def _canvas_to_field(self, cx: int, cy: int) -> tuple:
        """
        @brief Converterá o pixel clicado para o quadrado correspondente
        @param cx Posição X do pixel
        @param cy Posição Y do pixel
        @return tupla de posições reais
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
        @brief Disporá os widgets da interface de forma inteligente, provendo informações úteis.
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
        @brief Desenharemos um jogador na posição especificada
        @param field_x Posição real em X
        @param field_y Posição real em Y
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
        @brief Responsável por identificar onde o usuário clicou e adicionar essa posição na lista
        @param event Argumento default do bind
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

    def on_double_click_in_configs(self, event: tk.Event) -> None:
        """
        @brief Responsável por plotar a configuração de jogadores selecionada
        @param event Argumento Default de bind
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
        @brief Salvará uma configuração selecionada
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
        @brief Responsável por limpar as posições e a grade
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
        @brief Prepará uma nova configuração para ser criada
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
        @brief Apagará uma configuração selecionada
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
        @brief Responsável por atualizar e preencher tabela de configurações de posição
        """
        for i in self.tv_configs.get_children():
            self.tv_configs.delete(i)

        for chave, value in self.config_positions.items():
            self.tv_configs.insert("", "end", values=(chave, value))

    # -- Métodos de Overload
    def destroy(self):
        RobotPositionManager.save_config_positions(self.config_positions)
        super().destroy()



if __name__ == '__main__':
    root = RobotPositionManager()
    root.mainloop()






