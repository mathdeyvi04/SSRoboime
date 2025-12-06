#include "Agent/BasePlayer.hpp"

int main() {

    std::vector<BasePlayer> players;
    for(
        int i = 1;
        i <= 11;
        i++
    ){
        players.emplace_back(BasePlayer(i));
    }



    return 0;
}