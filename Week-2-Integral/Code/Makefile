CXX = g++
CXXFLAGS = -O3 -std=c++17
TARGETS = AES_distinguisher SmallScaleAES Convolution AES_4round_key_recovery

all: $(TARGETS)

AES_distinguisher: AES_distinguisher.cpp aes.cpp aes.h
	$(CXX) $(CXXFLAGS) -o AES_distinguisher AES_distinguisher.cpp aes.cpp

SmallScaleAES: SmallScaleAES.cpp SmallScaleAES.h
	$(CXX) $(CXXFLAGS) -o SmallScaleAES SmallScaleAES.cpp

Convolution: convolution.cpp
	$(CXX) $(CXXFLAGS) -o Convolution convolution.cpp

AES_4round_key_recovery: AES_4round_key_recovery.cpp aes.cpp aes.h
	$(CXX) $(CXXFLAGS) -o AES_4round_key_recovery AES_4round_key_recovery.cpp aes.cpp

clean:
	rm -f $(TARGETS)

.PHONY: all clean
