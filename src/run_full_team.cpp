#include "Agent/BasePlayer.hpp"
#include <vector>

int main() {

    std::signal(SIGINT, ender);

    std::vector<BasePlayer> players;
    players.reserve(11);
    for(
        int i = 1;
        i <= 2;
        i++
    ){
        players.emplace_back(i);
    }

    for(auto& p : players){
        p.commit_beam(0, 0, 0);
        p._scom.send();
    }

    for(auto& p : players){
        p._scom.receive();
    }

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