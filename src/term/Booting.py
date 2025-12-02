"""
@file Booting.py
@brief Implementação do Booting do time
"""
import os
import sys
import subprocess
import sysconfig
import nanobind
import threading
import pickle
from time import sleep
from term.Printing import Printing
from pathlib import Path

class Booting:
    """
    @brief Responsável por inicializar todas as necessidades de execução do time
    @details
    Assume as seguintes responsabilidades:
        - Estabelece um arquivo de configurações default caso já não exista um.
    """

    CONFIG_PATH = Path(__file__).resolve().parent / "config_team_params.txt"

    def __init__(self):
        """
        @brief Responsável por chamar as inicializações mínimas.
        """

        self.options = Booting.get_team_params()

        if getattr(sys, 'frozen', False):
            # Então estamos executando o binário!
            # Devemos forçar que o debug seja 0.
            self.options[8][1] = '0'
            Printing.IF_IN_DEBUG = False
        else:
            # Note que isso só faz sentido quando não estamos executando o código em binário
            # Já que esta execução não conteria os arquivos .hpp, por exemplo.
            Booting.cpp_builder()

    @staticmethod
    def get_team_params() -> list[list[str | int]]:
        """
        @brief Verifica existência de arquivo de parâmetros de time, caso não exista, usará o default.
        @details
        Faremos em tupla para permitir uso mínimo de memória.
        @return
        """

        if os.path.exists(Booting.CONFIG_PATH):
            with open(
                    Booting.CONFIG_PATH,
                    "r"
            ) as file_team_params:
                config_team_params: list[list[str | int]] =  [
                    string_tupla.split(",") for string_tupla in file_team_params.read().split("\n")[:-1]
                ]

                for idx in range(0, len(config_team_params)):
                    # Somente o IP Server e Team Name são palavras
                    if idx not in {0, 3}:
                        config_team_params[idx][1] = int(config_team_params[idx][1])



        config_team_params = [
            ["IP Server",       "localhost"],
            ["Agent Port",      3100], # Onde nos conectaremos com rcssserver3d
            ["Monitor Port",    3200], # Onde nos conectaremos com Roboviz
            ["Team Name",       "RoboIME"],
            ["Uniform Number",  1],
            ["Robot Type",      1],
            ["Penalty Shootout", 0],
            ["MagmaFatProxy",   0],
            ["Debug Mode",      1]
        ]

        # E criamos o arquivo
        with open(
            Booting.CONFIG_PATH,
            "w+"
        ) as file_team_params:
            for doc, value in config_team_params:
                file_team_params.write(
                    f"{doc},{value}\n"
                )

        return config_team_params

    @staticmethod
    def show_spinner(
            running_flag: list[bool]
    ) -> None:
        """
        @brief Por motivos estéticos, mostrará um spinner enquanto há o carregamento de módulos C++
        """

        spinner = ['|', '/', '-', '\\']
        i = 0
        while running_flag[0] and i < 1000:
            print(f"{spinner[i % len(spinner)]}", end='', flush=True)
            i += 1
            sleep(0.5)
            print("\b", end='')


    @staticmethod
    def cpp_builder() -> None:
        """
        @brief Responsável por buildar os arquivos .cpp presentes na pasta cpp.
        @return Funcionalidades C++ em condições de interoperabilidade.
        """

        # Vamos verificar quais arquivos .cpp estão disponíveis para buildar
        cpp_path = Path(__file__).resolve().parents[1] / "cpp"
        cpp_modules = [
            module for module in os.listdir(
                cpp_path
            ) if os.path.isdir(os.path.join(cpp_path, module))
        ]

        if not cpp_modules:
            return None # Não há nenhum para construirmos

        # Servirá para verificarmos quais binários estão atualizados com a versão
        python_cmd = f"python{sys.version_info.major}.{sys.version_info.minor}"

        # -- Os includes que serão necessários
        nb_root = os.path.dirname(nanobind.__file__)
        py_inc = sysconfig.get_path("include") # Python.h
        nb_inc = nanobind.include_dir() # nanobind.h
        robin_inc = os.path.join(nb_root, "ext", "robin_map", "include") # robin_map.h
        nb_src = os.path.join(nb_root, "src", "nb_combined.cpp")
        n_proc = str(os.cpu_count())
        command_chain = [
            "make",
            f"-j{n_proc}",
            f"PY_INC={py_inc}",
            f"NB_INC={nb_inc}",
            f"ROBIN_INC={robin_inc}",
            f"NB_SRC={nb_src}"
        ]

        first = True
        for cpp_module in cpp_modules:
            cpp_module_path = os.path.join(cpp_path, cpp_module)

            # Verificamos se já existe um binário pronto
            if os.path.isfile(
                os.path.join(
                    cpp_module_path,
                    f"{cpp_module}.so"
                )
            ):
                # Caso exista, devemos verificar se ele foi modificado em um limite de tempo
                with open(
                    os.path.join(
                        cpp_module_path,
                        f"{cpp_module}.cpp_info"
                    ),
                    "rb"
                ) as f:
                    info_version = pickle.load(f)

                if info_version == python_cmd:
                    # Considerando que está na mesma versão, ainda devemos verificar modificações

                    code_mod_time = max(
                        os.path.getmtime(
                            os.path.join(
                                cpp_module_path,
                                file_in_the_module
                            )
                        ) for file_in_the_module in os.listdir(
                            cpp_module_path
                        ) if file_in_the_module.endswith(".cpp") or file_in_the_module.endswith(".hpp")
                    )

                    bin_mod_time = os.path.getmtime(os.path.join(cpp_module_path, f"{cpp_module}.so"))

                    if bin_mod_time + 15 > code_mod_time:
                        continue

            if first:
                print("\033[1;7m/* ---- Construção de Funcionalidades C++ ---- */\033[0m")
                first = False
            msg = f"\033[1;7mConstruindo: \033[32;40m{cpp_module}\033[0m"
            print(f"{msg:.<{60}}", end='', flush=True)

            processo = subprocess.Popen(
                command_chain,
                cwd=cpp_module_path,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=False
            )

            # Iniciamos thread de spinner
            running_flag = [True]
            worker = threading.Thread(target=Booting.show_spinner, args=(running_flag,))
            worker.start()

            output, error = processo.communicate()
            return_code = processo.wait()

            running_flag[0] = False
            worker.join()

            if return_code == 0:
                print("\033[7m\033[1mSucesso\033[0m")

                # Podemos construir um arquivo de fiscalização
                with open(
                    os.path.join(cpp_module_path, f"{cpp_module}.cpp_info"),
                    "wb"
                ) as f:
                    # noinspection PyTypeChecker
                    pickle.dump(python_cmd, f)
            else:
                Printing.print_message("Abortando", "error")
                print()
                print(output.decode(), error.decode())
                exit()

            subprocess.run(
                ["make", "clean"],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                cwd=cpp_module_path
            )

        return None
