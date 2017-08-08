CFLAGS=-std=c++0x -fPIC
LFLAGS=-lstdc++ -fPIC -L.
APPS=udp_client
OBJS_UDP_CLIENT=udp_client.o

ifeq ($(DEBUG),y)
CFLAGS+=-g
endif

ifeq ($(PACING_LIB),y)
	CFLAGS+=-DUSE_PACING_LIB
	OBJS_UDP_CLIENT+=libpacing.a
	LFLAGS+=-lpacing
endif

%.o: %.cpp
	g++ $(CFLAGS) -c $^

lib%.a: %.o
	ar -r $@ $^

all:$(APPS)

udp_client: $(OBJS_UDP_CLIENT)
	g++ -o $@ $(LFLAGS) $^

clean:
	rm -rf *.o

cleanall: clean
	rm -rf tags *.a $(APPS)

