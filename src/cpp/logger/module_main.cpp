#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h> ///< Necessário para converter std::string <-> str automaticamente
#include "Logger.hpp"

namespace nb = nanobind;

NB_MODULE(logger, m) {

    // Vinculamos a classe Logger.
    // Note que não usamos .def(nb::init<...>) pois o construtor é privado.
    nb::class_<Logger>(m, "Logger")
        // nb::rv_policy::reference -> Diz ao Python para criar apenas uma referência
        // para o objeto estático existente no C++, sem tentar gerenciá-lo ou deletá-lo.
        .def_static("get", &Logger::get, nb::rv_policy::reference,
            "Acesso à instância única")
        .def("info", &Logger::info, nb::arg("msg"),
            "Adiciona log nível INFO.\n\n"
            "Args:\n"
            "    msg (str): Mensagem a ser imprimida.\n\n"
            "Details:\n"
            "    Recebe por valor para permitir std::move (otimização de r-values).")
        .def("warn", &Logger::warn, nb::arg("msg"),
            "Adiciona log nível WARN.\n\n"
            "Args:\n"
            "    msg (str): Mensagem a ser imprimida.\n\n"
            "Details:\n"
            "    Recebe por valor para permitir std::move (otimização de r-values).")
        .def("error", &Logger::error, nb::arg("msg"),
            "Adiciona log nível ERROR.\n\n"
            "Args:\n"
            "    msg (str): Mensagem a ser imprimida.\n\n"
            "Details:\n"
            "    Recebe por valor para permitir std::move (otimização de r-values).");
}

