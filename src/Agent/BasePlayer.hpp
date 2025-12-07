#pragma once

#include "../Booting/booting_tactical_formation.hpp"
#include "../Booting/booting_templates.hpp"
#include "../Communication/ServerComm.hpp"
#include <iostream>
#include <vector>

/**
 * @class BasePlayer
 * @brief Representa a entidade básica de um jogador na simulação.
 */
class BasePlayer {
protected:
    /**
     * @brief Gerenciador de comunicação com o servidor rcssserver3d.
     * @details Instanciado automaticamente na criação do jogador. É responsável por
     * enviar comandos e receber estados do jogo via TCP.
     */
    ServerComm _scom;

    /**
     * @brief Lista estática compartilhada contendo ponteiros para os comunicadores de todos os jogadores.
     * @details Usada para passar a referência dos "outros jogadores" durante a inicialização
     * e sincronização (Keep-Alive). Como fazemos o .reserve no construtor, não é necessário
     * que nos preocupemos com performance.
     */
    inline static std::vector<ServerComm*> _all_players_scom;

public:
    /**
     * @brief Número do uniforme do jogador.
     * @details Tipo `uint8_t` utilizado para otimização de memória, já que o valor varia apenas de 1 a 11.
     */
    uint8_t unum;

public:
    /**
     * @brief Construtor: Inicializa o jogador e estabelece conexão com o servidor.
     * @details Realiza a reserva de memória no vetor estático, cria a estrutura que
     * representará a lista de posições de cada jogador, define o número do uniforme,
     * executa o protocolo de handshake e registra o comunicador deste jogador na lista global.
     * @param unum Número do uniforme desejado para o agente (1 a 11).
     */
    BasePlayer(
        uint8_t unum
    ) {
        // Então é a primeira vez que estamos executando
        if(BasePlayer::_all_players_scom.capacity() < 11){
            // Otimização: Evita múltiplas realocações do vetor de ponteiros
            BasePlayer::_all_players_scom.reserve(11);

        }

        this->unum = unum;

        // Inicializa a conexão passando a lista atual de parceiros para sincronia
        this->_scom.initialize_agent(
            unum,
            BasePlayer::_all_players_scom
        );

        // Registra o comunicador deste jogador na lista estática para os próximos agentes
        BasePlayer::_all_players_scom.emplace_back(&this->_scom);
    }
};
