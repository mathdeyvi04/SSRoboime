#pragma once

#include <iostream>
#include <cstring>

unsigned char exemplo_de_mensagem[] = {0x28, 0x74, 0x69, 0x6D, 0x65, 0x20, 0x28, 0x6E, 0x6F, 0x77, 0x20, 0x35, 0x36, 0x2E, 0x37, 0x32, 0x29, 0x29, 0x28, 0x47, 0x53, 0x20};
int size = 22;

/**
 * 
 */
class SimulatorParser {
private:

	int temp_int = 0; // Para fazermos operações temporárias

	/**
	 * @brief Utilizará uma lógica específica para printar os caracteres. Não há qualquer tipo de verificação.
	 * @param buffer Ponteiro para buffer de mensagem.
	 * @param length Comprimento de Leitura.
	 * @return Será printado na tela os caracteres correspondentes, no estilo C, usando printf.
	 */
	inline
	void
	imprimir(
		const unsigned char* buffer,
		int length
	) const { printf("%.*s", length, buffer); }

	/**
	 * @brief Verifica se o buffer de hexadecimais é igual à sequência de caracteres.
	 * @param buffer Ponteiro para buffer de comparação
	 * @param pattern Sequência de caracteres que será verificada.	
	 * @return True se for a mesma sequência, False, caso contrário.
	 */
	inline
	bool
	bytescmp(
		const unsigned char* buffer,
		const char* pattern
	) const { return std::equal(buffer, buffer + strlen(pattern), pattern); }

	/**
	 * @brief
	 * @details
	 */
	int
	get_next_tag(
		const unsigned char* buffer
	){


	}

public:

	/**
	 * @brief Realizará o parsing das informações recebidas.
	 * @param buffer Ponteiro para buffer da mensagem original.
	 * @param size Comprimento total do buffer. 
	 */
	void
	parsing(
		const unsigned char* buffer,
		int size
	){

		
	}
};
