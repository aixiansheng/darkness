DIST := darkness

gamedir:
	rsync $(BINDIR)/gameinfo.txt	$(GAMEDST)/gameinfo.txt
	rsync $(BINDIR)/darkness.fgd	$(GAMEDST)/darkness.fgd
	rsync $(BINDIR)/lights.rad	$(GAMEDST)/lights.rad
	rsync -r $(BINDIR)/cfg/		$(GAMEDST)/cfg
	rsync -r $(BINDIR)/scripts/	$(GAMEDST)/scripts
	rsync -r $(BINDIR)/resource/	$(GAMEDST)/resource

gamedir_bigfiles:
	rsync -r $(BINDIR)/materials/	$(GAMEDST)/materials
	rsync -r $(BINDIR)/models/	$(GAMEDST)/models
	rsync -r $(BINDIR)/maps/	$(GAMEDST)/maps

updatebin:
	rsync $(GAMEDST)/bin/*.dll	$(BINDIR)/bin/
	rsync $(GAMEDST)/maps/*.bsp	$(BINDIR)/maps/

$(DIST):
	./update_dist.sh $(DIST)
	rsync --exclude "*.lib" --exclude "*.pdb" $(GAMEDST)/bin/* $(DIST)/bin/

darkness.zip:
	rm -f $@
	zip -r $@ $(DIST)

.PHONY: clean $(DIST) darkness.zip


clean:
	rm -rf $(DIST) darkness.zip
