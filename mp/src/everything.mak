# VPC MASTER MAKEFILE



SHELL:=/bin/bash
# to control parallelism, set the MAKE_JOBS environment variable
ifeq ($(strip $(MAKE_JOBS)),)
	ifeq ($(shell uname),Darwin)
		CPUS:=$(shell /usr/sbin/sysctl -n hw.ncpu)
	endif
	ifeq ($(shell uname),Linux)
		CPUS:=$(shell grep processor /proc/cpuinfo | wc -l)
	endif
	MAKE_JOBS:=$(CPUS)
endif

ifeq ($(strip $(MAKE_JOBS)),)
	MAKE_JOBS:=8
endif

# All projects (default target)
all: 
	@$(MAKE) -f $(lastword $(MAKEFILE_LIST)) -j$(MAKE_JOBS) all-targets

all-targets : client_hl2mp shaders_hl2mp mathlib raytrace server_hl2mp serverplugin_empty tier1 vgui_controls 


# Individual projects + dependencies

client_hl2mp : mathlib tier1 vgui_controls 
	@echo "Building: client_hl2mp"
	@+cd /home/wep/devel/darkness/mp/src/game/client && $(MAKE) -f client_linux32_hl2mp.mak $(CLEANPARAM)

shaders_hl2mp : mathlib tier1 
	@echo "Building: shaders_hl2mp"
	@+cd /home/wep/devel/darkness/mp/src/materialsystem/stdshaders && $(MAKE) -f game_shader_dx9_linux32_hl2mp.mak $(CLEANPARAM)

mathlib : 
	@echo "Building: mathlib"
	@+cd /home/wep/devel/darkness/mp/src/mathlib && $(MAKE) -f mathlib_linux32.mak $(CLEANPARAM)

raytrace : 
	@echo "Building: raytrace"
	@+cd /home/wep/devel/darkness/mp/src/raytrace && $(MAKE) -f raytrace_linux32.mak $(CLEANPARAM)

server_hl2mp : mathlib tier1 
	@echo "Building: server_hl2mp"
	@+cd /home/wep/devel/darkness/mp/src/game/server && $(MAKE) -f server_linux32_hl2mp.mak $(CLEANPARAM)

serverplugin_empty : mathlib tier1 
	@echo "Building: serverplugin_empty"
	@+cd /home/wep/devel/darkness/mp/src/utils/serverplugin_sample && $(MAKE) -f serverplugin_empty_linux32.mak $(CLEANPARAM)

tier1 : 
	@echo "Building: tier1"
	@+cd /home/wep/devel/darkness/mp/src/tier1 && $(MAKE) -f tier1_linux32.mak $(CLEANPARAM)

vgui_controls : 
	@echo "Building: vgui_controls"
	@+cd /home/wep/devel/darkness/mp/src/vgui2/vgui_controls && $(MAKE) -f vgui_controls_linux32.mak $(CLEANPARAM)

# this is a bit over-inclusive, but the alternative (actually adding each referenced c/cpp/h file to
# the tags file) seems like more work than it's worth.  feel free to fix that up if it bugs you. 
TAGS:
	@rm -f TAGS
	@find /home/wep/devel/darkness/mp/src/game/client -name '*.cpp' -print0 | xargs -0 etags --declarations --ignore-indentation --append
	@find /home/wep/devel/darkness/mp/src/game/client -name '*.h' -print0 | xargs -0 etags --language=c++ --declarations --ignore-indentation --append
	@find /home/wep/devel/darkness/mp/src/game/client -name '*.c' -print0 | xargs -0 etags --declarations --ignore-indentation --append
	@find /home/wep/devel/darkness/mp/src/materialsystem/stdshaders -name '*.cpp' -print0 | xargs -0 etags --declarations --ignore-indentation --append
	@find /home/wep/devel/darkness/mp/src/materialsystem/stdshaders -name '*.h' -print0 | xargs -0 etags --language=c++ --declarations --ignore-indentation --append
	@find /home/wep/devel/darkness/mp/src/materialsystem/stdshaders -name '*.c' -print0 | xargs -0 etags --declarations --ignore-indentation --append
	@find /home/wep/devel/darkness/mp/src/mathlib -name '*.cpp' -print0 | xargs -0 etags --declarations --ignore-indentation --append
	@find /home/wep/devel/darkness/mp/src/mathlib -name '*.h' -print0 | xargs -0 etags --language=c++ --declarations --ignore-indentation --append
	@find /home/wep/devel/darkness/mp/src/mathlib -name '*.c' -print0 | xargs -0 etags --declarations --ignore-indentation --append
	@find /home/wep/devel/darkness/mp/src/raytrace -name '*.cpp' -print0 | xargs -0 etags --declarations --ignore-indentation --append
	@find /home/wep/devel/darkness/mp/src/raytrace -name '*.h' -print0 | xargs -0 etags --language=c++ --declarations --ignore-indentation --append
	@find /home/wep/devel/darkness/mp/src/raytrace -name '*.c' -print0 | xargs -0 etags --declarations --ignore-indentation --append
	@find /home/wep/devel/darkness/mp/src/game/server -name '*.cpp' -print0 | xargs -0 etags --declarations --ignore-indentation --append
	@find /home/wep/devel/darkness/mp/src/game/server -name '*.h' -print0 | xargs -0 etags --language=c++ --declarations --ignore-indentation --append
	@find /home/wep/devel/darkness/mp/src/game/server -name '*.c' -print0 | xargs -0 etags --declarations --ignore-indentation --append
	@find /home/wep/devel/darkness/mp/src/utils/serverplugin_sample -name '*.cpp' -print0 | xargs -0 etags --declarations --ignore-indentation --append
	@find /home/wep/devel/darkness/mp/src/utils/serverplugin_sample -name '*.h' -print0 | xargs -0 etags --language=c++ --declarations --ignore-indentation --append
	@find /home/wep/devel/darkness/mp/src/utils/serverplugin_sample -name '*.c' -print0 | xargs -0 etags --declarations --ignore-indentation --append
	@find /home/wep/devel/darkness/mp/src/tier1 -name '*.cpp' -print0 | xargs -0 etags --declarations --ignore-indentation --append
	@find /home/wep/devel/darkness/mp/src/tier1 -name '*.h' -print0 | xargs -0 etags --language=c++ --declarations --ignore-indentation --append
	@find /home/wep/devel/darkness/mp/src/tier1 -name '*.c' -print0 | xargs -0 etags --declarations --ignore-indentation --append
	@find /home/wep/devel/darkness/mp/src/vgui2/vgui_controls -name '*.cpp' -print0 | xargs -0 etags --declarations --ignore-indentation --append
	@find /home/wep/devel/darkness/mp/src/vgui2/vgui_controls -name '*.h' -print0 | xargs -0 etags --language=c++ --declarations --ignore-indentation --append
	@find /home/wep/devel/darkness/mp/src/vgui2/vgui_controls -name '*.c' -print0 | xargs -0 etags --declarations --ignore-indentation --append



# Mark all the projects as phony or else make will see the directories by the same name and think certain targets 

.PHONY: TAGS showtargets regen showregen clean cleantargets cleanandremove relink client_hl2mp shaders_hl2mp mathlib raytrace server_hl2mp serverplugin_empty tier1 vgui_controls 



# The standard clean command to clean it all out.

clean: 
	@$(MAKE) -f $(lastword $(MAKEFILE_LIST)) -j$(MAKE_JOBS) all-targets CLEANPARAM=clean



# clean targets, so we re-link next time.

cleantargets: 
	@$(MAKE) -f $(lastword $(MAKEFILE_LIST)) -j$(MAKE_JOBS) all-targets CLEANPARAM=cleantargets



# p4 edit and remove targets, so we get an entirely clean build.

cleanandremove: 
	@$(MAKE) -f $(lastword $(MAKEFILE_LIST)) -j$(MAKE_JOBS) all-targets CLEANPARAM=cleanandremove



#relink

relink: cleantargets 
	@$(MAKE) -f $(lastword $(MAKEFILE_LIST)) -j$(MAKE_JOBS) all-targets



# Here's a command to list out all the targets


showtargets: 
	@echo '-------------------' && \
	echo '----- TARGETS -----' && \
	echo '-------------------' && \
	echo 'clean' && \
	echo 'regen' && \
	echo 'showregen' && \
	echo 'client_hl2mp' && \
	echo 'shaders_hl2mp' && \
	echo 'mathlib' && \
	echo 'raytrace' && \
	echo 'server_hl2mp' && \
	echo 'serverplugin_empty' && \
	echo 'tier1' && \
	echo 'vgui_controls'



# Here's a command to regenerate this makefile


regen: 
	devtools/bin/vpc_linux /hl2mp +everything /mksln everything 


# Here's a command to list out all the targets


showregen: 
	@echo devtools/bin/vpc_linux /hl2mp +everything /mksln everything 

