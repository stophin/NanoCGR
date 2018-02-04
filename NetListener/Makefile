
OutDir=../Debug

src=$(wildcard *.cpp)
dir=$(notdir $(src))
obj=$(patsubst %.cpp,%.o,$(dir))
libd=libnetlistener.so
libs=libnetlistener.a

d: ${libd}

s: ${libs}

%.o: %.cpp
	g++ -g -rdynamic  -m64 -fPIC  $< -c -o $@
	
${libd}: ${obj}
	g++ -g -rdynamic  -m64 -fPIC -shared -o ${OutDir}/${libd} ${obj}
	
${libs}: ${obj}
	ar rcs ${OutDir}/${libs} ${obj}
	
clean:
	rm ${obj}

clean_d: clean
	rm  ${OutDir}/${libd}

clean_s: clean
	rm  ${OutDir}/${libs}
	