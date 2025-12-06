#pragma once

#include "../Booting/booting_templates.hpp"
#include "../Communication/ServerComm.hpp"

class BasePlayer {
private:

    ///< Classe gerenciadora de comunicação com servidor rcssserver3d. Já inicializada aqui!
    ServerComm __scom;

    inline static std::vector<BasePlayer*> all_players_in_match;
    inline static std::vector<ServerComm*> all_players_scom;
public:
    ///< Número de Uniforme, `uint8_t` para economizar memória, dado que varia de 1 à 11.
    uint8_t unum;
    ///< Tipo de Robô, `uint8_t` para economizar memória, dado que varia de 0 à 4.
    uint8_t type_robot;

public:
    BasePlayer(
        uint8_t unum
    ) {

        if(
            all_players_in_match.capacity() < 11
        ){
            all_players_in_match.reserve(11);
            all_players_scom.reserve(11);
        }

        // Realmente será útil em nosso código
        this->unum = unum;
        this->__scom.initialize_agent(unum, all_players_scom);

        // Ao final
        BasePlayer::all_players_in_match.push_back(this);
        BasePlayer::all_players_scom.push_back(&this->__scom);
    }







};
