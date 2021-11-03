all: tgdsBuild

tgdsBuild: 
	$(MAKE)	-R	-C	ntr/
	$(MAKE)	-R	-C	twl/

clean:
	$(MAKE) -C ntr/ clean
	$(MAKE) -C twl/ clean

#---------------------------------------------------------------------------------

commitChanges:
	-@git commit -a	-m '$(COMMITMSG)'
	-@git push origin HEAD
	
#---------------------------------------------------------------------------------

switchStable:
	-@git checkout -f	'TGDS1.64'
	
#---------------------------------------------------------------------------------

switchMaster:
	-@git checkout -f	'master'
