

# target, subdir, objects in current dir
TARGET	= controls
SUBDIRS	= controlserv
OBJECTS	= main.o 


all:subdirs ${OBJECTS}
	${CC} -o ${TARGET} $$(find ./${SUBDIRS} -name '*.o') ${OBJECTS} ${LDFLAGS} ${INCLUDES}
	#echo  $$(find ./${SUBDIRS} -name '*.o') 
	mv *.o ${TARGET} obj


clean:cleansubdirs
	rm -f ${TARGET} ${OBJECTS} *.o


# path of "make global scripts"
# NOTE, use absolute path. export once, use in all subdirs
export PROJECTPATH=${PWD}
export MAKEINCLUDE=${PROJECTPATH}/makeconfig/make.global

include ${MAKEINCLUDE}


