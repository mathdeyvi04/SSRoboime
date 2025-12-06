#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

// Inclua o seu header da classe Drawer aqui
#include "Drawer.hpp"

namespace nb = nanobind;

NB_MODULE(
    roboviz_drawer,
    m
) {

    nb::class_<Drawer>(m, "Drawer",
        R"pbdoc(
        Singleton de alta performance para envio de comandos ao RoboViz via UDP.

        O RoboViz organiza os desenhos em grupos chamados "Sets" (identificados por strings).
        Cada Set funciona como uma camada (layer) independente ou um quadro de animação.

        Regras de Ouro:
        1. **Agrupamento**: Todos os comandos de desenho (linhas, círculos, etc.) são apenas
           acumulados na memória até que você chame `swap_buffers(nome_do_set)`.

        2. **O Commit (Swap)**: O comando `swap_buffers("meu_set")` diz ao RoboViz:
           "Apague tudo que estava desenhado no set 'meu_set' e substitua pelo que enviei agora".

        3. **Independência**: Atualizar o set "bola" NÃO afeta o set "agentes".
           Isso permite ter desenhos estáticos (ex: linhas do campo) que você envia uma vez,
           e desenhos dinâmicos (ex: posição da bola) que você atualiza a 60FPS.

        4. **Cuidado com Nomes**: Se você desenhar uma linha no set "debug_A" mas chamar
           `swap_buffers("debug_B")`, a linha nunca aparecerá, pois você comitou o buffer errado.
           Mantenha a consistência nas strings.
        )pbdoc")

        .def_static("get_instance", &Drawer::get_instance, nb::rv_policy::reference,
            "Obtém a instância única da classe (Singleton).\n"
            ":return: Referência para o objeto Drawer. O Python não gerencia a memória deste objeto.")

        .def("clear", &Drawer::clear,
            "Limpa o buffer local de memória imediatamente, descartando comandos pendentes que não foram enviados.")

        .def("flush", &Drawer::flush,
            "Envia o pacote UDP final para o RoboViz.\n"
            "Deve ser chamado após os comandos de desenho e o swap_buffers.\n"
            ":return: True se houve envio de dados, False se o buffer estava vazio.")

        .def("swap_buffers", &Drawer::swap_buffers,
            nb::arg("set"),
            R"pbdoc(
            Finaliza o quadro para um conjunto específico (Commit).

            Este comando instrui o visualizador a trocar o buffer antigo pelo novo
            para o nome de set fornecido. Desenhos anteriores com este mesmo nome serão apagados.

            :param set: O nome do conjunto (ex: "debug_bola", "debug_agente_1").
            )pbdoc")

        .def("draw_line", &Drawer::draw_line,
            nb::arg("x1"), nb::arg("y1"), nb::arg("z1"),
            nb::arg("x2"), nb::arg("y2"), nb::arg("z2"),
            nb::arg("thickness"),
            nb::arg("r"), nb::arg("g"), nb::arg("b"),
            nb::arg("set"),
            "Adiciona uma linha ao buffer.\n"
            ":param x1, y1, z1: Ponto inicial.\n"
            ":param x2, y2, z2: Ponto final.\n"
            ":param thickness: Espessura da linha (em pixels).\n"
            ":param r, g, b: Cor RGB (0.0 a 1.0).\n"
            ":param set: Nome do conjunto ao qual este desenho pertence.")

        .def("draw_circle", &Drawer::draw_circle,
            nb::arg("x"), nb::arg("y"),
            nb::arg("radius"),
            nb::arg("thickness"),
            nb::arg("r"), nb::arg("g"), nb::arg("b"),
            nb::arg("set"),
            "Adiciona um círculo 2D (Billboard) ao buffer.\n"
            ":param x, y: Centro do círculo.\n"
            ":param radius: Raio em metros.\n"
            ":param thickness: Espessura da linha.\n"
            ":param r, g, b: Cor RGB (0.0 a 1.0).\n"
            ":param set: Nome do conjunto.")

        .def("draw_sphere", &Drawer::draw_sphere,
            nb::arg("x"), nb::arg("y"), nb::arg("z"),
            nb::arg("radius"),
            nb::arg("r"), nb::arg("g"), nb::arg("b"),
            nb::arg("set"),
            "Adiciona uma esfera 3D ao buffer.\n"
            ":param x, y, z: Centro da esfera.\n"
            ":param radius: Raio em metros.\n"
            ":param r, g, b: Cor RGB (0.0 a 1.0).\n"
            ":param set: Nome do conjunto.")

        .def("draw_point", &Drawer::draw_point,
            nb::arg("x"), nb::arg("y"), nb::arg("z"),
            nb::arg("size"),
            nb::arg("r"), nb::arg("g"), nb::arg("b"),
            nb::arg("set"),
            "Adiciona um ponto (pixel) ao buffer.\n"
            ":param x, y, z: Posição do ponto.\n"
            ":param size: Tamanho do ponto em pixels.\n"
            ":param r, g, b: Cor RGB (0.0 a 1.0).\n"
            ":param set: Nome do conjunto.")

        .def("draw_polygon", &Drawer::draw_polygon,
            nb::arg("verts"),
            nb::arg("r"), nb::arg("g"), nb::arg("b"), nb::arg("a"),
            nb::arg("set"),
            "Adiciona um polígono ao buffer.\n"
            ":param verts: Lista plana de floats [x1, y1, z1, x2, y2, z2, ...].\n"
            ":param r, g, b: Cor RGB (0.0 a 1.0).\n"
            ":param a: Alpha/Transparência (0.0 a 1.0).\n"
            ":param set: Nome do conjunto.")

        .def("draw_annotation", &Drawer::draw_annotation,
            nb::arg("text"),
            nb::arg("x"), nb::arg("y"), nb::arg("z"),
            nb::arg("r"), nb::arg("g"), nb::arg("b"),
            nb::arg("set"),
            "Adiciona um texto flutuante no espaço 3D.\n"
            ":param text: O texto a ser exibido.\n"
            ":param x, y, z: Posição do texto.\n"
            ":param r, g, b: Cor RGB (0.0 a 1.0).\n"
            ":param set: Nome do conjunto.");
}