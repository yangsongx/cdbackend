# Makefile for the java
#
#    how to execute:
#    `java -classpath protobuf-java-2.5.0.jar:. com.caredear.TokenMain`


.PHONY: all
all:
	javac -cp protobuf-java-2.5.0.jar TokenMain.java TokenMessage.java
	mv *.class com/caredear/

.cc.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY:clean
clean:
	find . -name '*.class' -exec rm -rf {} ";"

