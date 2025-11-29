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

        .def_ro("is_left", &Environment::is_left,
                "True caso sejamos o time da esquerda, False caso contrário.")

        .def_ro("current_mode", &Environment::current_mode,
                "True caso sejamos o time da esquerda, False caso contrário.")



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
            "Função responsável por atualizar o estado de ambiente a partir de mensagens do servidor."
        );


    nb::enum_<Environment::PlayMode>(m, "PlayMode")
            .value("OUR_KICKOFF", Environment::PlayMode::OUR_KICKOFF)
            .value("OUR_KICK_IN", Environment::PlayMode::OUR_KICK_IN)
            .value("OUR_CORNER_KICK", Environment::PlayMode::OUR_CORNER_KICK)
            .value("OUR_GOAL_KICK", Environment::PlayMode::OUR_GOAL_KICK)
            .value("OUR_FREE_KICK", Environment::PlayMode::OUR_FREE_KICK)
            .value("OUR_PASS", Environment::PlayMode::OUR_PASS)
            .value("OUR_DIR_FREE_KICK", Environment::PlayMode::OUR_DIR_FREE_KICK)
            .value("OUR_GOAL", Environment::PlayMode::OUR_GOAL)
            .value("OUR_OFFSIDE", Environment::PlayMode::OUR_OFFSIDE)

            .value("THEIR_KICKOFF", Environment::PlayMode::THEIR_KICKOFF)
            .value("THEIR_KICK_IN", Environment::PlayMode::THEIR_KICK_IN)
            .value("THEIR_CORNER_KICK", Environment::PlayMode::THEIR_CORNER_KICK)
            .value("THEIR_GOAL_KICK", Environment::PlayMode::THEIR_GOAL_KICK)
            .value("THEIR_FREE_KICK", Environment::PlayMode::THEIR_FREE_KICK)
            .value("THEIR_PASS", Environment::PlayMode::THEIR_PASS)
            .value("THEIR_DIR_FREE_KICK", Environment::PlayMode::THEIR_DIR_FREE_KICK)
            .value("THEIR_GOAL", Environment::PlayMode::THEIR_GOAL)
            .value("THEIR_OFFSIDE", Environment::PlayMode::THEIR_OFFSIDE)

            .value("BEFORE_KICKOFF", Environment::PlayMode::BEFORE_KICKOFF)
            .value("GAME_OVER", Environment::PlayMode::GAME_OVER)
            .value("PLAY_ON", Environment::PlayMode::PLAY_ON)
            .export_values();
}

