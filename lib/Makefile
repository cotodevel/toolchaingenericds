all: buildWoopsiEnv

buildWoopsiEnv: 
	$(MAKE)	-R	-C	Woopsi/libwoopsi/
	$(MAKE)	-R	-C	Woopsi/woopsiTemplates/TGDS-Woopsi-template
	
clean:
	$(MAKE)	clean	-C	Woopsi/libwoopsi/
	$(MAKE)	clean	-C	Woopsi/woopsiTemplates/TGDS-Woopsi-template