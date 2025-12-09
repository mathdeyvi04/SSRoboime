"""
@brief Implementação de Classe que nos permitirá ter a visão do robô em Tempo Real via Socket UNIX.
"""
import pygame
import socket
import os
import sys
from time import perf_counter
from math import sin, cos, radians, tan

# Dimensões da Janela
WIDTH, HEIGHT = 1200, 1000

class Elemento:
    """
    @brief Classe base para todos os elementos visuais da simulação.
    """

    def __init__(self):
        """
        @brief Inicializa propriedades básicas de um elemento visual.
        """
        self.color = None
        self.width, self.height = WIDTH, HEIGHT
        self.fov_h, self.fov_v = radians(120), radians(120)

    def projection_to_2d(self):
        """
        @brief Método abstrato para calcular a projeção 3D -> 2D.
        """
        pass

    def draw(self, window: pygame.Surface, font=None):
        """
        @brief Método abstrato para desenhar o objeto na tela.
        @param window Superfície do Pygame onde o desenho ocorrerá.
        @param font Fonte opcional para textos.
        """
        pass

    def project_point(self, triple_numbers: list[float], forward_axis="+x", flip_x=True, flip_y=False) -> list[float]:
        """
        @brief Realiza a projeção matemática de coordenadas polares/esféricas para o plano 2D (NDC).

        @param triple_numbers Lista contendo [distância, ângulo_horizontal, ângulo_vertical].
        @param forward_axis Eixo considerado como 'frente' na câmera.
        @param flip_x Flag para espelhar o eixo X.
        @param flip_y Flag para espelhar o eixo Y.

        @return Lista [u, v, scale] representando posições X, Y na tela e fator de escala.
        """
        ah = radians(triple_numbers[1])
        av = radians(triple_numbers[2])

        x = triple_numbers[0] * cos(av) * cos(ah)
        y = triple_numbers[0] * cos(av) * sin(ah)
        z = triple_numbers[0] * sin(av)

        if forward_axis == '+x':
            depth = x; cx = y; cy = z
        elif forward_axis == '-x':
            depth = -x; cx = -y; cy = z
        elif forward_axis == '+z':
            depth = z; cx = x; cy = y
        elif forward_axis == '-z':
            depth = -z; cx = x; cy = -y
        else:
            depth = x; cx = y; cy = z

        fx = (self.width / 2) / tan(self.fov_h / 2)
        fy = (self.height / 2) / tan(self.fov_v / 2)

        # Evita divisão por zero se depth for muito pequeno
        if depth < 0.01: depth = 0.01

        x_ndc = (cx / depth) * (-1 if flip_x else 1)
        y_ndc = (cy / depth) * (-1 if flip_y else 1)

        u = fx * x_ndc + self.width / 2
        v = - fy * y_ndc + self.height / 2

        scale = 0.5 * fx / depth

        return [u, v, scale]

class Ball(Elemento):
    """
    @brief Representação visual da bola.
    """
    def __init__(self, position: list[float]) -> None:
        """
        @brief Construtor da Bola.
        @param position Coordenadas polares da bola relativas ao agente.
        """
        super().__init__()
        self.color = (0, 255, 0)
        self.position_on_sphere = position
        self.position_on_window = self.projection_to_2d()

    def projection_to_2d(self) -> list[float]:
        """
        @brief Calcula a posição 2D da bola.
        @return Lista com coordenadas de tela.
        """
        return self.project_point(self.position_on_sphere)

    def draw(self, window, font=None) -> None:
        """
        @brief Desenha o círculo da bola na tela.
        """
        try:
            pygame.draw.circle(
                window,
                self.color,
                (int(self.position_on_window[0]), int(self.position_on_window[1])),
                10
            )
        except TypeError:
            pass # Proteção contra coordenadas inválidas

class Marker(Elemento):
    """
    @brief Representação de marcadores de campo (Flags).
    """
    def __init__(self, position: list[float]) -> None:
        super().__init__()
        self.color = (255, 0, 0)
        self.position_on_sphere = position
        self.position_on_window = self.projection_to_2d()

    def projection_to_2d(self) -> list[float]:
        return self.project_point(self.position_on_sphere)

    def draw(self, window, font=None) -> None:
        try:
            pygame.draw.circle(
                window,
                self.color,
                (int(self.position_on_window[0]), int(self.position_on_window[1])),
                5 # Tamanho fixo para marcador
            )
        except TypeError:
            pass

class Goal(Elemento):
    """
    @brief Representação das traves do gol.
    """
    def __init__(self, position: list[float]) -> None:
        super().__init__()
        self.color = (0, 0, 255)
        self.position_on_sphere = position
        self.position_on_window = self.projection_to_2d()

    def projection_to_2d(self) -> list[float]:
        return self.project_point(self.position_on_sphere)

    def draw(self, window, font=None) -> None:
        try:
            pygame.draw.circle(
                window,
                self.color,
                (int(self.position_on_window[0]), int(self.position_on_window[1])),
                8
            )
        except TypeError:
            pass

class Line(Elemento):
    """
    @brief Representação das linhas de campo.
    """
    def __init__(self, double_list_position):
        super().__init__()
        self.color = (255, 255, 255)
        self.position_on_sphere = double_list_position
        self.position_on_window = self.projection_to_2d()

    def projection_to_2d(self) -> list[float]:
        # Linhas possuem dois pontos (início e fim)
        return self.project_point(self.position_on_sphere[:3]) + self.project_point(self.position_on_sphere[3:])

    def draw(self, window, font=None) -> None:
        try:
            pygame.draw.line(
                window,
                self.color,
                (int(self.position_on_window[0]), int(self.position_on_window[1])),
                (int(self.position_on_window[3]), int(self.position_on_window[4])),
                2
            )
        except TypeError:
            pass

class RobotVision:
    """
    @brief Classe principal que gerencia a conexão Socket, interpretação e renderização.
    """

    def __init__(self, agent_id=1):
        """
        @brief Inicializa o visualizador.
        @param agent_id ID do agente para criar o socket correto (/tmp/rc_debug_ID.sock).
        """
        # Variáveis de Estado
        self.last_raw_msg = ""
        self.objects = []

        # Variáveis de Rede (Socket UNIX)
        self.agent_id = agent_id
        self.socket_path = f"/tmp/debug_vision_agent_{self.agent_id}.sock"
        self.server_socket = None

    def setup_socket(self) -> None:
        """
        @brief Configura o socket UNIX do tipo DGRAM para receber dados do C++.
        @details
        Se o arquivo de socket já existir, ele é removido para evitar erros de 'Address already in use'.
        O socket é configurado como não-bloqueante para não travar a interface gráfica.
        """
        # Limpeza preventiva
        if os.path.exists(self.socket_path):
            os.remove(self.socket_path)

        # Criação do Socket
        self.server_socket = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
        self.server_socket.bind(self.socket_path)

        # Modo Não-Bloqueante: Se não houver dados, lança exceção ao invés de esperar
        self.server_socket.setblocking(False)

        print(f"[@] Ouvindo debug do Agente {self.agent_id} em: {self.socket_path}")

    def receive_from_socket(self) -> bool:
        """
        @brief Tenta receber um novo pacote do socket.
        @return True se recebeu novos dados, False se o buffer estava vazio.
        """
        try:
            # Buffer de 8KB (suficiente para mensagens de visão normais)
            data = self.server_socket.recv(8192)
            if data:
                self.last_raw_msg = data.decode('utf-8', errors='ignore')
                return True
        except BlockingIOError:
            # Nenhum dado disponível no momento
            pass
        except Exception as e:
            print(f"[!] Erro ao ler socket: {e}")

        return False

    def _get_only_tag_See(self) -> str | None:
        """
        @brief Extrai o bloco '(See ...)' da última mensagem recebida.
        @return Substring contendo apenas o bloco See ou None.
        """

        if not self.last_raw_msg:
            return None

        # 1. Definir o marcador de início
        start_marker = "(See"
        start_index = self.last_raw_msg.find(start_marker)

        # Se não houver informação visual
        if start_index == -1:
            return None

        # 2. Lógica de balanceamento de parênteses
        balance = 0
        end_index = -1

        for i in range(start_index, len(self.last_raw_msg)):
            char = self.last_raw_msg[i]

            if char == '(':
                balance += 1
            elif char == ')':
                balance -= 1

            if balance == 0:
                end_index = i
                break

        if end_index != -1:
            return self.last_raw_msg[start_index: end_index + 1]

        return None

    def parse_frame(self) -> None:
        """
        @brief Interpreta a mensagem 'See' extraída e popula a lista de objetos.
        @details
        A lógica de parsing interna permanece inalterada, apenas a fonte de dados mudou.
        """

        # Limpa objetos antigos para desenhar o novo frame
        self.objects.clear()

        chunk_see = self._get_only_tag_See()

        if not chunk_see:
            return

        # --- INÍCIO DA LÓGICA DE INTERPRETAÇÃO ORIGINAL (PRESERVADA) ---

        # Remove o "(See " inicial e o ")" final
        chunk_see = chunk_see[5:-1]

        counter_entry = 0
        buffer_objeto = ""
        for char in chunk_see:
            if char == '(':
                if counter_entry == 0:
                    buffer_objeto = ""
                counter_entry += 1

            if counter_entry > 0:
                buffer_objeto += char

            if char == ')':
                counter_entry -= 1

                if counter_entry == 0:
                    try:
                        match buffer_objeto[1]:
                            case 'B': # Ball
                                self.objects.append(Ball(list(map(float, buffer_objeto.split("pol")[1].replace(")", "").split()))))

                            case 'L': # Line
                                temp = buffer_objeto.replace("(", "").replace(")", "").split()
                                # Remove L, pol, pol (ajuste simples baseado no código original)
                                coords = [float(x) for x in temp if x not in ['L', 'pol']]
                                self.objects.append(Line(coords))

                            case 'F': # Flag
                                self.objects.append(Marker(list(map(float, buffer_objeto.split("pol")[1].replace(")", "").split()))))

                            case 'G': # Goal
                                self.objects.append(Goal(list(map(float, buffer_objeto.split("pol")[1].replace(")", "").split()))))
                            case _:
                                pass
                    except Exception as e:
                        # Proteção contra mensagens mal formadas que possam crashar o debug
                        pass

        # --- FIM DA LÓGICA DE INTERPRETAÇÃO ---

    @staticmethod
    def draw_legend(screen, items, font, padding=10, line_height=20):
        """
        @brief Desenha a legenda de cores na tela.
        @param screen Surface do Pygame.
        @param items Lista de tuplas (Nome, Cor).
        @param font Objeto de fonte do Pygame.
        """
        x = padding
        y = screen.get_height() - padding - line_height * len(items)

        for name, color in items:
            pygame.draw.rect(screen, color, (x, y + 4, 12, 12))
            text = font.render(name, True, (255, 255, 255))
            screen.blit(text, (x + 20, y))
            y += line_height

    def mainloop(self) -> None:
        """
        @brief Loop principal da aplicação (Game Loop).
        @details
        Gerencia eventos de entrada, recebimento de rede e renderização.
        """

        # 1. Configuração Inicial
        self.setup_socket()

        pygame.init()
        screen = pygame.display.set_mode((WIDTH, HEIGHT))
        pygame.display.set_caption(f"RobotVision - Agente {self.agent_id}")

        font = pygame.font.SysFont(None, 25)
        clock = pygame.time.Clock()

        legenda_dos_elementos = [
            ("Linha de Campo", (255, 255, 255)),
            ("Bola", (0, 255, 0)),
            ("Bandeira de Canto", (255, 0, 0)),
            ("Trave de Gol", (0, 0, 255))
        ]

        running = True

        # Variável para indicar se recebemos dados recentemente (para UI)
        last_update_time = perf_counter()
        connected_status = False

        print("--- Iniciando Loop Visual ---")

        try:
            while running:
                # 2. Processamento de Eventos (Input)
                for event in pygame.event.get():
                    if event.type == pygame.QUIT:
                        running = False

                    # Tecla ESC para sair
                    if event.type == pygame.KEYDOWN:
                        if event.key == pygame.K_ESCAPE:
                            running = False

                # 3. Rede: Tenta buscar novos dados
                if self.receive_from_socket():
                    self.parse_frame()
                    last_update_time = perf_counter()
                    connected_status = True
                else:
                    # Timeout visual simples: se passar 2 segundos sem dados, considera desconectado
                    if perf_counter() - last_update_time > 2.0:
                        connected_status = False

                # 4. Renderização
                screen.fill((0, 0, 0)) # Limpa tela

                # Desenha os objetos interpretados
                for obj in self.objects:
                    obj.draw(screen)

                # Desenha UI (Legenda e Status)
                self.draw_legend(screen, legenda_dos_elementos, font)

                # Status de Conexão no topo
                status_text = "CONECTADO" if connected_status else "AGUARDANDO DADOS..."
                status_color = (0, 255, 0) if connected_status else (255, 165, 0)
                lbl_status = font.render(f"Agente {self.agent_id}: {status_text}", True, status_color)
                screen.blit(lbl_status, (10, 10))

                pygame.display.flip()
                clock.tick(60) # Limita a 60 FPS para não fritar a CPU

        except KeyboardInterrupt:
            print("\nEncerrando via Teclado...")
        finally:
            # 5. Limpeza de Recursos
            print("Limpando recursos...")
            if self.server_socket:
                self.server_socket.close()
            if os.path.exists(self.socket_path):
                os.remove(self.socket_path)
            pygame.quit()

if __name__ == '__main__':
    # Permite passar o ID do agente via linha de comando: python3 vision.py 2
    agent = 1
    if len(sys.argv) > 1:
        try:
            agent = int(sys.argv[1])
        except ValueError:
            print("ID do agente inválido. Usando padrão 1.")

    app = RobotVision(agent_id=agent)
    app.mainloop()