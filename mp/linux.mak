#
# The linux dist is simple:
# take the zip file from the windows build
# - remove everything from bin/
# - put the linux bin/ files there
# - tar it up
#

GAMEDIR := $(PWD)/game/mod_hl2mp
BIN_SRC := $(GAMEDIR)/bin
WIN_ZIP := $(PWD)/darkness.zip

DIST_DIR := darkness
DIST_TGZ := darkness.tgz

setup:
	./setup_bin.sh

$(DIST_TGZ): $(WIN_ZIP)
	unzip $(WIN_ZIP)
	rm $(DIST_DIR)/bin/*
	cp $(BIN_SRC)/* $(DIST_DIR)/bin/
	tar -czf $(DIST_TGZ) $(DIST_DIR)

.PHONY: clean resources textures dist setup

clean:
	rm -rf darkness darkness.tgz
