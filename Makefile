# include user defined parameters
include Defs.mk

bin_dir				:=	bin
target_bin			:=	$(bin_dir)/$(target)
build_dir			:=	builds/$(target)/

src_files			:=	$(shell find $(sub_dirs) -name '*.cpp')

obj_files			:=	$(notdir $(src_files:%.cpp=%.o))
full_obj_files		:=	$(addprefix $(build_dir), $(obj_files))

dep_files			:=	$(subst .o,.d,$(full_obj_files))

INC					+=	$(addprefix -I./, $(addsuffix /include, $(sub_dirs)))

create-build-dir	:=	$(shell test -d $(build_dir) || mkdir -p $(build_dir))
create-binary-dir	:=	$(shell test -d $(bin_dir) || mkdir -p $(bin_dir))

vpath %.cpp $(sub_dirs)

.PHONY: $(platform)
$(platform): $(target_bin)

$(target_bin): $(full_obj_files)
	@echo	'	LD $@'
	@$(CPP) $(CPPFLAG) $(INC) $^ -o $@ $(LIB)
	@echo	'	STRIP $@'
	@$(STRIP) $(STRIP_FLAG) $@

$(build_dir)%.o: %.cpp
	@echo '	CPP $<'
	@$(CPP) $(CPPFLAG) $(INC) -M -MF $(@:.o=.d) -MP -MT $@ $<
	@$(CPP) $(CPPFLAG) $(INC) $< -c -o $@

.PHONY: clean
clean:
	rm -rf *.log
	rm -rf bin
	rm -rf builds

ifneq "$(MAKECMDGOALS)" "clean"
-include $(dep_files)
endif
