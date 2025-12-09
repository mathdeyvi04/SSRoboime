gdb_threads:
	@g++ -g -O0 -std=c++20 -pthread src/run_full_threads.cpp; gdb ./a.out; rm a.out;

gdb:
	@g++ -g -O0 -std=c++20 src/run_full_team.cpp; gdb ./a.out; rm a.out;

gdb_player:
	@g++ -g -O0 -std=c++20 src/run_player.cpp; gdb ./a.out; rm a.out;

debug_vision:
	@g++ -g -O0 -std=c++20 src/run_full_team.cpp -DENABLE_DEBUG_VISION; gdb ./a.out; rm a.out;

.PHONY: docs
docs:
	@echo ">>> Criando documentação..."
	@doxygen Doxyfile
	@$(MAKE) -C docs/latex
	@mv docs/latex/refman.pdf Documentation.pdf
	@rm -r docs
