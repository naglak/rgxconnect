GCC := g++
OUTPUT := rgxconnect 
SOURCES := rgxconnect.cpp jconnector.cpp
HEADERS := jconnector.h
CCFLAGS := -ljack -pthread

all: $(OUTPUT)
	
$(OUTPUT):$(SOURCES) $(HEADERS)
	$(GCC) -Wall -o $(OUTPUT) $(CCFLAGS) $(SOURCES)
	
clean:
	rm $(OUTPUT)
	
.PHONY: all
