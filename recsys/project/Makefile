CC = g++
CC_FLAGS = -std=c++11 -w -fpermissive -O3 #-g -DDEBUG #-O3  

EXEC = rec
SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

# Main target
$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o  $(EXEC)

# To obtain object files
%.o: %.cpp
	$(CC) -c $(CC_FLAGS) $< -o $@

all:	$(EXEC)

# It is expected that all the three files exist in current directory  
run:
	./recommender content.csv ratings.csv targets.csv > submission.csv
	
clean:
	rm -f $(EXEC) $(OBJECTS)

	