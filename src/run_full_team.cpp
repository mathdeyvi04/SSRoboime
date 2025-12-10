#include "Agent/BasePlayer.hpp"
#include <vector>

///< Verifique o is_left do Environment

int main() {

    std::signal(SIGINT, ender);

    std::vector<BasePlayer> players;
    players.reserve(11);
    for(
        int i = 1;
        i <= 10;
        i++
    ){
        players.emplace_back(i);
    }

    float x = - 0.5;
    for(auto& p : players){
        p.commit_beam(x, 10, 0, False);
        p._scom.send();
        x -= 1.5;
    }

    for(auto& p : players){
        p._scom.receive();
    }
    see_only_when_i_want = true;

    while(::is_running){
        for(auto& p : players){
            p._scom.send();
        }

        for(auto& p : players){
            p._scom.receive();
        }
    }

    std::cout << "Encerrando corretamente." << std::flush;

    return 0;
}