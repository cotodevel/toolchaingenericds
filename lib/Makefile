all: buildWoopsiEnv

buildWoopsiEnv: 
	$(MAKE)	-R	-C	Woopsi/libwoopsi/
	$(MAKE)	-R	-C	Woopsi/woopsiExamples/demoWoopsi
	
clean:
	$(MAKE)	clean	-C	Woopsi/libwoopsi/
	$(MAKE)	clean	-C	Woopsi/woopsiExamples/demoWoopsi