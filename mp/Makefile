DIST := darkness

resources:
	rsync $(BINDIR)/gameinfo.txt	$(GAMEDST)/gameinfo.txt
	rsync $(BINDIR)/darkness.fgd	$(GAMEDST)/darkness.fgd
	rsync -r $(BINDIR)/cfg/		$(GAMEDST)/cfg

$(DIST):
	./update_dist.sh $(DIST)
	rsync --exclude "*.lib" --exclude "*.pdb" $(GAMEDST)/bin/* $(DIST)/bin/

darkness.zip:
	rm -f $@
	zip -r $@ $(DIST)

.PHONY: clean $(DIST) darkness.zip


clean:
	rm -rf $(DIST) darkness.zip
