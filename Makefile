CFLAGS=-std=c++0x -fPIC
LFLAGS=-lstdc++ -fPIC -L.
APPS=udp_client

ifeq ($(DEBUG),y)
CFLAGS+=-g
endif

%.o: %.cpp
	g++ $(CFLAGS) -c $<

lib%.a: %.o
	ar -r $@ $^

all:$(APPS)

udp_client: udp_client.o libpacing.a 
	g++ -o $@ $(LFLAGS) -lpacing $^

clean:
	rm -rf *.o

cleanall: clean
	rm -rf tags *.a $(APPS)

