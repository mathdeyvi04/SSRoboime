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
	@python3 src/exec_booting.py
	@# Atente-se à como referencia os arquivos, pois eles podem virar pastas.
	@pyinstaller \
	--add-data "./src/term/config_team_params.txt:term" \
	--add-data "./src/agent/tactical_formation.pkl:agent" \
	src/run_full_team.py \
	--onefile \
	--noconfirm \
	--name run_full_team
	@mv dist/run_full_team ./
	@rm -rf build/ dist/ *.spec
