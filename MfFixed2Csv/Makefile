# make EXE="MfFixed2Csv" OBJS="MfFixed2Csv.o MfFixed2Csv_Impl.o" CXXFLAGS="-O2 -std=gnu++17 -lstdc++fs"

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $? 

clean:
	$(RM) $(EXE) $(OBJS)

