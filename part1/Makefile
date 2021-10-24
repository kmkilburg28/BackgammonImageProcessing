CC = g++
OPENCV_FLAGS := $(shell pkg-config --cflags --libs /usr/local/Cellar/opencv/4.5.1_3/lib/pkgconfig/opencv4.pc)
CFLAGS = -Wall -g ${OPENCV_FLAGS} -std=c++11
LDFLAGS =
OBJFILES = p1_code.o
TARGET = p1
# g++ ${OPENCV_FLAGS} -std=c++11  main.cpp -o simple-demo
# g++ $(pkg-config --cflags --libs /usr/local/Cellar/opencv/4.5.1_3/lib/pkgconfig/opencv4.pc) -std=c++11  main.cpp -o simple-demo

all : ${TARGET}

${TARGET} :
	${CC} ${CFLAGS}  p1_code.cpp -o ${TARGET} ${LDFLAGS}
# ${TARGET} : ${OBJFILES}
# 	${CC} ${CFLAGS} ${OBJFILES} -o ${TARGET} ${LDFLAGS}

# main.o : label_components.o find_comp_origin.o
# 	${CC} ${CFLAGS} main.cpp ${LDFLAGS}
# label_components.o :
# 	${CC} ${CFLAGS} label_components.cpp ${LDFLAGS}
# find_comp_origin.o :
# 	${CC} ${CFLAGS} find_comp_origin.cpp ${LDFLAGS}

clean :
	rm -f ${TARGET} ${OBJFILES} *~
