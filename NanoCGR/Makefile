
OutDir=../Debug

src=$(wildcard *.cpp)
dir=$(notdir $(src))
obj=$(patsubst %.cpp,%.o,$(dir))
libd= ${OutDir}/libnanoc.so
libs= ${OutDir}/libnanoc.a
liba= -lnanoc
objd=nanocgr_d
objs=nanocgr_s

d: ${objd}

s: ${objs}

%.o: %.cpp
	g++ -g -rdynamic  -m64 -fPIC  $< -c -o $@
	
${objd}: ${obj}
	g++ -g -rdynamic  -m64 -o ${OutDir}/${objd} ${obj} -L${OutDir}/ ${liba} -Wl,--rpath=${OutDir}/ -ldl -lpthread
	
${objs}: ${obj}
	g++ -g -rdynamic  -m64 -o ${OutDir}/${objs}  ${obj} ${libs}  -Wl,--rpath=${OutDir}/ -ldl -lpthread
	
clean:
	rm ${obj}

clean_d: clean
	rm ${OutDir}/${objd}
	
clean_s: clean
	rm ${OutDir}/${objs}
	
	
run_d:
	../Debug/nanocgr_d
	
run_s:
	../Debug/nanocgr_s
