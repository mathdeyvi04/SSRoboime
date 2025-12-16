#include "Agent/BasePlayer.hpp"

int main() {

    BasePlayer p = BasePlayer(1);

    while(True){
        p._scom.send();
        p._scom.receive();
    }

    return 0;
}