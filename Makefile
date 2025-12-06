gdb:
	@g++ -g -std=c++20 src/run_full_team.cpp; gdb ./a.out; rm a.out;


.PHONY: docs
docs:
	@echo ">>> Criando documentação..."
	@doxygen Doxyfile
	@$(MAKE) -C docs/latex
	@mv docs/latex/refman.pdf Documentation.pdf
