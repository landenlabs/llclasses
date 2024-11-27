CXX = clang++
CXXFLAGS = -std=c++14

# define the C source files
SRCS = llclasses.cpp ClassRel.cpp Parser.cpp Presenter.cpp \
 parsers/ParseCpp.cpp \
 parsers/ParseJava.cpp \
 utils/Log.cpp \
 utils/DirectoryFiles.cpp \
 publishers/PublishViz.cpp \
 publishers/PublishText.cpp \
 publishers/PublishHtml.cpp \
 

OBJS = $(SRCS:.c=.o)

# define the executable file 
MAIN = llreplace

all: $(MAIN)
      
      
$(MAIN): $(OBJS) 
	$(CXX) $(CXXFLAGS) -I . -I parsers -I publishers -I utils -o $(MAIN) $(OBJS) # $(LFLAGS) $(LIBS)
  

clean:
	rm -rf *.o* $(MAIN)


#depend: $(SRCS)
#    makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
