from tarfile import TruncatedHeaderError

with open(
    "medidas_de_frente_para_F1L.txt",
    "r"
) as f:
    dados = f.read().split('\n')

errors_from_F1L = [
    [
        int(linha.split("-")[0]),
        round((0.5 + 1.5 * (int(linha.split("-")[0]) - 1)) - float(linha.split("-")[1]), 3)
    ] for linha in dados
]

with open(
    "medidas_de_frente_para_F1R.txt",
    "r"
) as f:
    dados = f.read().split('\n')

errors_from_F1R = [
    [
        int(linha.split("-")[0]),
        round((15.5 + 1.5 * (int(linha.split("-")[0]) - 1)) - float(linha.split("-")[1]), 3)
    ] for linha in dados
]

import statistics as stats

valores_de_distancia = []
valores_de_erro_media = []
valores_de_erro_desv = []
listas_a_serem_verificadas = [
    errors_from_F1L,
    errors_from_F1R
]
for init_dist in [0.5, 15.5]:
    for i in range(1, 11):
        valores_de_distancia.append(
            init_dist + (i - 1) * 1.5
        )
        valores_de_erro_media.append(
            stats.mean(
                [
                    valor_do_erro for unum, valor_do_erro in listas_a_serem_verificadas[init_dist == 15.5] if unum == i
                ]
            )
        )
        valores_de_erro_desv.append(
            stats.pstdev(
                [
                    valor_do_erro for unum, valor_do_erro in listas_a_serem_verificadas[init_dist == 15.5] if unum == i
                ]
            )
        )

from matplotlib import pyplot as pp
import numpy as np

pp.figure(figsize=(8, 5))
pp.errorbar(
    valores_de_distancia,
    valores_de_erro_media,
    yerr=valores_de_erro_desv,
    fmt='o', color='blue', ecolor='red', capsize=5, label='Média e Desvio Padrão do Erro Na Distância'
)
pp.axhline(y=stats.mean(valores_de_erro_media[1:]), color='green', linestyle='--', linewidth=1, label=f'Média do Erro Geral: {stats.mean(valores_de_erro_media[1:]):.5f}')
pp.xlabel("Distância Real")
pp.ylabel("Erro Associado")
pp.title("Medidas de Erro de Estimativas de Distância")
pp.legend()
pp.grid(True)
pp.show()

coef = np.polyfit(valores_de_distancia, valores_de_erro_desv, 1)
reta_ajustada = np.poly1d(coef)

pp.figure(figsize=(8, 5))
pp.scatter(
    valores_de_distancia, valores_de_erro_desv
)
pp.plot(
    valores_de_distancia,
    reta_ajustada(valores_de_distancia),
    color="red",
    linestyle="--",
    label=f'Reg Linear: y={coef[0]:.5f}x + {coef[1]:.5f}'
)
pp.xlabel("Distância Real")
pp.ylabel("Desvio Padrão do Erro Associado")
pp.title("Medidas de Desvio Padrão do Erro na Distância")
pp.legend()
pp.grid(True)
pp.show()




