#include "Agent/BasePlayer.hpp"

int main() {

    BasePlayer p = BasePlayer(1);

    while(True){
        p.think_send_and_receive();
    }

    return 0;
}