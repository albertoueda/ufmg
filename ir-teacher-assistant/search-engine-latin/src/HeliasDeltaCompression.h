/*
 * HeliasDeltaCompression.h
 *
 *  Created on: Mar 26, 2013
 *      
 */

#ifndef HELIASDELTACOMPRESSION_H_
#define HELIASDELTACOMPRESSION_H_

#include <iostream>
#include <cstdlib>
#include "common.h"

using namespace std;

/**
 * This class implements the Helias Delta Compression
 */
class HeliasDeltaCompression {
public:
	/**
	 * @pio the file to read/write the data
	 * @isRead indicates if the class will be used to read data.
	 */
	HeliasDeltaCompression(iostream& pio, bool isRead);
     ~HeliasDeltaCompression();

     /**
      * Reads from input stream and converts the number from its data representation.
      */
     HeliasDeltaCompression& operator>>(ui& i);
     /**
      * Converts the number to its delta representation and writes in outputstream.
      */
     HeliasDeltaCompression& operator<<(ui i);

     /**
      * Flushes the buffer.
      */
     void putMark(void);
     /**
      * Flushes the buffer.
      */
     void flush(void);
     /**
      * Return the number of bytes written until this moment.
      */
     ul getNumberOfBytesWritten(void) const{
    	 return numberOfBytesWritten;
     }

     /**
      * Is used when the seek operation is performed.
      */
     void reset(void){
    	 isFirstRead = true;
    	 bufferPosition = 0;
     }

private:
     iostream& io;//Stream to read/write data
     bool isRead;//Indicates if this object will be use to read
     ul buffer;//buffer to store the bits written
     ui bufferPosition;//position of the bit buffer
     const ui bufferSizeBytes;//size in bytes of the buffer
     const ui bufferSizeBits;//size in bits of the buffer
     bool isFirstRead;//Indicates if is the first reading operation
     ul numberOfBytesWritten;//number of bytes written until this moment.


    /**
     * Writes one bit in the buffer. If the buffer becomes full it is written
     * in the output stream.
     */
    void writeBit(ui bit){
    	if(bit){
    		buffer |= (1ul << bufferPosition);
    	}
    	++bufferPosition;
    	if(bufferPosition == bufferSizeBits){
    		numberOfBytesWritten += bufferSizeBytes;
    		io.write(reinterpret_cast<char*>(&buffer), bufferSizeBytes);
    		buffer = 0ul;
    		bufferPosition = 0ul;
    	}
    }

    /**
     * Reads the next bit from the buffer.
     * If the buffer is empty it's filled with the input stream data.
     */
    ui nextBit(void){
    	if(isFirstRead || bufferPosition == bufferSizeBits){
    		isFirstRead = false;
    		io.read(reinterpret_cast<char*>(&buffer), bufferSizeBytes);
    		if(!io){
    			cerr << "Reading end of file!!!" << endl;
    			exit(-1);
    		}
    		bufferPosition = 0;
    	}

    	return testIBitUl(buffer, bufferPosition++);
    }

    /**
     * Gets the most significant bit from v
     */
    ui log2quick(ui v){
     	register ui r; // result of log2(v) will go here
     	register ui shift;

     	r =     (v > 0xFFFF) << 4; v >>= r;
     	shift = (v > 0xFF  ) << 3; v >>= shift; r |= shift;
     	shift = (v > 0xF   ) << 2; v >>= shift; r |= shift;
     	shift = (v > 0x3   ) << 1; v >>= shift; r |= shift;
     	                                        r |= (v >> 1);
     	return r;
     }

    /**
     * Test if the bit i is set in the binary number n.
     */
    bool testIBitUl(ul n, ui i){
    	return (n & (1ul << i));
    }

    /**
     * Test if the bit i is set in the binary number n.
     */
     bool testIBit(ui n, ui i){
     	return (n & (1u << i));
     }

     /*
      * Writes the unary representation of n.
      */
     void writeUnary(ui n);
     /*
      * Writes the binary representation of n.
      */
     void writeBynary(ui n, ui nbits);
     /*
      * Writes the gamma representation of n.
      */
     void writeEliasGamma(ui n);
     /*
      * Writes the delta representation of n.
      */
     void writeEliasDelta(ui n);

     /**
      * Gets the next number from the input stream/buffer.
      */
     ui decodeEliasDelta();
};

#endif /* HELIASDELTACOMPRESSION_H_ */
