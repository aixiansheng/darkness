#
# These are exported for modelsrc/*/Makefile, which
# make use of cmd.exe batch files that require windows
# paths
#
export GAMEDIR_CYG := $(PWD)/game/mod_hl2mp
export GAMEDIR := $(shell cygpath -m ${GAMEDIR_CYG})

GAME_SRC := game/game_src
GAME_DST := game/mod_hl2mp

MDL_SRC := $(GAME_SRC)/modelsrc
MAT_SRC := $(GAME_SRC)/materialsrc
MAT_DST := $(GAME_DST)/materials

VPK_PUBKEY := $(PWD)/darkness.publickey.vdf
VPK_PRIVKEY := $(PWD)/darkness.privatekey.vdf
VPK_MANIFEST := $(PWD)/vpk_manifest.txt
VPK_PUBKEY_WIN := $(shell cygpath -w ${VPK_PUBKEY})
VPK_PRIVKEY_WIN := $(shell cygpath -w ${VPK_PRIVKEY})
VPK_MANIFEST_WIN := $(shell cygpath -w ${VPK_MANIFEST})
VPK_DEST := $(PWD)/darkness.vpk
VPK_DEST_WIN := $(shell cygpath -w ${VPK_DEST})
DIST_DIR := darkness

VPK_DIR := darkness_dir.vpk
VPK_ARCH := darkness_000.vpk

all: $(MDL_SRC) $(MAT_SRC)
	rsync -ar -L --exclude 'materialsrc' --exclude 'modelsrc' --exclude 'mapsrc' $(GAME_SRC)/ $(GAME_DST)/

textures: $(MAT_SRC)

setup:
	./setup_bin.sh

resources:
	rsync -ar $(GAME_SRC)/resource/ $(GAME_DST)/resource/
	rsync -a $(GAME_SRC)/scripts/ $(GAME_DST)/scripts/
	rsync -a $(GAME_SRC)/gameinfo.txt $(GAME_DST)/gameinfo.txt
	rsync -a --delete --delete-excluded --exclude 'config.cfg' $(GAME_SRC)/cfg/ $(GAME_DST)/cfg/
	rsync -a $(GAME_SRC)/particles/ $(GAME_DST)/particles/


#$(VPK_DIR): $(VPK_MANIFEST)
#	@rm -f $@
#	cygstart -d $(GAME_DST) cmd.exe /K "vpk.exe -P -k $(VPK_PUBKEY_WIN) -K $(VPK_PRIVKEY_WIN) a $(VPK_DEST_WIN) @$(VPK_MANIFEST_WIN)"
#
#dist: $(VPK_DIR)
#	@mkdir -p $(DIST_DIR)
#	@rsync -ar --delete --delete-excluded --include-from=for_dist.txt --exclude '*' $(GAME_DST)/ $(DIST_DIR)/
#	@cp $(VPK_DIR) $(VPK_ARCH) $(DIST_DIR)/
#	@echo "Game dist dir: $(DIST_DIR)"

dist:
	-mkdir -p $(DIST_DIR)
	-rsync -ar --delete --delete-excluded --exclude-from=filter.txt $(GAME_DST)/ $(DIST_DIR)/
	-echo "Game dist: $(DIST_DIR)"

darkness.zip:
	zip -r $@ $(DIST_DIR)

.PHONY: clean $(MDL_SRC) $(MAT_SRC) resources textures dist vpk setup matonly

$(MDL_SRC): resources
	$(MAKE) -C $(MDL_SRC)

$(MAT_SRC):
	rsync -a --delete --delete-excluded --exclude '*.png' --exclude '*.tga' --exclude '*.xcf' $(MAT_SRC)/ $(MAT_DST)/
	rsync -a $(MAT_SRC)/game.ico $(MAT_SRC)/game.tga $(MAT_SRC)/game.png $(MAT_DST)/

matonly:
	rsync -a --delete --delete-excluded --exclude '*.png' --exclude '*.tga' --exclude '*.xcf' $(MAT_SRC)/ $(MAT_DST)/
	rsync -a $(MAT_SRC)/game.ico $(MAT_SRC)/game.tga $(MAT_SRC)/game.png $(MAT_DST)/

clean:
	rm *.vpk
	rm -rf darkness
