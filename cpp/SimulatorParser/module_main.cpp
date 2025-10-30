// #include <pybind11/pybind11.h>
// #include <pybind11/buffer_info.h>
// #include <pybind11/numpy.h>
// #include <iostream>

// namespace py = pybind11;

// // Dado pelo chatgpt
// // void parse_buffer(py::buffer buffer) {
// //     // Obtem informações do buffer Python
// //     py::buffer_info info = buffer.request();

// //     // Ponteiro cru para os bytes
// //     const unsigned char* data = static_cast<unsigned char*>(info.ptr);

// //     std::size_t size = info.size;  // Número de elementos (bytes)
// //     std::cout << "\n\nTamanho do buffer: " << size << " bytes\n";

// //     // Mostra os primeiros bytes em hex
// //     for (std::size_t i = 0; i < size; ++i) {
// //         printf("%02X", data[i]);
// //     }
// //     printf("\n");
// // }

// PYBIND11_MODULE(simulator_parser, m) {
//     // m.def("parse_buffer", &parse_buffer, "Recebe um bytearray do Python");
// }

#include "SimulatorParser.hpp"

int main(){

	SimulatorParser ex;
	ex.parsing(
		exemplo_de_mensagem,
		size
	);

	return 0;
}
