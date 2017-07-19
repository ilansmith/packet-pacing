CFLAGS=-std=c++0x
LFLAGS=-lstdc++
APPS=udp_client udp_server

ifeq ($(DEBUG),y)
CFLAGS+=-g
endif

%.o: %.cpp
	g++ $(CFLAGS) -c $<

all:$(APPS)

udp_client: udp_client.o
	g++ -o $@ $^ $(LFLAGS)

udp_server: udp_server.o
	g++ -o $@ $^ $(LFLAGS)

clean:
	rm -rf *.o

cleanall: clean
	rm -rf tags $(APPS)

