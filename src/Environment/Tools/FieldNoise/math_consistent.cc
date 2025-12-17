#include <iostream>
#include <cmath>
#include <vector>
#include <iomanip>
#include <limits>
#include <string>

// Supondo que o arquivo header esteja disponível
#include "FieldNoise.hpp"

class UnitTest {
public:
    // ============================================================================
    // UTILITÁRIOS DE TESTE
    // ============================================================================

    bool is_approx(double a, double b, double epsilon = 1e-7) {
        return std::fabs(a - b) < epsilon;
    }

    void print_result(std::string title, bool passed, std::string details = "") {
        std::cout << "[" << (passed ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "] "
                  << std::left << std::setw(50) << title
                  << details << std::endl;
    }

    // ============================================================================
    // TESTES MATEMÁTICOS GERAIS (DA GAUSSIANA)
    // ============================================================================

    /**
     * @brief TESTE 1: Normalização (Área Total = 1)
     */
    void test_normalization() {
        double mean = 5.0;
        double std = 2.0;
        double inf_neg = -1e9;
        double inf_pos = 1e9;

        double log_area = FieldNoise::log_prob_normal_distribution(mean, std, inf_neg, inf_pos);

        bool passed = is_approx(log_area, 0.0, 1e-5);
        print_result("Axioma: Area Total == 1 (Log=0)", passed,
            passed ? "" : "Obtido: " + std::to_string(log_area));
    }

    /**
     * @brief TESTE 2: Simetria da Gaussiana
     */
    void test_symmetry() {
        double mean = 0.0;
        double std = 1.0;
        double log_prob_right = FieldNoise::log_prob_normal_distribution(mean, std, 1.0, 2.0);
        double log_prob_left = FieldNoise::log_prob_normal_distribution(mean, std, -2.0, -1.0);

        bool passed = is_approx(log_prob_right, log_prob_left);
        print_result("Propriedade: Simetria Espelhada", passed);
    }

    /**
     * @brief TESTE 3: Aditividade de Intervalos Disjuntos
     */
    void test_additivity() {
        double mean = 0.0;
        double std = 1.0;
        double lp_A = FieldNoise::log_prob_normal_distribution(mean, std, 0.0, 0.5);
        double lp_B = FieldNoise::log_prob_normal_distribution(mean, std, 0.5, 1.0);
        double lp_Total = FieldNoise::log_prob_normal_distribution(mean, std, 0.0, 1.0);

        double sum_p = std::exp(lp_A) + std::exp(lp_B);
        double total_p = std::exp(lp_Total);

        bool passed = is_approx(sum_p, total_p);
        print_result("Axioma: Aditividade (Soma das Partes)", passed);
    }

    /**
     * @brief TESTE 4: Invariância de Escala (Z-Score)
     */
    void test_scale_invariance() {
        double lp_1 = FieldNoise::log_prob_normal_distribution(0.0, 1.0, -1.0, 1.0);
        double lp_2 = FieldNoise::log_prob_normal_distribution(100.0, 50.0, 50.0, 150.0);

        bool passed = is_approx(lp_1, lp_2);
        print_result("Propriedade: Invariancia de Escala", passed);
    }

    // ============================================================================
    // TESTES ESPECÍFICOS DE DISTÂNCIA (log_prob_r)
    // ============================================================================

    /**
     * @brief TESTE 5: Maxima Verossimilhança em log_prob_r
     * Verifica se a probabilidade é máxima quando a leitura (r) é igual à distância real (d).
     */
    void test_r_maximum_likelihood() {
        double dist = 10.0;

        // Caso perfeito: r == d
        double p_exact = FieldNoise::log_prob_r(dist, dist);

        // Caso com erro: r == d + 10cm
        double p_error = FieldNoise::log_prob_r(dist, dist + 0.10);

        // O valor exato deve ter probabilidade maior (menos negativa em log)
        bool passed = p_exact > p_error;

        print_result("Distancia: Maxima Verossimilhança (Pico)", passed,
             passed ? "" : "P_exact: " + std::to_string(p_exact) + " P_error: " + std::to_string(p_error));
    }

    /**
     * @brief TESTE 6: Decaimento com Distância (Modelo Relativo)
     * Como o modelo de ruído é percentual, a incerteza absoluta cresce com a distância.
     * Portanto, a probabilidade de acertar dentro de uma janela fixa (ex: +/- 5mm)
     * deve ser muito menor a 100m do que a 1m.
     */
    void test_r_distance_decay() {
        // A 1 metro, +/- 5mm é uma fatia razoável da curva.
        double p_near = FieldNoise::log_prob_r(1.0, 1.0);

        // A 100 metros, +/- 5mm é uma fatia minúscula (pois o desvio padrão em metros é enorme).
        double p_far  = FieldNoise::log_prob_r(100.0, 100.0);

        // Esperamos que Prob(perto) > Prob(longe)
        bool passed = p_near > p_far;

        print_result("Distancia: Decaimento com Distancia", passed,
            passed ? "" : "Perto: " + std::to_string(p_near) + " Longe: " + std::to_string(p_far));
    }

    /**
     * @brief TESTE 7 CORRIGIDO: Consistência de Erro Relativo (Perto da Média)
     * Usamos um erro pequeno (0.1%) para evitar que a diferença infinitesimal
     * nos limites de integração seja amplificada pela inclinação da cauda extrema.
     */
    void test_r_relative_consistency() {
        // Erro de 0.1% (Z-score ~ 1.0, região segura/estável)

        // 10m -> Erro 0.1% -> Leitura 10.01m
        double p_10m = FieldNoise::log_prob_r(10.0, 10.01);

        // 100m -> Erro 0.1% -> Leitura 100.1m
        double p_100m = FieldNoise::log_prob_r(100.0, 100.1);

        // A diferença teórica deve ser apenas log(10) devido à largura da janela
        double expected_diff = std::log(10.0);
        double actual_diff = p_10m - p_100m;

        // A tolerância ainda precisa ser relaxada um pouco (0.2) pois o efeito
        // da janela híbrida (Absoluta/Relativa) ainda existe, mas é menor.
        bool passed = is_approx(actual_diff, expected_diff, 0.2);

        print_result("Distancia: Consistencia Percentual (Small Error)", passed,
            passed ? "" : "Diff real: " + std::to_string(actual_diff) + " Esperado: " + std::to_string(expected_diff));
    }

    void execute_testes() {
        std::cout << "=== Bateria de Testes Matematicos Probabilisticos ===" << std::endl;
        std::cout << "--- Nucleo Gaussiano ---" << std::endl;
        test_normalization();
        test_symmetry();
        test_additivity();
        test_scale_invariance();

        std::cout << "\n--- Modelo de Sensor de Distancia (log_prob_r) ---" << std::endl;
        test_r_maximum_likelihood();
        test_r_distance_decay();
        test_r_relative_consistency();
        std::cout << "=====================================================" << std::endl;
    }
};


int main() {
    UnitTest ut;
    ut.execute_testes();
    return 0;
}