INC_PATH = /home/yunfei/myworld/network_ramework/dep/boost/include
LIB_PATH = /home/yunfei/myworld/network_ramework/dep/boost/lib

SRC = $(shell ls *.cpp)
OBJS = $(subst .cpp,.o,$(SRC))

test:$(OBJS)
	g++ -o test $(OBJS) -I $(INC_PATH) -L $(LIB_PATH) -lboost_system -lboost_thread

$(OBJS):%.o:%.cpp
	g++ -c $< -I $(INC_PATH) -L $(LIB_PATH)

#.PHONY:clean
clean:
	rm test *.o
