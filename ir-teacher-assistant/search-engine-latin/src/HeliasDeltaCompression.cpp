/*
 * HeliasDeltaCompression.cpp
 *
 *  Created on: Mar 26, 2013
 *      
 */

#include "HeliasDeltaCompression.h"

/**
 * @pio the file to read/write the data
 * @isRead indicates if the class will be used to read data.
 */
HeliasDeltaCompression::HeliasDeltaCompression(iostream& pio, bool pisRead): io(pio), isRead(pisRead),
				buffer(0), bufferPosition(0), bufferSizeBytes(sizeof(buffer)),
				bufferSizeBits(bufferSizeBytes*8), isFirstRead(true), numberOfBytesWritten(0ul)
{

}

HeliasDeltaCompression::~HeliasDeltaCompression() {
	flush();
}


/*
 * Writes the unary representation of n.
 */
void HeliasDeltaCompression::writeUnary(ui n){
	for(ui i = 0; i < n; ++i){
		writeBit(0);
	}
	writeBit(1);
}
/*
 * Writes the binary representation of n.
 */
void HeliasDeltaCompression::writeBynary(ui n, ui nbits){
	for(ui i = nbits; i> 0; --i){
		writeBit(testIBit(n, i-1));
	}
}

/*
 * Writes the gama representation of n.
 */
void HeliasDeltaCompression::writeEliasGamma(ui n){
	ui nbits = log2quick(n);
	writeUnary(nbits);
	writeBynary(n, nbits);
}

/*
 * Writes the delta representation of n.
 */
void HeliasDeltaCompression::writeEliasDelta(ui n){
	ui nbits = log2quick(n);
	writeEliasGamma(nbits+1);
	writeBynary(n, nbits);
}



/**
 * Reads from input stream and converts the number from its data representation.
 */
HeliasDeltaCompression& HeliasDeltaCompression::operator >>(ui& i) {
	//io.read(reinterpret_cast<char*>(&i), sizeof(i));
	i = decodeEliasDelta();
	--i;//is subtract one because in the written phase is added one.
	    //This is done to allows the number 0, because this compression don't handles numbers less than 1.
	return *this;
}

/**
 * Converts the number to its delta representation and writes in outputstream.
 */
HeliasDeltaCompression& HeliasDeltaCompression::operator <<(ui i) {
	++i;//Is added one to allows the number 0.
	//io.write(reinterpret_cast<char*>(&i), sizeof(i));
	writeEliasDelta(i);
	return *this;
}

/**
 * Flushes the buffer.
 */
void HeliasDeltaCompression::putMark(void) {
	if(isRead || bufferPosition == 0){
		return;
	}
	for(ui i = bufferPosition; i < bufferSizeBits; ++i){
		writeBit(1);
	}
}

/**
 * Flushes the buffer.
 */
void HeliasDeltaCompression::flush(void) {
	putMark();
	io.flush();
	isFirstRead = true;
}

/**
 * Gets the next number from the input stream/buffer.
 */
ui HeliasDeltaCompression::decodeEliasDelta(){
	ui L;
	for(L=0; nextBit() != 1u; ++L);//Reads the unary
	ui N = 1u << L;//sets the most significant bit
	for(ui i = 1u; i <= L; ++i){
		if(nextBit() == 1u){
			N |= 1u << (L-i);
		}
	}
	--N;
	ui r = 0;
	for(ui i = 1; i <= N; ++i){
		if(nextBit()){
			r |= 1 << (N-i);
		}
	}
	return (1u << N) + r;
}

