#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <atomic>
#include <format>

namespace fs = std::filesystem;

#define True true
#define False false

/**
 * @brief Singleton para logging assíncrono.
 * @details Focada em performance utiliza uma lógica de fila de mensagens.
 */
class Logger {
public:
    /**
     * @brief Acesso à instância única
     */
    static Logger& get(){ static Logger instance; return instance; }

    Logger(const Logger&) = delete;
    void operator=(const Logger&) = delete;

    /**
     * @brief Adiciona log nível INFO.
     * @param msg Mensagem a ser imprimida.
     * @details Recebe por valor para permitir std::move (otimização de r-values).
     */
    void
    info(std::string msg){ this->log("[INFO]  ", std::move(msg)); }

    /**
     * @brief Adiciona log nível WARN.
     * @param msg Mensagem a ser imprimida.
     * @details Recebe por valor para permitir std::move (otimização de r-values).
     */
    void
    warn(std::string msg){ this->log("[WARN]  ", std::move(msg)); }

    /**
     * @brief Adiciona log nível ERROR.
     * @param msg Mensagem a ser imprimida.
     * @details Recebe por valor para permitir std::move (otimização de r-values).
     */
    void
    error(std::string msg){ this->log("[ERROR] ", std::move(msg)); }

    /**
     * @brief Log INFO usando C++20 std::format (Alta Performance).
     * @param fmt A string de formatação (ex: "Valor: {}"). Deve ser uma string literal (constante).
     * @param args Os argumentos a serem formatados.
     */
    template<typename... Args>
    void info(std::format_string<Args...> fmt, Args&&... args) {
        // std::format gera a std::string final de forma otimizada.
        // std::forward garante que não haja cópias desnecessárias dos argumentos.
        this->log("[INFO]  ", std::format(fmt, std::forward<Args>(args)...));
    }

    /**
     * @brief Log WARN usando C++20 std::format.
     */
    template<typename... Args>
    void warn(std::format_string<Args...> fmt, Args&&... args) {
        this->log("[WARN]  ", std::format(fmt, std::forward<Args>(args)...));
    }

    /**
     * @brief Log ERROR usando C++20 std::format.
     */
    template<typename... Args>
    void error(std::format_string<Args...> fmt, Args&&... args) {
        this->log("[ERROR] ", std::format(fmt, std::forward<Args>(args)...));
    }

private:
    // Buffers para técnica de Double Buffering
    std::vector<std::string> _current_buffer;
    std::vector<std::string> _write_buffer;
    
    std::mutex _mutex;
    std::condition_variable _cv;
    std::thread _worker;
    std::atomic<bool> _is_running;
    std::atomic<bool> is_the_first = True;
    std::ofstream _file_stream;

    /**
     * @brief Construtor privado: Inicializa arquivo e thread.
     * @details Reservará 1000 slots para evitarmos realocações
     */
    Logger() : _is_running(True) {
        // Reserva memória prévia para evitar realocações frequentes no vetor
        this->_current_buffer.reserve(30);
        this->_write_buffer.reserve(30);
    }

    /**
     * @brief Destrutor: Sinaliza parada e espera thread terminar.
     */
    ~Logger(){
        this->_is_running = false;
        this->_cv.notify_one(); ///< Informa a thread da condição de encerramento

        if(this->_worker.joinable()){ this->_worker.join(); }
        if(this->_file_stream.is_open()){ this->_file_stream.close(); }
    }

    /**
     * @brief Responsável por criar ambiente de .log
     * @details Possui uma lógica para garantir que logs sejam únicos.
     */
    void
    _init_file(){
        if(!fs::exists("logs")){ fs::create_directory("logs"); }

        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << "logs/" << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d_%H-%M-%S") << ".log";

        // std::ios::app não é necessário se o arquivo é único por execução
        // mas útil se reiniciarmos o logger no mesmo segundo -> Impossível?
        this->_file_stream.open(ss.str(), std::ios::out | std::ios::app);

        // Desabilita sincronização automática com stdio para performance
        std::ios_base::sync_with_stdio(false);
    }

    /**
     * @brief Responsável por providenciar genérica chamada de impressão em .log
     * @param prefixo Cabeçalho que será colocada antes da mensagem.
     * @param msg Mensagem principal.
     * Usa lock apenas para empurrar no vetor (operação de nanossegundos).
     */
    void
    log(const char* prefixo, std::string&& msg) {

        // --- INÍCIO DA ADIÇÃO DO TIMESTAMP ---
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss_time;
        // Formato: [YYYY-MM-DD HH:MM:SS]
        ss_time << std::put_time(std::localtime(&in_time_t), "[%Y-%m-%d %H:%M:%S] ");
        // --- FIM DA ADIÇÃO DO TIMESTAMP ---

        ///< Esse lock_guard trava enquanto estiver nesse escopo
        {
            std::lock_guard<std::mutex> lock(this->_mutex);
            // Constrói a string final na memória RAM
            this->_current_buffer.emplace_back(ss_time.str() + prefixo + msg);

            if( this->is_the_first ){ this->_init_file();
                                      this->_worker = std::thread(&Logger::_worker_loop, this);
                                      this->is_the_first = False;
                                    }
        }

        // Notifica a thread de escrita que há dados
        _cv.notify_one();
    }

    /**
     * @brief Loop da thread de background, responsável por escrever no arquivo .log da melhor forma possível.
     * @details Função de alto nível
     */
    void
    _worker_loop() {

        while(
            _is_running || !_current_buffer.empty()
        ){

            std::unique_lock<std::mutex> lock(_mutex);

            ///< Espera até ter dados ou ser instruído a encerrar
            /*
            A thread fica bloqueada pelo sistema operacional, sem consumir CPU.
            Pesquise, isso é muito foda.
            */
            _cv.wait(
                lock,
                [this](){ return !this->_current_buffer.empty() || !this->_is_running; }
            );

            if( this->_current_buffer.empty() && !this->_is_running ){ break; }

            // --- A MÁGICA DA PERFORMANCE (SWAP) ---
            // Trocamos o vetor cheio pelo vazio instantaneamente.
            // O Mutex é liberado log depois disso.
            std::swap(this->_current_buffer, this->_write_buffer);
            lock.unlock();

            ///< Agora escrevemos no disco SEM bloquear quem quer adicionar logs
            if(this->_file_stream.is_open()) {
                for(const auto& line : this->_write_buffer){ this->_file_stream << line << "\n"; }
                // Flush manual apenas após lote grande
                this->_file_stream.flush();
            }

            // Limpa o buffer de escrita para ser reusado no próximo swap
            this->_write_buffer.clear();
        }
    }
};

