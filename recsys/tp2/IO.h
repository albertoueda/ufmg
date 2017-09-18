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

	static void load_content(const string);
	static void load_ratings(const string);
	static void load_targets(const string);
	static void load_stopwords(const string);

	static void print_exec_time();
	static void print_ratings(int, bool);
	static void print_ratings();
	static void print_predictions();
	static void print_vocabulary();
	static void print_vocabulary(int);
	static void print_sorted_vocabulary(int);

	static void write_predictions(const string);
};

#endif /* IO_H_ */
