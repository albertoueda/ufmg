CC = g++

# Fastest (not for debug)  -Ofast?
CC_FLAGS = -O3 -fpermissive -w -std=c++11 -I ./ -I htmlcxx/html/ -I ricplib/ -I crawler/    
# Debug
#CC_FLAGS = -g -O0 -lm -lz -fpermissive -w -pthread -std=c++11 -I ./ -I htmlcxx/html/ -I ricplib/
# CC_FLAGS = -O0 -pg  -W -Wall -pedantic -lm -std=c++0x -I header/
LIBS = -pthread -lcrypto -lm -lz 

#File names
EXEC = indexer
SOURCES = $(wildcard **/*.cpp) $(wildcard *.cpp) $(wildcard **/**/*.cpp) 

OBJECTS = $(SOURCES:.cpp=.o)

#Main target
$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -lz -o  $(EXEC) $(LIBS) 

# To obtain object files
%.o: %.cpp
	$(CC) -c $(CC_FLAGS) $< -o $@ $(LIBS)

run: $(EXEC)
	./$(EXEC) -c /home/itamar/estudos/mestrado/ri/data -q -w /home/itamar/estudos/mestrado/ri/www-data3 

query: $(EXEC)
	./$(EXEC) -n index.txt -c /home/itamar/estudos/mestrado/ri/data/   -w /home/itamar/estudos/mestrado/ri/www-data3/ -q -a

help: $(EXEC)
	./$(EXEC) -h

clean:
	rm -f $(EXEC) $(OBJECTS)

