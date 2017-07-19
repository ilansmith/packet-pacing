CFLAGS=-std=c++11
LFLAGS=-lstdc++
APP=udp_client

ifeq ($(DEBUG),y)
CFLAGS+=-g
endif

%.o: %.cpp
	g++ $(CFLAGS) -c $<

$(APP): $(APP).o
	gcc -o $@ $^ $(LFLAGS)

clean:
	rm -rf *.o

cleanall: clean
	rm -rf tags $(APP)

