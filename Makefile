default: d

clean: clean_d

run: run_d

d: NetListener_d NanoC_d NanoCImp_d NanoCGR_d

s: NetListener_s NanoC_s NanoCImp_s NanoCGR_s

##################################
NetListener_d:
	make -C NetListener d
	
NanoC_d:
	make -C NanoC d
	
NanoCImp_d:
	make -C NanoCImp d
	
NanoCGR_d:
	make -C NanoCGR d
	
run_d:
	make -C NanoCGR run_d
	
##################################
NetListener_s:
	make -C NetListener s
	
NanoC_s:
	make -C NanoC s
	
NanoCImp_s:
	make -C NanoCImp s
	
NanoCGR_s:
	make -C NanoCGR s
	
run_s:
	make -C NanoCGR run_s
	
##################################
clean_d:
	make -C NetListener clean_d
	make -C NanoC clean_d
	make -C NanoCImp clean_d
	make -C NanoCGR clean_d
	
clean_s:
	make -C NetListener clean_s
	make -C NanoC clean_s
	make -C NanoCImp clean_s
	make -C NanoCGR clean_s
##################################
