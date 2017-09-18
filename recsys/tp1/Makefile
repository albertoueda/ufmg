CC = g++
CC_FLAGS = -std=c++11 -O3 -w -fpermissive #-g -DDEBUG

EXEC = recommender
SOURCES = $(wildcard *.cpp) 
OBJECTS = $(SOURCES:.cpp=.o)

# Main target
$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o  $(EXEC)

# To obtain object files
%.o: %.cpp
	$(CC) -c $(CC_FLAGS) $< -o $@

all:	$(EXEC)

# It is expected that both files exist in current directory  
run:
	./recommender ratings.csv targets.csv > submission.csv
	
clean:
	rm -f $(EXEC) $(OBJECTS)

	