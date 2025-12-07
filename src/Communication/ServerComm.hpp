#pragma once

#include "../Booting/booting_templates.hpp"

// --- Bibliotecas da Standard Library ---
#include <vector>
#include <string>
#include <iostream>
#include <cstring>
#include <cstdint>
#include <string_view>
#include <format>

// --- Bibliotecas de Sistema (POSIX) ---
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <sys/select.h>


/**
 * @class ServerComm
 * @brief Gerencia a comunicação TCP de baixo nível com o servidor rcssserver3d.
 * @details Implementa estratégias de buffering, leitura não-bloqueante segura (polling)
 * e envio otimizado via writev.
 */
class ServerComm {
private:
    /// Descritor de arquivo do socket
    int __sock_fd;
    /// Buffer persistente para leitura (evita realocações frequentes)
    std::vector<char> __read_buffer;
    
    // Pode ser que precisemos implementar um buffer de envio.

    /**
     * @brief Tenta ler exatamente N bytes do socket.
     * @param buffer Ponteiro para o destino dos dados.
     * @param len Quantidade de bytes a serem lidos.
     * @return True se leu todos os bytes com sucesso.
     * @return False se houve erro, timeout ou fechamento da conexão (EOF).
     */
    bool __recv_all(
        void* buffer,
        size_t len
    ) {
        size_t total_read = 0;
        char* ptr = static_cast<char*>(buffer);

        while(total_read < len){
            ssize_t bytes = ::recv(
                this->__sock_fd,
                ptr + total_read,
                len - total_read,
                0
            );

            if(bytes > 0){
                total_read += bytes;
            }
            else if(bytes == 0){
                return False; // EOF (Servidor fechou)
            }
            else {
                if(errno == EINTR){ continue; }
                // Timeout do socket (SO_RCVTIMEO) configurado no construtor
                if(errno == EAGAIN || errno == EWOULDBLOCK){ return False; }
                return False; // Erro fatal
            }
        }
        return True;
    }

public:
    /**
     * @brief Destrói o objeto e executa o encerramento gracioso (graceful shutdown) da conexão TCP.
     * @details
     * Implementa uma sequência robusta de finalização para prevenir erros de socket no lado do servidor
     * (como 'Broken pipe' ou 'Connection reset by peer'), comuns em servidores assíncronos.
     * 1. Shutdown de escrita (`SHUT_WR`): Envia um pacote TCP FIN, sinalizando logicamente que o cliente cessou o envio.
     * 2. Modo Não-Bloqueante (`O_NONBLOCK`): Configura o socket para garantir que a leitura de limpeza não congele a thread.
     * 3. Dreno do Buffer (`recv`): Consome dados residuais no buffer de entrada do kernel para evitar que o SO responda com RST ao fechar o socket.
     * 4. Fechamento (`close`): Libera, por fim, o descritor de arquivo do sistema.
     */
    ~ServerComm() {
        if(this->__sock_fd >= 0){
            shutdown(this->__sock_fd, SHUT_WR);
            int flags = fcntl(this->__sock_fd, F_GETFL, 0);
            fcntl(this->__sock_fd, F_SETFL, flags | O_NONBLOCK);
            recv(this->__sock_fd, this->__read_buffer.data(), 4096, 0);
            close(this->__sock_fd);
        }
    }

    /**
     * @brief Inicializa socket, buffers e configurações de rede.
     * @details Configura TCP_NODELAY para baixa latência e SO_RCVTIMEO para evitar deadlocks.
     */
    ServerComm() {
        // Ajuste para 64KB (mensagens de visão podem ser grandes)
        this->__read_buffer.resize(65536);

        this->__sock_fd = socket(
            AF_INET,
            SOCK_STREAM,
            0
        );

        if(this->__sock_fd < 0) {
            std::cerr << "Erro fatal: Socket falhou." << std::endl;
            exit(1);
        }

        // 1. TCP_NODELAY (Performance: envia pacotes pequenos imediatamente)
        int flag = 1;
        setsockopt(
            this->__sock_fd,
            IPPROTO_TCP,
            TCP_NODELAY,
            (char*)&flag,
            sizeof(int)
        );

        // 2. Timeout de Recebimento (Segurança: evita travamento eterno na leitura)
        struct timeval tv = {2, 0}; // 2 segundos
        setsockopt(
            this->__sock_fd,
            SOL_SOCKET,
            SO_RCVTIMEO,
            (const char*)&tv,
            sizeof(tv)
        );

        struct sockaddr_in serv_addr;
        std::memset(
            &serv_addr,
            0,
            sizeof(serv_addr)
        );
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(AGENT_PORT);
        inet_pton(
            AF_INET,
            AGENT_HOST,
            &serv_addr.sin_addr
        );

        // Tentativa de conexão com espera ativa simples
        while(
            connect(
                this->__sock_fd,
                (struct sockaddr*)&serv_addr,
                sizeof(serv_addr)
            ) != 0
        ){
            usleep(500000); // 0.5s wait
        }
    }


    /**
     * @brief Verifica se há dados prontos para leitura no Kernel.
     * @details Utiliza select com timeout 0 (polling) para não bloquear a thread.
     * @return True se houver bytes para ler, False caso contrário.
     */
    bool is_readable() {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(
            this->__sock_fd,
            &readfds
        );
        struct timeval tv = {0, 0}; // Retorno imediato

        return select(
            this->__sock_fd + 1,
            &readfds,
            NULL,
            NULL,
            &tv
        ) > 0;
    }

    /**
     * @brief Envia uma mensagem imediatamente utilizando Scatter/Gather I/O.
     * @details Constrói o cabeçalho de 4 bytes e envia junto com o corpo em uma única syscall (ou loop de syscalls),
     * garantindo integridade mesmo em caso de escritas parciais.
     * @param msg A mensagem a ser enviada (string_view evita cópias).
     * @return True se enviado com sucesso, False em caso de erro fatal.
     */
    bool send_immediate(
        std::string_view msg
    ) {
        if(msg.empty()){ return True; }

        uint32_t msg_len_host = static_cast<uint32_t>(msg.size());
        uint32_t msg_len_net = htonl(msg_len_host);

        struct iovec iov[2];
        size_t total_to_send = 4 + msg_len_host;
        size_t total_sent = 0;

        char* header_ptr = reinterpret_cast<char*>(&msg_len_net);
        const char* body_ptr = msg.data();

        while(total_sent < total_to_send){
            int iov_cnt = 0;

            if(total_sent < 4){
                // Parte 1: Cabeçalho ainda não foi totalmente enviado
                iov[iov_cnt].iov_base = header_ptr + total_sent;
                iov[iov_cnt].iov_len  = 4 - total_sent;
                iov_cnt++;

                // Parte 2: Corpo inteiro ainda precisa ir
                iov[iov_cnt].iov_base = (void*)body_ptr;
                iov[iov_cnt].iov_len  = msg_len_host;
                iov_cnt++;
            }
            else{
                // Parte 1 já foi, enviando apenas o restante do corpo
                size_t body_offset = total_sent - 4;
                iov[iov_cnt].iov_base = (void*)(body_ptr + body_offset);
                iov[iov_cnt].iov_len  = msg_len_host - body_offset;
                iov_cnt++;
            }

            ssize_t res = ::writev(
                this->__sock_fd,
                iov,
                iov_cnt
            );

            if(res > 0){ total_sent += res; }
            else if(res < 0){
                if(errno == EINTR){ continue; }
                if(errno == EAGAIN || errno == EWOULDBLOCK) {
                    usleep(1000); // Backoff curto para não fritar CPU
                    continue;
                }
                return False; // Erro real
            }
        }
        return True;
    }

    /**
     * @brief Lê uma mensagem completa do servidor.
     * @details Implementa estratégia de "Drenagem": Lê todas as mensagens disponíveis
     * e retorna apenas a mais recente para evitar lag acumulado.
     * @return std::string_view apontando para o buffer interno contendo a mensagem. Vazio se erro/timeout.
     */
    std::string_view receive() {
        uint32_t last_msg_size = 0;

        while(True) {
            uint32_t net_len = 0;

            // Tenta ler o cabeçalho (4 bytes)
            if(
                !this->__recv_all(
                    &net_len,
                    4
                )
            ){ break; }

            uint32_t msg_len = ntohl(net_len);

            // Tenta ler o corpo da mensagem
            if(
                !this->__recv_all(
                    this->__read_buffer.data(),
                    msg_len
                )
            ){ break; }

            last_msg_size = msg_len;

            // Estratégia de Drenagem: Se não há mais dados pendentes no Kernel,
            // paramos aqui e retornamos o que temos.
            if(!this->is_readable()){ break; }
        }

        if(last_msg_size > 0){
            this->__read_buffer[last_msg_size] = '\0'; // Null-terminate por segurança
            return std::string_view(
                this->__read_buffer.data(),
                last_msg_size
            );
        }
        return {};
    }

    /**
     * @brief Aguarda resposta do servidor mantendo os outros agentes vivos (Keep-Alive).
     * @details Realiza polling neste socket. Se não houver dados, envia (syn) para os parceiros
     * e drena a leitura deles para evitar buffer overflow.
     * @param other_players Lista de ponteiros para os comunicadores dos outros jogadores.
     */
    void receive_async(
        const std::vector<ServerComm*>& other_players
    ) {
        // Se não houver ninguém, apenas lê (pode bloquear por até 2s no timeout configurado)
        if(other_players.empty()){
            this->receive();
            return;
        }

        while(True){
            // 1. Se EU tenho dados, leio e saio imediatamente.
            if(this->is_readable()){
                this->receive();
                break;
            }

            // 2. Mantenho os outros vivos enquanto espero
            for(auto* p : other_players){
                p->send_immediate("(syn)");

                // Drena buffer dos outros SE houver dados
                if(p->is_readable()) {
                    p->receive();
                }
            }

            // Yield para a CPU (1ms) para evitar uso de 100% em busy wait
            usleep(1000);
        }
    }

    /**
     * @brief Realiza o handshake inicial do agente (Scene, Init e Sincronização).
     * @param unum Número do uniforme do jogador.
     * @param other_players Referência para lista de outros jogadores para sincronização.
     */
    void initialize_agent(
        int unum,
        std::vector<ServerComm*>& other_players
    ) {
        // Scene: Define o modelo do corpo do robô
        this->send_immediate(
            std::format(
                "(scene rsg/agent/nao/nao_hetero.rsg {})",
                (unum <= 1) ? 0 :
                (unum <= 4) ? 1 :
                (unum == 5) ? 2 :
                (unum <= 8) ? 3 : 4
            )
        );
        this->receive_async(other_players);

        // Init: Define time e número
        this->send_immediate(
            std::format(
                "(init (unum {}) (teamname {}))",
                unum,
                TEAM_NAME
            )
        );
        this->receive_async(other_players);

        // Sync Loop: Garante que todos entrem no ciclo de simulação juntos
        for(int i = 0; i < 3; ++i){
            this->send_immediate("(syn)");

            for(auto* p : other_players){
                p->send_immediate("(syn)");
            }

            // Drena outros sem travar
            for(auto* p : other_players) {
                if(p->is_readable()){ p->receive(); }
            }

            if(this->is_readable()){ this->receive(); }
        }
    }
};