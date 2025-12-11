#pragma once

#include "../Booting/booting_tactical_formation.hpp"
#include "../Booting/booting_templates.hpp"
#include "../Communication/ServerComm.hpp"
#include "../Logger/Logger.hpp"
#include "../Environment/Environment.hpp"
#include <iostream>
#include <vector>

/**
 * @class BasePlayer
 * @brief Representa a entidade básica de um jogador na simulação.
 */
class BasePlayer {
public:  ///< Devemos modificar isso e tornar protegidos.
    /**
     * @brief Gerenciador de comunicação com o servidor rcssserver3d.
     * @details Instanciado automaticamente na criação do jogador. É responsável por
     * enviar comandos e receber estados do jogo via TCP.
     */
    ServerComm _scom;

    /**
     * @brief Representador do Ambiente
     * @details Instanciado automaticamente na criação do jogador. É responsável por
     * enviar representar todas as características do ambiente.
     */
    Environment _env;


    /**
     * @brief Lista estática compartilhada contendo ponteiros para os comunicadores de todos os jogadores.
     * @details Usada para passar a referência dos "outros jogadores" durante a inicialização
     * e sincronização (Keep-Alive). Como fazemos o .reserve no construtor, não é necessário
     * que nos preocupemos com performance.
     */
    inline static std::vector<ServerComm*> _all_players_scom;

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
    ) :
        _env(Logger::get())
    {
        // Então é a primeira vez que estamos executando
        if(BasePlayer::_all_players_scom.capacity() < 11){
            // Otimização: Evita múltiplas realocações do vetor de ponteiros
            BasePlayer::_all_players_scom.reserve(11);
        }

        // Inicializa a conexão passando a lista atual de parceiros para sincronia
        this->_scom.initialize_agent(
            unum,
            BasePlayer::_all_players_scom,
            &this->_env
        );

        // Registra o comunicador deste jogador na lista estática para os próximos agentes
        BasePlayer::_all_players_scom.emplace_back(&this->_scom);
    }

    /**
     * @brief Comando de beam oficial do agente
     * @param posx Posição X de beam
     * @param posy Posição Y de beam
     * @param rotation Valor de rotação a ser dado ao robô.
     * @param init_beam Booleano para indicar se trata-se do primeiro beam, o de alocação.
     */
    void commit_beam(float posx, float posy, float rotation, bool init_beam = False) {
        this->_scom.commit(
            std::format(
                "(beam {} {} {})",
                (init_beam) ? TacticalFormation::Default[this->_env.unum - 1][0] :
                              posx,
                (init_beam) ? TacticalFormation::Default[this->_env.unum - 1][1] :
                              posy,
                (init_beam) ? 0 :
                              rotation
            )
        );
    }
};
