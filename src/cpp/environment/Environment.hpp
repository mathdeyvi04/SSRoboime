#pragma once

#include <iostream>
#include <string_view>

class Environment {
public:
    /* Atributos Públicos de Ambiente*/



    /* Métodos Inerentes a Execução da Aplicação */

    /**
     * @brief
     * @details
     * @param
     * @param
     * @return
     */
    int
    update_from_server(
        std::string_view data
    ){
        std::cout << "Recebi: "
                  << data
                  << "\nNo total: "
                  << data.size();

        return 1;
    }
};




