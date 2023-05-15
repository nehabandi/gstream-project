#compiler
CC=g++
C_FLAGS=-Wall

#gstreamer library
GST_LIB= $(shell pkg-config --cflags --libs gstreamer-1.0)

#source_file
SOURCE=SME_project.cpp

#executable file
FILE=OUTPUT

#BUILD
all :$(FILE)

$(FILE): $(SOURCE)
	$(CC) $(C_FLAGS) $(SOURCE) -o $(FILE) $(GST_LIB) 

clean : 
	rm -rf $(FILE)



