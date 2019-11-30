$(info #############################################################)
$(info ##                    Running sink.mak                     ##)
$(info #############################################################)

# run build scripts for configuration
SRC_DIR=..\..
-include $(SRC_DIR)/config_build.mak

#####################################################################################################
##### Set up prompts based on configuration
#####################################################################################################
build : prompts

PROMPTS_DIR = $(shell $(PYTHON) -c "str='$(IDE_CONFIG)';print str.lower()")

FILESYSTEM = customer_ro_filesystem

PROMPT := $(shell cat ../$(FILESYSTEM)/audio_prompt_config.csr)

prompts :
ifneq ("$(wildcard ../../sink_prompts/$(PROMPTS_DIR)/.)","")
#Prompts are defined for this project
ifneq ("$(wildcard ../$(FILESYSTEM)/audio_prompt_config.csr)","")
	$(info $(PROMPT))
#existing $(FILESYSTEM) directory detected - check if default
ifeq ("$(PROMPT)", "Default")
#Using default prompts so update audio prompt files
	@$(info Using Default prompts in image directory. Updating prompts from ../../sink_prompts/$(PROMPTS_DIR) to $(FILESYSTEM)/*)
	$(clean_audio_prompts)
	$(create_audio_prompt_dirs)
	$(copy_audio_prompts)
else
# do nothing - Customer is using customised audio prompts
	$(info Non default voice prompts detected. Not updating.)
endif
else
# ../$(FILESYSTEM)/prompts does not exist, Copy in all audio prompts files
	@$(info No prompts exist in image directory. Copying default prompts from ../../sink_prompts/$(PROMPTS_DIR) to $(FILESYSTEM)/*)
	@xcopy /Q /Y ..\\..\\sink_prompts\\$(PROMPTS_DIR)\\*.csr ..\\$(FILESYSTEM)\\
	$(create_audio_prompt_dirs)
	$(copy_audio_prompts)
endif
else
	$(info No voice prompts defined for this application)
endif

define clean_audio_prompts
	@$(del) ../$(FILESYSTEM)/prompts  # delete any existing voice prompts
	@$(del) ../$(FILESYSTEM)/headers  # delete any existing voice prompt headers
	@$(del) ../$(FILESYSTEM)/refname  # delete any existing voice prompt refname
endef

define create_audio_prompt_dirs
	@$(info Creating Audio prompts directories )
	@$(mkdir) ../$(FILESYSTEM)/prompts
	@$(mkdir) ../$(FILESYSTEM)/headers
	@$(mkdir) ../$(FILESYSTEM)/refname
endef

define copy_audio_prompts
	@xcopy /Q /Y ..\..\\sink_prompts\\$(PROMPTS_DIR)\\*.prm ..\\$(FILESYSTEM)\\prompts 
	@xcopy /Q /Y ..\..\\sink_prompts\\$(PROMPTS_DIR)\\*.idx ..\\$(FILESYSTEM)\\headers
	@xcopy /Q /Y ..\..\\sink_prompts\\$(PROMPTS_DIR)\\*.txt ..\\$(FILESYSTEM)\\refname 
endef
