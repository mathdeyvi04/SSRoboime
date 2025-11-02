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


.PHONY: run
run:

