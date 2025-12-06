#include "Agent/BasePlayer.hpp"

int main() {

    std::vector<BasePlayer> players;
    players.reserve(11);
    for(
        int i = 1;
        i <= 11;
        i++
    ){
        players.emplace_back(i);
    }

    usleep(50000000);



    return 0;
}