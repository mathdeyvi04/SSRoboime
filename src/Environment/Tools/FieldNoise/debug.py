from matplotlib import pyplot as plt
from mpmath import mp
import pandas as pd

# Definimos a precisão que desejamos para ter o valor absoluto
mp.dps = 50

def get_true_value(x_pos):
    # Intervalo do teste: [x, x+step]
    step = 0.5

    start = x_pos
    end = x_pos + step

    # --- ALTERAÇÃO AQUI ---
    # Definindo o desvio padrão desejado
    sigma = mp.mpf('0.1480')

    # O denominador do argumento da erf deve ser (sigma * sqrt(2))
    denom = sigma * mp.sqrt(2)

    # Normalização
    z_start = mp.mpf(start) / denom
    z_end = mp.mpf(end) / denom

    # Cálculo da probabilidade usando erfc (Complementary Error Function)
    # Usamos erfc porque nas caudas (z grande), erfc mantém a precisão
    # enquanto (1 - erf) perderia dígitos significativos.
    # Prob = 0.5 * (erfc(z_start) - erfc(z_end))
    prob = 0.5 * (mp.erfc(z_start) - mp.erfc(z_end))

    try:
        # Convertemos de volta para float padrão do Python apenas no final (para o gráfico)
        # Se for muito pequeno, pode dar erro de domínio no log, por isso o try/except
        return float(mp.log(prob))
    except:
        # Isso ocorre se a probabilidade for literalmente 0 (o que requer x > 1000 com mpmath)
        return None

try:
    df = pd.read_csv('benchmark_data.txt')
except FileNotFoundError:
    print("Erro: O arquivo 'benchmark_data.txt' não foi encontrado.")
    print("Certifique-se de rodar o código C++ primeiro.")
    exit()

df['true_value'] = df['x_pos'].apply(get_true_value)

plt.figure(figsize=(12, 7))

# Linha da Verdade (Preto espesso e transparente
plt.plot(df['x_pos'][df.index], df['true_value'][df.index], 'k-', lw=4, alpha=0.3, label='Verdadeiro (mpmath)')

# Linhas das Funções C++
plt.plot(df['x_pos'], df['naive'], 'r--', label='Naive (erf)')
plt.plot(df['x_pos'], df['hybrid'], 'g:', lw=2, label='Hybrid (erfc)')
plt.plot(df['x_pos'][df.index < 26], df['custom'][df.index < 26], 'b-.', label='Custom')

plt.title('Log-Prob com std=0.1480')
plt.xlabel(f'Posição x, Intervalo [x, x + 0.5]')
plt.ylabel('Log Probabilidade')
plt.legend()
plt.grid(True, linestyle='--', alpha=0.6)

# Ajustar limites para visualização (opcional, dependendo de quão rápido cai)
# plt.ylim(-800, 10)

plt.tight_layout()
plt.show()