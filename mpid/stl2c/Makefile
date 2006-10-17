
STL2CSOURCE := stl2c_basic.c stl2c_vector.c stl2c_list.c stl2c_map.c
STL2COBJECT := stl2c_basic.o stl2c_vector.o stl2c_list.o stl2c_map.o
STL2CHEADER := stl2c_basic.h  stl2c_bool.h  stl2c_deque.h stl2c_list.h stl2c_map.h stl2c_vector.h

default: stl2c

stl2c:  $(STL2CSOURCE:.c=.o)

$(STL2COBJECT): $(STL2CHEADER)

clean:
	rm -rf $(STL2COBJECT)
