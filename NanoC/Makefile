
OutDir=../Debug

src=$(wildcard *.cpp)
dir=$(notdir $(src))
obj=$(patsubst %.cpp,%.o,$(dir))
ext=../NetListener/NetListener.o ../NetListener/NetSession.o
liba= -lnetlistener
libd=libnanoc.so
libs=libnanoc.a

d: ${libd}

s: ${libs}

%.o: %.cpp
	g++ -g -rdynamic -m64 -fPIC  $< -c -o $@
	
${libd}: ${obj}
	g++ -m64 -fPIC -shared -o ${OutDir}/${libd} ${obj} -L${OutDir}/ ${liba}
	
${libs}: ${obj}
	ar rcs ${OutDir}/${libs} ${obj} ${ext}
	
clean:
	rm ${obj}

clean_d: clean
	rm  ${OutDir}/${libd}

clean_s: clean
	rm  ${OutDir}/${libs}
	
