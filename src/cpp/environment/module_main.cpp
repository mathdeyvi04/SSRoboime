#include "Environment.hpp"
#include <nanobind/nanobind.h>

namespace nb = nanobind;

NB_MODULE(
    environment,
    m
){
    nb::class_<Environment>(
        m,
        "Environment",
        "Responsável por representar o ambiente externo ao robô.\n"
        "Agrupará todos os métodos de interpretação do mundo."
    )
        .def(
            nb::init<Logger&>(),
            nb::arg("Logger"),
            R"(Classe Logger para imprimirmos possíveis erros.)"
        )

        /* -- Definição de Atributos da Classe -- */
        .def_ro("time_server", &Environment::time_server,
        "Instante de Tempo do Servidor, útil apenas para sincronização entre agentes")

        .def_ro("time_match", &Environment::time_match,
            "Instante de Tempo de Partida")

        .def_ro("goals_scored", &Environment::goals_scored,
            "Nossos Gols, pode ser útil para mudarmos de tática conforme o jogo avança")

        .def_ro("goals_conceded", &Environment::goals_conceded,
            "Gols adversários, pode ser útil para mudarmos de tática conforme o jogo avança")


        /* -- Métodos da Classe -- */
        .def(
            "update_from_server",
            // A função anônima é apenas para convertermos os tipos
            [](
                Environment &self,
                const nb::bytearray& from_server
            ){
                return self.update_from_server(std::string_view(reinterpret_cast<const char*>(from_server.data()), from_server.size()));
            },
            nb::arg("from_server"),

        );
}

