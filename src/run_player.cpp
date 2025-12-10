#include "Agent/BasePlayer.hpp"

int main() {

    BasePlayer p = BasePlayer(1);

    float x = -15;
    while(True){
        p.commit_beam(x, 10, 0, False);
        p._scom.send();
        p._scom.receive();
        x += 0.5;
    }

    return 0;
}