all: mmcopier mscopier

mmcopier: mmcopier.cpp
	g++ -Wall -Werror -o mmcopier mmcopier.cpp -lpthread

mscopier: mscopier.cpp
	g++ -Wall -Werror -o mscopier mscopier.cpp -lpthread

clean:
	rm -f mscopier mmcopier