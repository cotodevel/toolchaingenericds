all: buildWoopsiEnv

buildWoopsiEnv: 
	$(MAKE)	-R	-C	Woopsi/libwoopsi/
	$(MAKE)	-R	-C	Woopsi/woopsiExamples/demoWoopsi
	
clean:
	$(MAKE)	-R	-C	Woopsi/libwoopsi/ clean
	$(MAKE)	-R	-C	Woopsi/woopsiExamples/demoWoopsi clean
	