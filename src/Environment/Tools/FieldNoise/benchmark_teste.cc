#include "FieldNoise.hpp"
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <iomanip>
#include <limits>

double log_prob_naive(double mean, double std, double interval1, double interval2) {
    const double SQRT2 = std::sqrt(2.0);
    const double LOG_05 = std::log(0.5);

    // Z-scores
    double z1 = (mean - interval1) / (std * SQRT2);
    double z2 = (mean - interval2) / (std * SQRT2);

    // FIX: Usar std::fabs para garantir que a diferença seja positiva
    // Probabilidade = |erf(z1) - erf(z2)| / 2
    double diff = std::fabs(std::erf(z1) - std::erf(z2));

    if (diff <= 0.0) return -std::numeric_limits<double>::infinity();
    return std::log(diff) + LOG_05;
}

static double erf_aux(double a){
    double r, s, t, u;

    t = fabs (a);
    s = a * a;

    r = fma (-5.6271698458222802e-018, t, 4.8565951833159269e-016);
    u = fma (-1.9912968279795284e-014, t, 5.1614612430130285e-013);
    r = fma (r, s, u);
    r = fma (r, t, -9.4934693735334407e-012);
    r = fma (r, t,  1.3183034417266867e-010);
    r = fma (r, t, -1.4354030030124722e-009);
    r = fma (r, t,  1.2558925114367386e-008);
    r = fma (r, t, -8.9719702096026844e-008);
    r = fma (r, t,  5.2832013824236141e-007);
    r = fma (r, t, -2.5730580226095829e-006);
    r = fma (r, t,  1.0322052949682532e-005);
    r = fma (r, t, -3.3555264836704290e-005);
    r = fma (r, t,  8.4667486930270974e-005);
    r = fma (r, t, -1.4570926486272249e-004);
    r = fma (r, t,  7.1877160107951816e-005);
    r = fma (r, t,  4.9486959714660115e-004);
    r = fma (r, t, -1.6221099717135142e-003);
    r = fma (r, t,  1.6425707149019371e-004);
    r = fma (r, t,  1.9148914196620626e-002);
    r = fma (r, t, -1.0277918343487556e-001);
    r = fma (r, t, -6.3661844223699315e-001);
    r = fma (r, t, -1.2837929411398119e-001);
    r = fma (r, t, -t);

    return r;
}

double log_prob_normal_custom(double mean, double std, double interval1, double interval2) {

    static const double SQRT2 = std::sqrt(2.0);
    static const double LOG_05 = std::log(0.5);

    // Otimização: inverso para multiplicar
    const double inv_denom = 1.0 / (std * SQRT2);

    double erf1_x = (mean - interval1) * inv_denom;
    double erf2_x = (mean - interval2) * inv_denom;

    // --- ZONA SEGURA (Perto da média) ---
    // Se |z| < 2.0 ou se os intervalos cruzam a média (sinais opostos)
    // Usamos a diferença direta das funções de erro, garantindo valor absoluto
    if (std::fabs(erf1_x) < 2.0 || std::fabs(erf2_x) < 2.0 || ((erf1_x > 0) != (erf2_x > 0))) {
        double diff = std::fabs(std::erf(erf1_x) - std::erf(erf2_x));

        // Proteção contra log(0) caso os intervalos sejam idênticos
        if (diff <= 0.0) return -std::numeric_limits<double>::infinity();

        return std::log(diff) + LOG_05;
    }

    // --- ZONA CRÍTICA (Caudas longas) ---
    // Aqui usamos erf_aux. Não importa se z é positivo ou negativo,
    // pois erf_aux usa fabs() internamente e retorna valores logarítmicos negativos.

    double val1 = erf_aux(erf1_x);
    double val2 = erf_aux(erf2_x);

    // ALGORITMO ROBUSTO (Log-Difference-Exp):
    // Queremos calcular log( | exp(val1) - exp(val2) | )
    // Matematicamente equivalente a: max_val + log( 1 - exp(min_val - max_val) )
    // Isso garante que o argumento do exp seja sempre negativo (seguro) e
    // o argumento do log seja sempre positivo (seguro).

    double max_val = std::max(val1, val2); // O valor "menos negativo" (maior probabilidade)
    double min_val = std::min(val1, val2); // O valor "mais negativo"

    double diff_exp = min_val - max_val; // Sempre <= 0

    // Se diff_exp for muito pequeno (ex: -50), exp será 0. log(1) será 0. Retorna max_val.
    // Se diff_exp for 0 (intervalos iguais), log(0) -> -inf.

    return max_val + std::log(1.0 - std::exp(diff_exp)) + LOG_05;
}

static double log_prob_hybrid(double mean, double std, double interval1, double interval2) {

    // Constantes Matemáticas Estáticas
    static constexpr double SQRT2 = 1.41421356237309504880;
    static constexpr double LOG_05 = -0.69314718055994530941; // ln(0.5)

    // Constante para a Zona 3: ln(sqrt(pi))
    // Derivação: ln(PDF) + ln(delta) requer termos que simplificam para ln(sqrt(pi))
    static constexpr double LOG_SQRT_PI = 0.57236494292470008707;

    // OTIMIZAÇÃO: Combina divisões.
    // O argumento da erf é (x - mu) / (sigma * sqrt(2)).
    // Calculamos o inverso disso para usar multiplicação (mais rápido que divisão).
    const double inv_denom = 1.0 / (std * SQRT2);

    // Z-scores normalizados para erf/erfc (já divididos por sqrt(2))
    double z1 = (interval1 - mean) * inv_denom;
    double z2 = (interval2 - mean) * inv_denom;

    // Pré-cálculo dos valores absolutos (usados em todas as ramificações)
    double abs_z1 = std::fabs(z1);
    double abs_z2 = std::fabs(z2);

    // --- 1. ZONA SEGURA (Perto da média, |z| < 1) ---
    // A precisão da erf padrão é perfeita aqui.
    double diff = std::fabs(std::erf(z1) - std::erf(z2));

    // Verificação rápida para evitar log(0) se intervalos forem iguais
    if (diff > 0.0) {
        return std::log(diff) + LOG_05;
    }
    // Se diff <= 0, cai para o retorno de erro no final da função
    // --- 2. ZONA DE CAUDA (Usando erfc) ---
    // Calcula |erfc(|z1|) - erfc(|z2|)|
    // Usamos fabs nos inputs pois erfc(x) = 2 - erfc(-x), a diferença simétrica se mantém.
    double erfc1 = std::erfc(abs_z1);
    double erfc2 = std::erfc(abs_z2);

    diff = std::fabs(erfc1 - erfc2);

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

    if (delta_z_norm == 0.0) return -std::numeric_limits<double>::infinity();

    // FÓRMULA OTIMIZADA LOG-PDF:
    // Originalmente: ln(P) = -0.5 * z_raw^2 - ln(sqrt(2pi)) + ln(width_raw)
    // Substituindo z_raw = z_norm * sqrt(2) e simplificando as constantes:
    // Resulta em: -z_norm^2 - ln(sqrt(pi)) + ln(delta_z_norm)

    double term_exp = -(z_closer_norm * z_closer_norm); // -z^2

    return term_exp - LOG_SQRT_PI + std::log(delta_z_norm);
}

int main() {
    // Teste de Benchmark
    std::ofstream file("benchmark_data.txt");
    file << "x_pos,naive,hybrid,custom\n";

    // Configurações de Erro de Distância Por Exemplo
    double mean = 0.0;
    double std_dev = 0.1480;

    // Vamos testar x indo de 0 até 12 (onde a precisão quebra)
    // Para cada x, calculamos a chance de cair entre x e x+0.5
    for (double x = 0.0; x <= 12.0; x += 0.1) {

        // Intervalo: De x até x + 0.5
        // Para garantir a ordem correta na subtração interna (maior - menor área),
        // passamos o intervalo mais "distante" primeiro ou seguimos a lógica da função.

        double val_start = x;
        double val_end = x + 0.5;

        // 1. Naive
        double y_naive = log_prob_naive(mean, std_dev, val_end, val_start);

        // 2. Hybrid
        double y_hybrid = log_prob_hybrid(mean, std_dev, val_end, val_start);

        // 3. Custom
        double y_custom = log_prob_normal_custom(mean, std_dev, val_end, val_start);

        // Salvando
        file << std::fixed << std::setprecision(16)
             << x << ","
             << y_naive << ","
             << y_hybrid << ","
             << y_custom << "\n";
    }

    file.close();
    return 0;
}