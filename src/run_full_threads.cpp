#include "Agent/BasePlayer.hpp"
#include <thread>
#include <vector>

void worker(BasePlayer* p, int* valor_a_ser_incrementado) {

    std::cout << "Estou aqui vendo: " << static_cast<void*>(p) << std::endl;
    while(
        *valor_a_ser_incrementado < 15
    ){
        std::cout << "Neste momento, vejo: " << *valor_a_ser_incrementado << std::endl;
        (*valor_a_ser_incrementado)++;
        usleep(1*1000*1000);
    }
    std::cout << "Saindo." << std::endl;
}

int main() {

    ///< Por motivos de cuidado, faremos inicialização de forma sequencial
    std::vector<BasePlayer> players;
    players.reserve(11);
    for(
        int i = 1;
        i <= 11;
        i++
    ){
        players.emplace_back(i);
    }

    std::vector<std::thread> threads;
    threads.reserve(11);
    int valores[11] = {0};
    int i = 0;

    for(auto& p : players) {  // Captura referência se players for container
        threads.emplace_back(
            [&p, &valores, i]() {  // &valores captura referência ao array
                worker(&p, valores + i);
            }
        );
        i++;
    }

    for(auto& t: threads){
        if(t.joinable()){ t.join(); }
    }

    return 0;
}