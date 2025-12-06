/**
 * @file debug.cc
 * @brief Teste Interativo passo-a-passo.
 * @details Permite verificar se o problema é volume de dados, delay ou conexão.
 */

#include "Drawer.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>

///< Função auxiliar para aguardar o usuário
void wait_enter(const std::string& msg) {
    std::cout << "\n----------------------------------------------------" << std::endl;
    std::cout << "[PAUSA] " << msg << std::endl;
    std::cout << "-> Pressione ENTER para continuar..." << std::endl;
    std::cin.ignore();
}

int main() {
    std::cout << "=== INICIANDO DEBUG INTERATIVO DO DRAWER ===" << std::endl;

    Drawer& drawer = Drawer::get_instance();
    std::string set_static = "debug_estatico";
    std::string set_anim = "debug_animacao";

    // -----------------------------------------------------------
    // PASSO 1: Teste de Conectividade Mínima
    // -----------------------------------------------------------
    // Objetivo: Garantir que 1 pacote pequeno chega.

    std::cout << "1. Enviando uma unica linha de teste..." << std::endl;

    drawer.draw_line(0, 0, 0, 0, 0, 2, 5.0f, 1, 1, 1, set_static);
    drawer.draw_annotation("Teste 1: OK", 0, 0, 2.2, 1, 1, 1, set_static);

    drawer.swap_buffers(set_static); // Commita o desenho
    drawer.flush();                  // Envia o pacote

    wait_enter("Verifique se apareceu uma linha BRANCA vertical no centro.");

    // -----------------------------------------------------------
    // PASSO 2: Teste de Volume (Shapes variados)
    // -----------------------------------------------------------
    // Objetivo: Testar se shapes diferentes quebram o parser.
    // Enviaremos em pacotes separados para evitar MTU por enquanto.

    std::cout << "2. Enviando formas geometricas..." << std::endl;

    // Círculo
    drawer.draw_circle(2, 2, 1.0, 3.0f, 1, 0, 0, set_static); // Vermelho
    drawer.draw_annotation("Circulo", 2, 2, 1.2, 1, 0, 0, set_static);

    // Esfera
    drawer.draw_sphere(-2, 2, 1.0, 0.5, 0, 1, 0, set_static); // Verde
    drawer.draw_annotation("Esfera", -2, 2, 1.8, 0, 1, 0, set_static);

    // Polígono
    std::vector<float> poly = {1, -1, 0,  2, -2, 0,  0, -2, 0};
    drawer.draw_polygon(poly, 0, 0, 1, 0.5, set_static); // Azul semi-transparente

    drawer.swap_buffers(set_static);
    drawer.flush();

    wait_enter("Verifique se surgiram: Circulo Vermelho, Esfera Verde e Triangulo Azul.");

    // -----------------------------------------------------------
    // PASSO 3: Teste de "Lag" / Animação (Loop)
    // -----------------------------------------------------------
    // Objetivo: Verificar se o Roboviz consegue processar atualizações contínuas (60 FPS).
    // Se o Roboviz estiver "demorando para interpretar", a animação ficará travada.

    std::cout << "3. Iniciando teste de animacao (10 segundos)." << std::endl;
    std::cout << "   Uma bola amarela deve orbitar o centro suavemente." << std::endl;
    std::cout << "   Se ela pular ou travar, ha gargalo na rede ou no parser." << std::endl;

    float angle = 0.0f;
    for (int i = 0; i < 600; ++i) { // ~10 segundos a 60fps
        angle += 0.05f;
        float x = std::cos(angle) * 3.0f;
        float y = std::sin(angle) * 3.0f;

        // Desenha a bola
        drawer.draw_sphere(x, y, 0.5, 0.2, 1, 1, 0, set_anim);

        // Desenha o "braço" que segura a bola
        drawer.draw_line(0, 0, 0, x, y, 0.5, 2.0f, 1, 1, 1, set_anim);

        // Troca APENAS o buffer da animação. O estático permanece lá.
        drawer.swap_buffers(set_anim);

        // Envia imediatamente
        drawer.flush();

        // Dorme ~16ms (60 FPS)
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    // Limpa a animação ao final
    drawer.clear(); // Limpa buffer local
    drawer.swap_buffers(set_anim); // Manda um swap vazio para apagar o desenho no roboviz
    drawer.flush();

    std::cout << "\nTeste Finalizado." << std::endl;
    return 0;
}