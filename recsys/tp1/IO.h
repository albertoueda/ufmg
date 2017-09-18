/*
 * IO.h
 *
 *  Created on: 04/10/2015
 *      Author: alberto
 */

#ifndef IO_H_
#define IO_H_

#include "Olympus.h"

class IO
{
public:
	IO();
	virtual ~IO();

	static void IO::load_ratings(const string);
	static void IO::load_targets(const string);
	static void IO::print_exec_time();
	static void IO::print_ratings(int, bool);
	static void IO::print_ratings();
	static void IO::print_similarities(int, int);
	static void write_predictions(const string);
	static void IO::print_predictions();
};

#endif /* IO_H_ */
