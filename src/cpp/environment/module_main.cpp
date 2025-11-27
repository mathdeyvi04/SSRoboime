#include "Environment.hpp"
#include <nanobind/nanobind.h>

namespace nb = nanobind;

NB_MODULE(
    environment,
    m
){
    nb::class_<Environment>(m, "Environment")
        .def(nb::init<Logger&>())
        .def(
            "update_from_server",
            // A função anônima é apenas para convertermos os tipos
            [](
                Environment &self,
                const nb::bytearray& b
            ){
                return self.update_from_server(std::string_view(reinterpret_cast<const char*>(b.data()), b.size()));
            },
            "doc"
        );
}

