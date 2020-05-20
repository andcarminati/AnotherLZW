/*
Another simple implementation of LZW compression algorithm
Copyright (C) 2020  Andreu Carminati
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LZW_H
#define	LZW_H


// This macro defines the amount of dictionary entries that we can use, i.e, it 
// can dictates the compression ratio obtained. The rationale beyoind the macro is
// that we dont use dynamic allocation, we use a single large buffer that is splited
// on demand at each time a dictionary entry is created.
#define MAX_SEQ_BUFF 4000


void reset();
void dumpstat();
int encode(int size_in, unsigned char* in_data, int size_out, unsigned char* out_data);
int decode(int size_in, unsigned char* in_data, int size_out, unsigned char* out_data);

#endif	/* LZW_H */

