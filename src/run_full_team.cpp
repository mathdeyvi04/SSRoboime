#include "Agent/BasePlayer.hpp"
#include <vector>

int main() {

    std::signal(SIGINT, ender);

    std::vector<BasePlayer> players;
    players.reserve(11);
    for(
        int i = 1;
        i <= 11;
        i++
    ){
        players.emplace_back(i);
    }

    while(::is_running){
        usleep(5*100*1000);
    }

    std::cout << "Encerrando corretamente." << std::flush;

    return 0;
}