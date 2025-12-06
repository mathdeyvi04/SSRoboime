#pragma once
/// Responsável pela implementação da comunicação com servidor rcssserver3d

#include "../Booting/booting_templates.hpp"

// --- Bibliotecas da Standard Library (C++) ---
#include <vector>       // Container dinâmico usado para buffers de leitura
#include <string>       // Manipulação de strings para filas de mensagens
#include <iostream>     /// Entrada e saída padrão (std::cerr, std::cout)
#include <cstring>      // Manipulação de memória bruta (std::memcpy, std::memset)
#include <cstdint>      // Tipos de inteiros com tamanho fixo (uint32_t)
#include <thread>       // (Opcional) Para sleep_for se desejar substituir usleep
#include <chrono>       // (Opcional) Para unidades de tempo
#include <string_view>  // O std::string_view é apenas uma "janela" leve (ponteiro + tamanho)
#include <format>

// --- Bibliotecas de Sistema (POSIX/Linux) ---
#include <sys/socket.h> // API principal de Sockets (socket, connect, recv, send)
#include <arpa/inet.h>  // Conversão de endereços IP (sockaddr_in, inet_pton, htons)
#include <netinet/tcp.h>// Definições específicas do protocolo TCP (TCP_NODELAY)
#include <unistd.h>     // Chamadas de sistema Unix padrão (close, writev, usleep)
#include <sys/uio.h>    // Estruturas para I/O vetorial (struct iovec para writev)
#include <fcntl.h>      // Controle de descritores de arquivo (bloqueante/não-bloqueante)
#include <sys/select.h> // Multiplexação de I/O síncrono (select)

class ServerComm {
private:
    /// File descriptor do socket
    int __sock_fd;

    /// Buffer persistente para leitura de dados (evita realocações)
    std::vector<char> __read_buffer;

    /// Fila de mensagens a serem enviadas (buffer de escrita)
    std::string __message_queue;

    /**
     * @brief Garante o recebimento completo de N bytes.
     * @return True se leu tudo, False se a conexão caiu ou erro fatal.
     */
    bool __recv_all(char* buffer, size_t len) {
        size_t total_read = 0;

        while (total_read < len) {
            char* write_ptr = buffer + total_read;
            size_t bytes_needed = len - total_read;

            // recv retorna: >0 (bytes lidos), 0 (conexão fechada), -1 (erro)
            ssize_t bytes = ::recv(this->__sock_fd, write_ptr, bytes_needed, 0);

            if (bytes > 0) {
                total_read += bytes;
            }
            else if (bytes == 0) {
                std::cerr << "Conexão fechada pelo servidor (EOF)." << std::endl;
                return False;
            }
            else {
                // Se bytes == -1, verificamos o erro
                if (errno == EINTR) {
                    continue; // Foi apenas interrompido por um sinal, tenta de novo
                }
                // Se for EAGAIN/EWOULDBLOCK, significa que o socket está não-bloqueante
                // e não há dados agora. Se sua lógica exige bloqueio, isso é erro.
                std::cerr << "Erro no recv. Errno: " << errno << std::endl;
                return False;
            }
        }
        return True;
    }

    /**
     * @brief Configura o socket para modo bloqueante ou não-bloqueante.
     * @param blocking True para bloqueante, False para não-bloqueante.
     */
    inline void __set_blocking_mode(bool blocking) {
        int flags = fcntl(this->__sock_fd, F_GETFL, 0);
        if(flags == -1){ return; }

        if(blocking){
            flags &= ~O_NONBLOCK;
        } else {
            flags |= O_NONBLOCK;
        }

        fcntl(this->__sock_fd, F_SETFL, flags);
    }

public:

    /**
     * @brief Construtor que inicializa o socket e buffers.
     */
    ServerComm() {
        // Inicialização de variáveis membro
        this->__sock_fd = -1;
        this->__read_buffer.resize(4096); // Pré-aloca 4KB
        this->__message_queue.reserve(4096); // Reserva espaço

        // Criação do Socket TCP
        this->__sock_fd = socket(AF_INET, SOCK_STREAM, 0);

        if(this->__sock_fd < 0){
            std::cerr << "Erro fatal: Falha ao criar socket." << std::endl;
            exit(1);
        }

        // Otimização de Performance: TCP_NODELAY
        // desativa o Algoritmo de Nagle, que agrupa pequenos pacotes antes de enviar (reduz overhead, mas aumenta latência)
        int flag = 1;
        int result_opt = ::setsockopt(
            this->__sock_fd,
            IPPROTO_TCP,
            TCP_NODELAY,
            (char*)&flag,
            sizeof(int)
        );

        if(result_opt < 0){
             std::cerr << "Aviso: Falha ao definir TCP_NODELAY." << std::endl;
        }

        // Configuração do Endereço
        struct sockaddr_in serv_addr;
        std::memset(&serv_addr, 0, sizeof(serv_addr));

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(AGENT_PORT);
        inet_pton(AF_INET, AGENT_HOST, &serv_addr.sin_addr);

        // Tentativa de Conexão
        while(
            connect(
                this->__sock_fd,
                (struct sockaddr*)&serv_addr,
                sizeof(serv_addr)
            ) != 0
        ) {

            // Em produção, usar usleep para não travar CPU
            usleep(500000); // 0.5s
        }
    }

    /**
     * @brief Destrutor para limpeza de recursos.
     */
    ~ServerComm() {
        if(this->__sock_fd >= 0){ close(this->__sock_fd); }
    }

    /**
     * @brief Realiza o protocolo de inicialização do agente no campo (Handshake).
     * @param unum Número do uniforme do jogador (1-11).
     */
    void initialize_agent(int unum, std::vector<ServerComm*> other_players) {

        // Montamos a mensagem de SCENE para carregar a física do robô
        this->send_immediate(
            std::format(
                "(scene rsg/agent/nao/nao_hetero.rsg {})",
                (unum <= 1) ? 0 :
                (unum <= 4) ? 1 :
                (unum == 5) ? 2 :
                (unum <= 8) ? 3 : 4
            )
        );

        // Montamos a mensagem de INIT definindo o uniforme e o nome do time
        this->send_immediate(
            std::format(
                "(init (unum {}) (teamname {}))",
                unum,
                TEAM_NAME
            )
        );
    }

    ///< Espaço para definirmos `receive`, `receive_async` e continuarmos `initialize_agent`.














    /**
     * @brief Envia uma mensagem instantânea usando Scatter/Gather I/O (writev).
     * @param msg A string de dados a ser enviada.
     * @details Usa writev para enviar Header+Body em uma única syscall sem cópia de memória.
     */
    void send_immediate(const std::string& msg) {
        // Evitamos syscalls desnecessárias se a string estiver vazia
        if (msg.empty()) {
            return;
        }

        // Conversão do tamanho para Big Endian (Network Byte Order)
        // O cast para uint32_t garante que estamos lidando com 4 bytes exatos
        uint32_t msg_len = static_cast<uint32_t>(msg.size());
        uint32_t net_len = htonl(msg_len);

        // Preparamos o vetor de I/O para envio atômico
        struct iovec iov[2];

        // Parte 1: Cabeçalho de 4 bytes
        iov[0].iov_base = &net_len;
        iov[0].iov_len = 4; // Constante direta conforme solicitado

        // Parte 2: Corpo da mensagem (ponteiro direto, zero-copy)
        iov[1].iov_base = (void*)msg.data();
        iov[1].iov_len = msg_len;

        if(
            writev(
                this->__sock_fd,
                iov,
                2
            ) < 0
        ){
            std::cerr << "[ERROR] Falha no writev: "
                      << strerror(errno)
                      << std::endl;
        }
    }

    /**
     * @brief Adiciona uma mensagem à fila de envio (bufferização).
     * @param msg Mensagem em bytes/string.
     */
    void commit(const std::string& msg) {
        this->__message_queue += msg;
    }

    /**
     * @brief Envia todas as mensagens da fila de uma vez.
     * @details Adiciona (syn) ao final automaticamente se o socket estiver livre para escrita.
     */
    void send() {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(this->__sock_fd, &readfds);
        struct timeval tv = {0, 0};

        // Se select retornar 0, o socket de leitura está vazio, podemos enviar.
        if(
            select(
                this->__sock_fd + 1,
                &readfds,
                NULL,
                NULL,
                &tv
            ) == 0
        ) {
            this->__message_queue += "(syn)";
            this->send_immediate(this->__message_queue);
        }
        else {
             // Aviso: socket de leitura cheio, pulando envio para priorizar leitura no prox ciclo
             // std::cerr << "Warning: Read socket busy." << std::endl;
        }
        this->__message_queue.clear();
    }

    /**
     * @brief Limpa a fila de mensagens sem enviar.
     */
    void clear_queue() {
        this->__message_queue.clear();
    }
};