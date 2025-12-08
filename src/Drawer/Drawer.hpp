#pragma once

#include <vector>
#include <string>
#include <cstring>      // memcpy, memset
#include <cstdio>       // snprintf
#include <mutex>        // thread safety
#include <arpa/inet.h>  // sockets
#include <sys/socket.h> // sockets
#include <unistd.h>     // close
#include <iostream>

/**
 * @class Drawer
 * @brief Singleton de alta performance para envio de comandos ao RoboViz.
 * @details Implementa o protocolo híbrido (Binário + Texto ASCII Fixo) utilizado pelo visualizador.
 */
class Drawer {
private:
    int __socket_fd;                        ///< Descritor do socket UDP.
    struct sockaddr_in __dest_addr;         ///< Estrutura de endereço do destino (RoboViz).
    std::vector<unsigned char> __buffer;    ///< Buffer persistente para acumular comandos.
    std::mutex __mutex;                     ///< Mutex para garantir thread-safety.

    /**
     * @brief Construtor Privado (Singleton).
     * @details Inicializa o socket e reserva memória para evitar realocações frequentes.
     */
    Drawer() {
        std::string ip = "127.0.0.1";
        int port = 32769;

        this->__socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (this->__socket_fd < 0) {
            std::cerr << "[Drawer] Erro critico: Falha ao criar socket." << std::endl;
        }

        std::memset(&this->__dest_addr, 0, sizeof(this->__dest_addr));
        this->__dest_addr.sin_family = AF_INET;
        this->__dest_addr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &this->__dest_addr.sin_addr);

        // Reserva 65KB (tamanho máximo seguro de um pacote UDP)
        this->__buffer.reserve(65536);
    }

    /**
     * @brief Destrutor. Fecha o socket se estiver aberto.
     */
    ~Drawer() {
        if (this->__socket_fd >= 0) {
            close(this->__socket_fd);
        }
    }

    /**
     * @brief Escreve um byte único no buffer.
     * @param value O valor (0-255) a ser escrito.
     */
    inline void __write_byte(unsigned char value) {
        this->__buffer.push_back(value);
    }

    /**
     * @brief Escreve um float formatado como string ASCII de exatos 6 bytes.
     * @details Otimização: Usa resize + memcpy para evitar múltiplos push_back.
     * @param value O valor float a ser convertido.
     */
    inline void __write_float_val(float value) {
        char temp[16]; // Buffer pequeno na stack é rápido

        // Formata o float. O padrão %f garante casas decimais suficientes.
        std::snprintf(temp, sizeof(temp), "%f", value);

        // Otimização: Em vez de loop, expandimos o vetor e copiamos memória direta.
        size_t current_size = this->__buffer.size();
        this->__buffer.resize(current_size + 6);

        // Copia estritamente os primeiros 6 caracteres para o buffer
        std::memcpy(this->__buffer.data() + current_size, temp, 6);
    }

    /**
     * @brief Converte e escreve cores RGB (0.0-1.0) para bytes (0-255).
     * @param r Componente Vermelho.
     * @param g Componente Verde.
     * @param b Componente Azul.
     */
    inline void __write_color(float r, float g, float b) {
        // Clamping manual para segurança (garante 0-255)
        if (r < 0.0f) r = 0.0f; else if (r > 1.0f) r = 1.0f;
        if (g < 0.0f) g = 0.0f; else if (g > 1.0f) g = 1.0f;
        if (b < 0.0f) b = 0.0f; else if (b > 1.0f) b = 1.0f;

        this->__buffer.push_back(static_cast<unsigned char>(r * 255.0f));
        this->__buffer.push_back(static_cast<unsigned char>(g * 255.0f));
        this->__buffer.push_back(static_cast<unsigned char>(b * 255.0f));
    }

    /**
     * @brief Converte e escreve cores RGBA (0.0-1.0) para bytes (0-255).
     * @param r Componente Vermelho.
     * @param g Componente Verde.
     * @param b Componente Azul.
     * @param a Componente Alpha (Transparência).
     */
    inline void __write_color_alpha(float r, float g, float b, float a) {
        this->__write_color(r, g, b);

        if (a < 0.0f) a = 0.0f; else if (a > 1.0f) a = 1.0f;
        this->__buffer.push_back(static_cast<unsigned char>(a * 255.0f));
    }

    /**
     * @brief Escreve uma string seguida de um terminador nulo.
     * @details Otimização: Usa insert iterador para cópia em bloco.
     * @param str A string a ser escrita.
     */
    inline void __write_string(const std::string& str) {
        if (!str.empty()) {
            this->__buffer.insert(this->__buffer.end(), str.begin(), str.end());
        }
        this->__buffer.push_back(0); // Null terminator obrigatório
    }

public:
    // Remover construtores de cópia para garantir Singleton
    Drawer(const Drawer&) = delete;
    void operator=(const Drawer&) = delete;

    /**
     * @brief Obtém a instância única da classe (Singleton).
     * @return Referência estática para o Drawer.
     */
    static Drawer& get_instance() {
        static Drawer instance;
        return instance;
    }

    /**
     * @brief Limpa o buffer local sem enviar os dados.
     */
    void clear() {
        std::lock_guard<std::mutex> lock(this->__mutex);
        this->__buffer.clear();
    }

    /**
     * @brief Envia o conteúdo do buffer via UDP para o RoboViz.
     * @return True se enviou bytes com sucesso, False se o buffer estava vazio ou houve erro.
     */
    bool flush() {
        std::lock_guard<std::mutex> lock(this->__mutex);
        if(this->__buffer.empty()){ return false; }

        ssize_t sent = sendto(
            this->__socket_fd,
            this->__buffer.data(),
            this->__buffer.size(),
            0,
            (struct sockaddr*)&this->__dest_addr,
            sizeof(this->__dest_addr)
        );

        this->__buffer.clear();
        return sent > 0;
    }

    // --- Comandos de Desenho (API Pública) ---

    /**
     * @brief Envia o comando para renderizar os desenhos de um conjunto específico.
     * @param set Nome do conjunto (layer) a ser atualizado no visualizador.
     */
    void swap_buffers(const std::string& set) {
        std::lock_guard<std::mutex> lock(this->__mutex);
        this->__write_byte(0);
        this->__write_byte(0);
        this->__write_string(set);
    }

    /**
     * @brief Adiciona o comando de desenho de uma linha ao buffer.
     * @param x1 Coordenada X inicial.
     * @param y1 Coordenada Y inicial.
     * @param z1 Coordenada Z inicial.
     * @param x2 Coordenada X final.
     * @param y2 Coordenada Y final.
     * @param z2 Coordenada Z final.
     * @param thickness Espessura da linha.
     * @param r Cor Vermelha (0.0 - 1.0).
     * @param g Cor Verde (0.0 - 1.0).
     * @param b Cor Azul (0.0 - 1.0).
     * @param set Nome do conjunto.
     */
    void draw_line(
        float x1, float y1, float z1,
        float x2, float y2, float z2,
        float thickness,
        float r, float g, float b,
        const std::string& set
    ) {
        std::lock_guard<std::mutex> lock(this->__mutex);
        this->__write_byte(1); // Cmd Principal
        this->__write_byte(1); // Sub Cmd (Line)
        this->__write_float_val(x1); this->__write_float_val(y1); this->__write_float_val(z1);
        this->__write_float_val(x2); this->__write_float_val(y2); this->__write_float_val(z2);
        this->__write_float_val(thickness);
        this->__write_color(r, g, b);
        this->__write_string(set);
    }

    /**
     * @brief Adiciona o comando de desenho de um círculo (2D/Billboard) ao buffer.
     * @param x Centro X.
     * @param y Centro Y.
     * @param radius Raio.
     * @param thickness Espessura da linha.
     * @param r Cor Vermelha.
     * @param g Cor Verde.
     * @param b Cor Azul.
     * @param set Nome do conjunto.
     */
    void draw_circle(
        float x, float y,
        float radius,
        float thickness,
        float r, float g, float b,
        const std::string& set
    ) {
        std::lock_guard<std::mutex> lock(this->__mutex);
        this->__write_byte(1);
        this->__write_byte(0); // Sub Cmd (Circle)
        this->__write_float_val(x); this->__write_float_val(y);
        this->__write_float_val(radius);
        this->__write_float_val(thickness);
        this->__write_color(r, g, b);
        this->__write_string(set);
    }

    /**
     * @brief Adiciona o comando de desenho de uma esfera ao buffer.
     * @param x Centro X.
     * @param y Centro Y.
     * @param z Centro Z.
     * @param radius Raio.
     * @param r Cor Vermelha.
     * @param g Cor Verde.
     * @param b Cor Azul.
     * @param set Nome do conjunto.
     */
    void draw_sphere(float x, float y, float z, float radius,
                     float r, float g, float b, const std::string& set) {
        std::lock_guard<std::mutex> lock(this->__mutex);
        this->__write_byte(1);
        this->__write_byte(3); // Sub Cmd (Sphere)
        this->__write_float_val(x); this->__write_float_val(y); this->__write_float_val(z);
        this->__write_float_val(radius);
        this->__write_color(r, g, b);
        this->__write_string(set);
    }

    /**
     * @brief Adiciona o comando de desenho de um ponto ao buffer.
     * @param x Posição X.
     * @param y Posição Y.
     * @param z Posição Z.
     * @param size Tamanho do ponto.
     * @param r Cor Vermelha.
     * @param g Cor Verde.
     * @param b Cor Azul.
     * @param set Nome do conjunto.
     */
    void draw_point(float x, float y, float z, float size,
                    float r, float g, float b, const std::string& set) {
        std::lock_guard<std::mutex> lock(this->__mutex);
        this->__write_byte(1);
        this->__write_byte(2); // Sub Cmd (Point)
        this->__write_float_val(x); this->__write_float_val(y); this->__write_float_val(z);
        this->__write_float_val(size);
        this->__write_color(r, g, b);
        this->__write_string(set);
    }

    /**
     * @brief Adiciona o comando de desenho de um polígono ao buffer.
     * @param verts Vetor contendo as coordenadas dos vértices (x, y, z sequenciais).
     * @param r Cor Vermelha.
     * @param g Cor Verde.
     * @param b Cor Azul.
     * @param a Transparência (Alpha).
     * @param set Nome do conjunto.
     */
    void draw_polygon(const std::vector<float>& verts, float r, float g, float b, float a, const std::string& set) {
        std::lock_guard<std::mutex> lock(this->__mutex);
        unsigned char num_verts = static_cast<unsigned char>(verts.size() / 3);

        this->__write_byte(1);
        this->__write_byte(4); // Sub Cmd (Polygon)
        this->__write_byte(num_verts);
        this->__write_color_alpha(r, g, b, a);

        for(float v : verts){ this->__write_float_val(v); }
        this->__write_string(set);
    }

    /**
     * @brief Adiciona uma anotação de texto 3D ao buffer.
     * @param text O texto a ser exibido.
     * @param x Posição X.
     * @param y Posição Y.
     * @param z Posição Z.
     * @param r Cor Vermelha.
     * @param g Cor Verde.
     * @param b Cor Azul.
     * @param set Nome do conjunto.
     */
    void draw_annotation(const std::string& text, float x, float y, float z,
                         float r, float g, float b, const std::string& set) {
        std::lock_guard<std::mutex> lock(this->__mutex);
        this->__write_byte(2); // Cmd Principal (Annotation)
        this->__write_byte(0); // Sub Cmd
        this->__write_float_val(x); this->__write_float_val(y); this->__write_float_val(z);
        this->__write_color(r, g, b);
        this->__write_string(text);
        this->__write_string(set);
    }
};