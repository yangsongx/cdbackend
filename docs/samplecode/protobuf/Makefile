# Makefile of top level
#

# How to run java applet via command line:
#   * put .class to new dir com/caredear/
#   * execute `java -classpath protobuf-java-2.5.0.jar:. com.caredear.TokenMain`

.PHONY: all
all: cpp java

cpp:
	cd ./cpp_binding/ && $(MAKE)

java:
	cd ./java_binding/com/caredear/ && $(MAKE)


