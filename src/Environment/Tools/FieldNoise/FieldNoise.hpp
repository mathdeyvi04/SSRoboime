#pragma once

#include <cmath>

class FieldNoise {
public:

    /**
     * @brief Log Prob de uma distância real d, dada uma distância ruidosa r.
     */
    static double log_prob_r(double d, double r){
        return log_prob_normal_distribution(
            0,
            0.0965,
            100.0 * ((r-0.005)/d - 1),
            100.0 * ((r+0.005)/d - 1)
        );
    }

    /**
     * @brief Log Prob de um ângulo horizontal real h, dada um ângulo ruidoso phi.
     */
    static double log_prob_h(double h, double phi){
        return log_prob_normal_distribution(
            0,
            0.1225,
            phi - 0.005 - h,
            phi + 0.005 - h
        );
    }

    /**
     * @brief Log Prob de um ângulo vertical real v, dada um ângulo ruidoso theta.
     */
    static double log_prob_v(double v, double theta){
        return log_prob_normal_distribution(
            0,
            0.1480,
            theta - 0.005 - v,
            theta + 0.005 - v
        );
    }

private:
    /// Não será necessário instâncias
    FieldNoise(){};

    /**
     * @brief Calcula o logaritmo da probabilidade em um intervalo de uma distribuição normal.
     * * Implementa uma abordagem híbrida de três estágios para garantir precisão numérica
     * desde a média até as caudas mais extremas da distribuição.
     * *  **Estágio 1 (Normal):** Usa `std::erf` para diferenças próximas à média.
     * - **Estágio 2 (Cauda):** Usa `std::erfc` para manter precisão até ~27 desvios padrão.
     * - **Estágio 3 (Extremo):** Usa aproximação assintótica (Log-PDF) para evitar underflow
     * em probabilidades menores que 1e-308 (limite do double).
     * * @param mean A média da distribuição.
     * @param std O desvio padrão da distribuição.
     * @param lim_inf Um dos limites do intervalo de integração.
     * @param lim_sup O outro limite do intervalo de integração.
     * @return double O logaritmo natural da probabilidade: ln(P(lim_inf <= X <= lim_sup)).
     */
    static double log_prob_normal_distribution(const double& mean, const double& std, const double& lim_inf, const double& lim_sup) {

        // Constantes Matemáticas Estáticas
        static constexpr double SQRT2 = 1.41421356237309504880;
        static constexpr double LOG_05 = -0.69314718055994530941; // ln(0.5)

        // Constante para a Zona 3: ln(sqrt(pi))
        // Derivação: ln(PDF) + ln(delta) requer termos que simplificam para ln(sqrt(pi))
        static constexpr double LOG_SQRT_PI = 0.57236494292470008707;

        // O argumento da erf é (x - mu) / (sigma * sqrt(2)).
        // Calculamos o inverso disso para usar multiplicação (mais rápido que divisão).
        const double inv_denom = 1.0 / (std * SQRT2);

        // Z-scores normalizados para erf/erfc (já divididos por sqrt(2))
        double z1 = (lim_inf - mean) * inv_denom;
        double z2 = (lim_sup - mean) * inv_denom;

        // Pré-cálculo dos valores absolutos (usados em todas as ramificações)
        double abs_z1 = std::fabs(z1);
        double abs_z2 = std::fabs(z2);

        // --- 1. ZONA SEGURA (Perto da média, |z| < 1) ---
        // A precisão da erf padrão é perfeita aqui.
        double diff = std::fabs(std::erf(z1) - std::erf(z2));
        // Verificação rápida para evitar log(0) se intervalos forem iguais
        if(diff > 0.0){
            return std::log(diff) + LOG_05;
        }
        // Se diff <= 0, cai para o retorno de erro no final da função

        // --- 2. ZONA DE CAUDA (Usando erfc) ---
        // Calcula |erfc(|z1|) - erfc(|z2|)|
        // Usamos fabs nos inputs pois erfc(x) = 2 - erfc(-x), a diferença simétrica se mantém.
        double erfc1 = std::erfc(abs_z1);
        double erfc2 = std::erfc(abs_z2);

        double diff = std::fabs(erfc1 - erfc2);

        // Se o double conseguiu representar a diferença (> 1e-308), retornamos.
        if (diff > std::numeric_limits<double>::min()) {
            return std::log(diff) + LOG_05;
        }

        // --- 3. ZONA EXTREMA (Underflow Assintótico) ---
        // Se chegamos aqui, diff == 0.0. Estamos na cauda longínqua (Z > ~26).
        // A probabilidade é minúscula. Usamos a aproximação retangular:
        // P ~= PDF(z_closer) * largura_do_intervalo

        // Determina quem está mais perto do pico (menor Z absoluto tem maior probabilidade)
        double z_closer_norm = std::min(abs_z1, abs_z2);

        // Calcula a largura do intervalo normalizado
        double delta_z_norm = std::fabs(z1 - z2);

        if(delta_z_norm == 0.0){ return -std::numeric_limits<double>::infinity(); }

        // FÓRMULA OTIMIZADA LOG-PDF:
        // Originalmente: ln(P) = -0.5 * z_raw^2 - ln(sqrt(2pi)) + ln(width_raw)
        // Substituindo z_raw = z_norm * sqrt(2) e simplificando as constantes:
        // Resulta em: -z_norm^2 - ln(sqrt(pi)) + ln(delta_z_norm)

        return -(z_closer_norm * z_closer_norm) - LOG_SQRT_PI + std::log(delta_z_norm);
    }
};





