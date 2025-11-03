# Dependências do Ambiente Virtual
VENV_DIR := venv
PYTHON := ./venv/bin/python3
PIP := ./venv/bin/pip
REQ_FILE := requirements.txt

.PHONY: init
init:
	@echo ">>> Criando ambiente virtual..."
	@python3 -m venv $(VENV_DIR)
	@echo ">>> Atualizando pip..."
	@$(PYTHON) -m pip install --upgrade pip
	@echo ">>> Instalando requirements.txt"
	@$(PIP) install -r $(REQ_FILE)

.PHONY: install
install:
	@if [ -f "$(REQ_FILE)" ]; then \
		echo ">>> Instalando pacotes de $(REQ_FILE)..."; \
		$(PIP) install -r $(REQ_FILE); \
	else \
		echo ">>> Nenhum requirements.txt encontrado."; \
	fi

.PHONY: docs
docs:
	@echo ">>> Criando documentação..."
	@doxygen Doxyfile
	@$(MAKE) -C docs/latex
	@mv docs/latex/refman.pdf Documentation.pdf

.PHONY: build
build:
	@echo ">>> Buildando aplicação..."
	# Vamos preencher conforme necessidade
	@pyinstaller \
				--onefile $(SRC) \
				--name $(NOME)
	@mv dist/$(NOME) ./$(NOME)
	@rm -rf build dist $(NOME).spec
