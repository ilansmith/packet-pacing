CFLAGS=-std=c++0x -fPIC
LFLAGS=-lstdc++ -fPIC
APPS=libpacing.so udp_client

ifeq ($(DEBUG),y)
CFLAGS+=-g
endif

%.o: %.cpp
	g++ $(CFLAGS) -c $<

all:$(APPS)

udp_client: udp_client.o libpacing.so 
	g++ $(LFLAGS) -L. -lpacing  -o $@ $<

libpacing.so: pacing.o
	g++ -shared -o $@ $< -lm	

clean:
	rm -rf *.o

cleanall: clean
	rm -rf tags $(APPS)

