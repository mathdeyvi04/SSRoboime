"""
@brief Implementação de Classe que nos permitirá ter a visão do robô
"""
import pygame
from time import perf_counter
from math import sin, cos, radians, tan

WIDTH, HEIGHT = 1200, 1000

class Elemento:

    def __init__(self):
        self.color = None
        self.width, self.height = WIDTH, HEIGHT  # Deve-se alterar no tamanho da imagem também
        self.fov_h, self.fov_v = radians(120), radians(120)

    def projection_to_2d(self):
        pass

    def draw(self, window: pygame.Surface, font=None):
        pass

    def project_point(self, triple_numbers: list[float], forward_axis="+x", flip_x=True, flip_y=False) -> list[float]:
        ah = radians(triple_numbers[1])
        av = radians(triple_numbers[2])

        x = triple_numbers[0] * cos(av) * cos(ah)
        y = triple_numbers[0] * cos(av) * sin(ah)
        z = triple_numbers[0] * sin(av)

        if forward_axis == '+x':
            depth = x
            cx = y
            cy = z
        elif forward_axis == '-x':
            depth = -x
            cx = -y
            cy = z
        elif forward_axis == '+z':
            depth = z
            cx = x
            cy = y
        elif forward_axis == '-z':
            depth = -z
            cx = x
            cy = -y
        else:
            # default
            depth = x
            cx = y
            cy = z

        # Vamos definir x como profundidade?

        fx = (self.width / 2) / tan(self.fov_h / 2)
        fy = (self.height / 2) / tan(self.fov_v / 2)

        x_ndc = (cx / depth) * (-1 if flip_x else 1)  # NDC horizontal (negativo corrige espelho)
        y_ndc = (cy / depth) * (-1 if flip_y else 1)  # NDC vertical

        u = fx * x_ndc + self.width / 2
        v = - fy * y_ndc + self.height / 2

        scale = 0.5 * fx / depth

        return [u, v, scale]

class Ball(Elemento):

    def __init__(self, position: list[float]) -> None:
        """
        @brief Construtor responsável por inicializara bola no e prover
        """

        super().__init__()

        self.color = (0, 255, 0)
        self.position_on_sphere = position
        self.position_on_window = self.projection_to_2d()

    def projection_to_2d(self) -> list[float]:
        # Será apenas a projeção de um ponto

        return self.project_point(self.position_on_sphere)

    def draw(self, window, font=None) -> None:
        pygame.draw.circle(
            window,
            self.color,
            self.position_on_window[:-1],
           10
        )

class Marker(Elemento):
    def __init__(self, position: list[float]) -> None:
        """
        @brief Construtor responsável por inicializara bola no e prover
        """

        super().__init__()

        self.color = (255, 0, 0)
        self.position_on_sphere = position
        self.position_on_window = self.projection_to_2d()

    def projection_to_2d(self) -> list[float]:
        # Será apenas a projeção de um ponto

        return self.project_point(self.position_on_sphere)

    def draw(self, window, font=None) -> None:

        pygame.draw.circle(
            window,
            self.color,
            self.position_on_window[:-1],
            self.position_on_window[-1]
        )

class Goal(Elemento):
    def __init__(self, position: list[float]) -> None:
        """
        @brief Construtor responsável por inicializara bola no e prover
        """

        super().__init__()

        self.color = (0, 0, 255)
        self.position_on_sphere = position
        self.position_on_window = self.projection_to_2d()

    def projection_to_2d(self) -> list[float]:
        # Será apenas a projeção de um ponto

        return self.project_point(self.position_on_sphere)

    def draw(self, window, font=None) -> None:

        pygame.draw.circle(
            window,
            self.color,
            self.position_on_window[:-1],
            self.position_on_window[-1]
        )

class Line(Elemento):

    def __init__(self, double_list_position):

        super().__init__()
        self.color = (255, 255, 255)
        self.position_on_sphere = double_list_position
        self.position_on_window = self.projection_to_2d()

    def projection_to_2d(self) -> list[float]:

        return self.project_point(self.position_on_sphere[:3]) + self.project_point(self.position_on_sphere[3:])

    def draw(self, window, font=None) -> None:
        pygame.draw.line(
            window,
            self.color,
            self.position_on_window[:2],
            self.position_on_window[3:5],
            2
        )



class RobotVision:
    """
    @brief Classe responsável por gerir a aplicação principal.
    """

    FRAMES_VISION_PATH = "frames_vision.txt"

    def __init__(self):
        self.frames = None
        self.current_index = 1
        self.need_to_update = True
        self.objects = []

    def load_frames_from_file(self) -> None:
        """
        @brief Abrirá um arquivo que conterá os frames recebidos pelo agente.
        @details
        Pode ser aprimorada para permitir observação em tempo real.
        """
        with open(RobotVision.FRAMES_VISION_PATH, "r") as f:
            self.frames = [line for line in f]

    def _get_only_tag_See(self) -> str | None:
        """
        @brief Buscará no frame principal o bloco referente ao conjunto See.
        @return String referente ao bloco See. None caso não exista.
        """

        # 1. Definir o marcador de início
        start_marker = "(See"
        start_index = self.frames[self.current_index].find(start_marker)

        # Se não houver informação visual (ex: robô cego ou mensagem de status puro)
        if start_index == -1:
            return None

        # 2. Lógica de counter_entry de parênteses
        balance = 0
        end_index = -1

        # Iteramos a partir do início do bloco See
        for i in range(start_index, len(self.frames[self.current_index])):
            char = self.frames[self.current_index][i]

            if char == '(':
                balance += 1
            elif char == ')':
                balance -= 1

            # 3. Verificação de saída
            # Se o balance voltou a zero, fechamos o bloco do (See ...)
            if balance == 0:
                end_index = i
                break

        # 4. Retornar a substring exata
        # Adicionamos +1 no end_index pois o slice em Python é exclusivo no final
        if end_index != -1:
            return self.frames[self.current_index][start_index: end_index + 1]

        return None

    def parse_frame(self) -> None:
        """
        @brief Interpreta uma mensagem 'See' do Simspark/Rcssserver3d.
        @details
        Divide a responsabilidade com subfunções.
        """

        if not self.need_to_update:
            return None

        self.need_to_update = False
        inicio_de_interpretacao = perf_counter()

        # ---------------------------------------

        self.objects.clear()
        chunk_see = self._get_only_tag_See()

        # Vamos iterar sobre essa string a fim de construirmos os elementos visuais do nosso frame.

        # Remove o "(See " inicial e o ")" final para iterar no conteúdo
        chunk_see = chunk_see[5:-1]

        counter_entry = 0
        buffer_objeto = ""
        for char in chunk_see:
            # 1. Controle de aninhamento
            if char == '(':
                if counter_entry == 0:
                    buffer_objeto = ""  # Limpa buffer para novo objeto pai
                counter_entry += 1

            # 2. Acumula caracteres se estivermos dentro de um objeto
            if counter_entry > 0:
                buffer_objeto += char

            # 3. Fechamento de nível
            if char == ')':
                counter_entry -= 1

                # Se counter_entry zerou, temos um objeto completo (ex: "(B (pol ...))")
                if counter_entry == 0:

                    # -- Aqui manteremos nossa lógica separada para cada flag

                    match buffer_objeto[1]:
                        case 'B':
                            # Receberemos apenas a posição dela
                            self.objects.append(
                                Ball(
                                    list(
                                        map(
                                            float,
                                            buffer_objeto.split("pol")[1].replace(")", "").split()
                                        )
                                    )
                                )
                            )

                        case 'L':
                            buffer_objeto = buffer_objeto.replace("(", "").replace(")", "").split()
                            buffer_objeto.remove('L')
                            buffer_objeto.remove('pol')
                            buffer_objeto.remove('pol')
                            self.objects.append(
                                Line(
                                    list(
                                        map(
                                            float,
                                            buffer_objeto
                                        )
                                    )
                                )
                            )

                        case 'F':
                            self.objects.append(
                                Marker(
                                    list(
                                        map(
                                            float,
                                            buffer_objeto.split("pol")[1].replace(")", "").split()
                                        )
                                    )
                                )
                            )

                        case 'G':
                            self.objects.append(
                                Goal(
                                    list(
                                        map(
                                            float,
                                            buffer_objeto.split("pol")[1].replace(")", "").split()
                                        )
                                    )
                                )
                            )

                        case _:
                            pass

        # ---------------------------------------

        final_de_interpretacao = perf_counter()
        print(f"Tempo Total de Interpretação: {final_de_interpretacao - inicio_de_interpretacao}")
        return None

    @staticmethod
    def draw_legend(screen, items, font, padding=10, line_height=20):
        """
        @brief Desenhará a legenda das cores
        """
        x = padding
        y = screen.get_height() - padding - line_height * len(items)

        for name, color in items:
            # desenha quadradinho da cor
            pygame.draw.rect(screen, color, (x, y + 4, 12, 12))

            # escreve texto
            text = font.render(name, True, (255, 255, 255))
            screen.blit(text, (x + 20, y))

            y += line_height

    def mainloop(self) -> None:

        self.load_frames_from_file()

        pygame.init()
        screen = pygame.display.set_mode((WIDTH, HEIGHT))
        pygame.display.set_caption("RobotVision")

        font = pygame.font.SysFont(None, 25)
        clock = pygame.time.Clock()

        legenda_dos_elementos = [
            ("Linha de Campo", (255, 255, 255)),
            ("Bola", (0, 255, 0)),
            ("Bandeira de Canto", (255, 0, 0)),
            ("Trave de Gol", (0, 0, 255))
        ]

        running = True
        holding = 0
        move_delay = 150
        last_move_time = 0
        while running:
            current_time = pygame.time.get_ticks()
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False

                if event.type == pygame.KEYDOWN:
                    if event.key in (pygame.K_RIGHT, pygame.K_d):
                        holding = 1
                        last_move_time = 0

                    if event.key in (pygame.K_LEFT, pygame.K_a):
                        holding = -1
                        last_move_time = 0


                if event.type == pygame.KEYUP:
                    if event.key in (pygame.K_RIGHT, pygame.K_d, pygame.K_LEFT, pygame.K_a):
                        holding = 0

            if holding != 0:
                if current_time - last_move_time > move_delay:
                    # Atualiza o índice
                    self.current_index = (self.current_index + holding) % len(self.frames)
                    self.need_to_update = True

                    # Reseta o cronômetro para o tempo atual
                    last_move_time = current_time

            screen.fill((0, 0, 0))

            # --- Implementação de Lógicas

            self.parse_frame()

            if self.frames:
                label = f"Frame {self.current_index + 1} / {len(self.frames)}"
                text = font.render(label, True, (255, 255, 255))
                screen.blit(text, (WIDTH - text.get_width() - 10, 10))


            for obj in self.objects:
                obj: Elemento
                obj.draw(screen)

            self.draw_legend(screen, legenda_dos_elementos, font)


            pygame.display.flip()
            clock.tick(30)

        pygame.quit()








if __name__ == '__main__':
    RobotVision().mainloop()
