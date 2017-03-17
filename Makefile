all : easy-test
	@./easy-test

easy-test : framework_test.o
	$(CXX) $(OUTPUT_OPTION) $^

framework_test.o: framework_test.cpp ttest.h

.PHONY : all clean

clean :
	@rm easy-test
	@rm *.o


.SILENT:

