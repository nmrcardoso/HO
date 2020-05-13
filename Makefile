############################################################################################################
VERSION  = V1_$(shell date "+%d_%m_%Y_%T")
VERSION  = V1_$(shell date "+%d_%m_%Y_%H-%M-%S")
STANDARD = c99
############################################################################################################

INC_DIR = $(shell fltk-config  --cxxflags)

FLTK_LIBS =$(shell fltk-config --use-forms --use-images --ldflags --use-gl --use-glut --ldstaticflags)
INC_LIBS = $(FLTK_LIBS) -lpng



GCC ?= g++ -O3 -std=c++11

all : ho




OBJS := ho.o timer.o plot.o ticks.o save_image.o gamma.o PostScript.o



$(OBJDIR)%.o: $(SRCDIR)%.cpp
	$(VERBOSE)$(GCC) $(CCFLAGS) $(INC_DIR) -I. -fopenmp -MMD -MP   -c $< -o $@ 
$(OBJDIR)%.o: $(SRCDIR)%.cxx
	$(VERBOSE)$(GCC) $(CCFLAGS) $(INC_DIR) -I. -fopenmp -MMD -MP   -c $< -o $@ 
	
$(OBJDIR)%.o: $(SRCDIR)%.c
	$(VERBOSE)$(GCC) $(CCFLAGS) $(INC_DIR) -I. -fopenmp -MMD -MP   -c $< -o $@ 

ho:  $(OBJS)
	$(VERBOSE)$(GCC) $(CCFLAGS)  -o $@ $+ $(INC_LIBS)  -fopenmp

clean:
	rm -f $(OBJS) ho *.d


deps = $(OBJS:.o=.d)


pack: 
	@echo Generating Package ho_$(VERSION).tar.gz
	@tar cvfz ho_$(VERSION).tar.gz *.cpp *.c *.cxx *.H *.h $(INCS) Makefile
	@echo Generated Package ho_$(VERSION).tar.gz

.PHONY : clean pack directories $(PROJECTNAME)

-include $(deps)
