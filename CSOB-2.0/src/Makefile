# Makefile of oddmap-demo

CC = g++
O = ./bin
# set O3 to avoid all optimization
OPT_LEVEL = -O3
COMPILE = $(CC) $(OPT_LEVEL)

SOURCE = calculate.cpp getbasic.cpp main.cpp

OBJS = $(patsubst %.cpp,$O/%.o,$(SOURCE))

INCS = $(wildcard include/*.h)

MONGDBPATH = -I/usr/local/include/bsoncxx/v_noabi \
	-I/usr/local/include/mongocxx/v_noabi \
	-I/usr/local/include/libmongoc-1.0/ \
	-I/usr/local/include/libbson-1.0 \
	-I/opt/boost_1_81_0 \
	-L/usr/local/lib
FLAG = -lhiredis -levent -lmongocxx -lbsoncxx

all : csob store datatocalculate 

store : $O/redis_subscriber.o $O/mongodbstorage.o $O/calculate.o $O/redis_publisher.o
	$(COMPILE) -o $@ $^ $(FLAG)

csob : $(OBJS) $O/redis_publisher.o $O/redis_subscriber.o
	$(COMPILE) -o $@ $^ $(FLAG)  /opt/boost_1_81_0/stage/lib/libboost_serialization.a 

test : test.cpp getbasic.o
#	g++ -o test test.cpp -I /opt/boost_1_81_0 -L /opt/boost_1_81_0/stage/lib/ -lboost_serialization 
	$(COMPILE) -o test $^ /opt/boost_1_81_0/stage/lib/libboost_serialization.a 

datatocalculate: $O/redis_subscriber.o $O/datatocalculate.o $O/redis_publisher.o $O/calculate.o
	$(COMPILE) -o $@ $^ $(FLAG)  /opt/boost_1_81_0/stage/lib/libboost_serialization.a 

# test_redis : test_redis.cpp $O/redis_publisher.o  $/getbasic.o
# 	$(COMPILE) -o $@ $^ $(FLAG)


$O/redis_subscriber.o : ./sub-pub/redis_subscriber.h
	$(COMPILE) -c ./sub-pub/redis_subscriber.cpp -o $@ 

$O/redis_publisher.o : ./sub-pub/redis_publisher.h
	$(COMPILE) -c ./sub-pub/redis_publisher.cpp -o $@ 

$O/main.o : main.cpp $(INCS)
	$(COMPILE) -c $< -o $@ $(MONGDBPATH)

$O/calculate.o : calculate.cpp $(INCS)
	$(COMPILE) -c $< -o $@ $(MONGDBPATH)

$O/getbasic.o : getbasic.cpp $(INCS) $O/redis_publisher.o
	$(COMPILE) -c $< -o $@ $(MONGDBPATH)

$O/mongodbstorage.o : mongodbstorage.cpp $(INCS) 
	$(COMPILE) -c $< -o $@ $(MONGDBPATH)

$O/datatocalculate.o : datatocalculate.cpp $(INCS) 
	$(COMPILE) -c $< -o $@ $(MONGDBPATH)

# $O/test_redis.o : test_redis.cpp $(INCS)
# 	$(COMPILE) -c $< -o $@ $(MONGDBPATH)


#if you add any *.cpp to the project, you should add a line like lines upon and add the *.h to main.cpp
.PHONY : clean
clean : 
	rm -f $O/*.o ./store ./csob ./test *.o
