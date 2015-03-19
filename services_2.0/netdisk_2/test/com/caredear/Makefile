##
# how to run:
# `java -classpath /home/yangsongxiang/server/backend/3rd_party/protobuf-2.5.0/java/protobuf-java-2.5.0.jar:. com.caredear.test`
#
.PHONY: all
all:
	mkdir -p com/caredear/
	javac -cp /home/yangsongxiang/server/backend/3rd_party/protobuf-2.5.0/java/protobuf-java-2.5.0.jar test.java NetdiskMessage.java
	mv *.class com/caredear/

.PHONY:clean
	clean:
	find . -name '*.class' -exec rm -rf {} ";"
