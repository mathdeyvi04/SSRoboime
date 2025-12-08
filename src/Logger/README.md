# Fluxo de Operação da Classe Logger

```
 [THREAD PRINCIPAL / JOGO]                       [THREAD LOGGER (WORKER)]
            |                                                |
            | (O robô está rodando...)                       | (Dormindo no cv.wait)
            |                                                |
 1. CHAMADA | logger.info("Msg")                             |
            |                                                |
 2. PREPARO | Formata string e Timestamp                     |
            | (Operação de CPU pura)                         |
            |                                                |
 3. LOCK    | Bloqueia __mutex  [>>]                         |
            |                     ||                         |
 4. PUSH    | Insere no           ||                         |
            | __current_buffer    ||                         |
            | (RAM - Muito Rápido)||                         |
            |                     ||                         |
 5. UNLOCK  | Libera __mutex    [<<]                         |
            |                                                |
 6. NOTIFY  | __cv.notify_one() ---------------------------> | 7. ACORDA (Wake Up)
            |                                                |
            | (Volta a rodar o jogo                          | 8. LOCK Mutex [>>]
            |  imediatamente. Performance                    |                ||
            |  preservada!)                                  |                ||
            |                                                | 9. SWAP (A Mágica)
            |                                                |    Troca ponteiros:
            |                                                |    __current (Cheio) <-> __write (Vazio)
            |                                                |                ||
            |                                                | 10. UNLOCK Mutex [<<]
            |                                                |
            |                                                | (A partir daqui, o Logger não
            |                                                |  atrapalha a Principal mesmo
  (Tempo)   |                                                |  sem Lock)
    ||      |                                                |
    ||      |                                                | 11. I/O PESADO
    \/      |                                                |     Itera sobre __write_buffer
            |                                                |     Escreve no arquivo .log
            |                                                |     Flush do arquivo
            |                                                |
            |                                                | 12. LIMPEZA
            |                                                |     __write_buffer.clear()
            |                                                |
            |                                                | 13. LOOP
            |                                                |     Volta a dormir (cv.wait)
            |                                                |
```