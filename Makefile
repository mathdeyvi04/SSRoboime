gdb_threads:
	@g++ -g -std=c++20 -pthread src/run_full_threads.cpp; gdb ./a.out; rm a.out;

gdb:
	@g++ -g -std=c++20 src/run_full_team.cpp; gdb ./a.out; rm a.out;

gdb_player:
	@g++ -g -std=c++20 src/run_player.cpp; gdb ./a.out; rm a.out;


.PHONY: docs
docs:
	@echo ">>> Criando documentação..."
	@doxygen Doxyfile
	@$(MAKE) -C docs/latex
	@mv docs/latex/refman.pdf Documentation.pdf
	@rm -r docs
