#include "Logger.hpp"

void tarefaPesada(int id) {
    for (int i = 0; i < 1000; ++i) {
        Logger::get().info("Thread " + std::to_string(id) + " msg " + std::to_string(i));
    }
}

int main() {

    /* --- Testar Assincronicamente --- */

//    auto start = std::chrono::high_resolution_clock::now();
//
//    std::vector<std::thread> threads;
//    threads.reserve(10);
//    for (int i = 0; i < 10; ++i) { // 10 Threads
//        threads.emplace_back(tarefaPesada, i);
//    }
//
//    for (auto& t : threads) t.join();
//
//    auto end = std::chrono::high_resolution_clock::now();
//    std::chrono::duration<double> diff = end - start;
//
//    std::cout << "10.000 logs escritos em: " << diff.count() << " s\n";

    /* --- Testar Sincronicamente --- */
    std::cout << "Iniciando teste C++ (Single Thread / 10.000 logs)...\n";

    // Ponto de início da medição
    auto start = std::chrono::high_resolution_clock::now();

    // Loop sequencial na thread principal
    for (int i = 0; i < 1; ++i) {
        Logger::get().info("SingleThread msg " + std::to_string(i));
    }

    // Ponto final da medição (Tempo que a thread principal ficou ocupada)
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout << "Tempo de execucao (Main Thread): " << diff.count() << " segundos.\n"  << std::flush;

    return 0;
}

/*
Código Python para eventual comparação:

-----------------------------------------------
import threading
import time
from pathlib import Path
from datetime import datetime
import random
from string import ascii_uppercase

class Logger():
    _folder = None

    def __init__(self, is_enabled: bool, topic: str) -> None:
        self.no_of_entries = 0
        self.enabled = is_enabled
        self.topic = topic

    def write(self, msg: str, timestamp: bool = True, step: int = None) -> None:
        '''
        Write `msg` to file named `self.topic`
        '''
        if not self.enabled: return

        # The log folder is only created if needed
        if Logger._folder is None:
            rnd = ''.join(
                random.choices(ascii_uppercase, k=6))  # Useful if multiple processes are running in parallel
            Logger._folder = "./logs_python/" + datetime.now().strftime("%Y-%m-%d_%H.%M.%S__") + rnd + "/"
            print("\nLogger Info: see", Logger._folder)
            Path(Logger._folder).mkdir(parents=True, exist_ok=True)

        self.no_of_entries += 1

        # O GARGALO ESTÁ AQUI: Abrir e fechar arquivo a cada linha
        with open(Logger._folder + self.topic + ".log", 'a+') as f:
            prefix = ""
            write_step = step is not None
            if timestamp or write_step:
                prefix = "{"
                if timestamp:
                    prefix += datetime.now().strftime("%a %H:%M:%S")
                    if write_step: prefix += " "
                if write_step:
                    prefix += f'Step:{step}'
                prefix += "} "
            f.write(prefix + msg + "\n")

def tarefa_pesada(logger_instance, thread_id):
    """
    Simula o workerThread do C++:
    Envia 1000 mensagens para o log.
    """
    for i in range(1000):
        # Formatando a mensagem igual ao exemplo C++
        logger_instance.write(f"Thread {thread_id} msg {i}")


def main():
    # --- Testar Assincronicamente ---
    # print("Iniciando teste de performance Python...")
    #
    # # 1. Instancia o Logger
    # logger = Logger(is_enabled=True, topic="performance_test")
    #
    # start_time = time.time()
    #
    # threads = []
    # num_threads = 10
    #
    # # 2. Cria e inicia as threads
    # for i in range(num_threads):
    #     t = threading.Thread(target=tarefa_pesada, args=(logger, i))
    #     threads.append(t)
    #     t.start()
    #
    # # 3. Aguarda todas as threads terminarem (join)
    # for t in threads:
    #     t.join()
    #
    # end_time = time.time()
    # duration = end_time - start_time
    #
    # print(f"\nProcessamento finalizado.")
    # print(f"Total de logs: {num_threads * 1000}")
    # print(f"Tempo total: {duration:.4f} segundos")

    # --- Testar Sincronicamente
    print("Iniciando teste Python (Single Thread / 10.000 logs)...")

    # Instancia
    logger = Logger(is_enabled=True, topic="single_thread_test")

    # Ponto de início da medição
    start_time = time.time()

    # Loop sequencial na thread principal
    for i in range(10000):
        logger.write(f"SingleThread msg {i}")

    # Ponto final da medição
    end_time = time.time()
    duration = end_time - start_time

    print(f"Tempo de execucao (Main Thread): {duration:.4f} segundos.")


if __name__ == "__main__":
    main()
*/