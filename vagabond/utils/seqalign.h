// blasty
// Copyright (C) 2019 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#ifndef __abmap__blast__
#define __abmap__blast__

#include <cstring>
#include <cmath>
#include <iostream>
#include <sstream>

#define IGNORED -3
#define SNP -2
#define MATCHED -1
#define UNTOUCHED 9

typedef struct
{
	std::string seq;
	char *mask;
	size_t *map;

} Alignment;

inline size_t mask_length(Alignment al, size_t threshold)
{
	size_t total = 0;

	for (size_t i = 0; i < al.seq.length(); i++)
	{
		if (al.mask[i] == UNTOUCHED || 
		    al.mask[i] > (char)threshold)
		{
			total++;
		}
	}
	
	return total;
}

inline int choose_seed(Alignment al, size_t threshold)
{
	size_t length = mask_length(al, threshold);
	
	if (length == 0)
	{
		return -1;
	}

	size_t seed = rand() % length;
	
	size_t count = 0;
	for (size_t i = 0; i < al.seq.length(); i++)
	{
		bool acceptable = (al.mask[i] == UNTOUCHED || 
		                   al.mask[i] > (char)threshold);

		if (!acceptable)
		{
			seed++;
		}
		
		if (i == seed)
		{
			count = i;
			break;
		}
	}
	
	return count;
}

/* gets alignment limits from other side */
inline void find_limits(Alignment ala, Alignment alb, size_t pos,
                 size_t *low, size_t *high, double *pct)
{
	*low = 0;
	*high = alb.seq.size();
	int low_idx = 0;
	int high_idx = ala.seq.size();

	for (size_t i = pos; i < ala.seq.size(); i++)
	{
		if (ala.map[i] != std::string::npos)
		{
			*high = ala.map[i];
			high_idx = i;
			break;
		}
	}

	for (int i = pos; i >= 0; i--)
	{
		if (ala.map[i] != std::string::npos)
		{
			*low = ala.map[i];
			low_idx = i + 1;
			break;
		}
	}
	
	*pct = (pos - low_idx) / (high_idx - low_idx);
}

inline size_t best_match(Alignment ala, Alignment alb, size_t pos, int threshold)
{
	int best_score = -2;
	size_t bstart = 0;
	size_t length = 0;
	size_t astart = 0;
	double best_pct_diff = 1;
	
	size_t low = 0;
	size_t high = alb.seq.length();
	double pct = 0;
	find_limits(ala, alb, pos, &low, &high, &pct);
	
	for (size_t i = low; i < high; i++)
	{
		if (alb.seq[i] != ala.seq[pos])
		{
			continue;
		}
		
		int count = -2; // two auto-hits will happen

		int as = pos;
		int bs = i;
		for (int j = (int)i; j >= 0; j--)
		{
			if (alb.seq[j] != ala.seq[as] 
			    || ala.mask[as] == MATCHED
			    || alb.mask[j] == MATCHED)
			{
				bs++; as++;
				break;
			}
			
			count++;
			bs--; as--;
			
			if (as < 0 || bs < 0)
			{
				as++;
				bs++;
				break;
			}
		}
		
		int ae = pos;
		int be = i;
		for (size_t j = i; j < alb.seq.size(); j++)
		{
			if (alb.seq[j] != ala.seq[ae] || 
			    ala.mask[ae] == MATCHED || 
			    alb.mask[j] == MATCHED)
			{
				break;
			}
			
			count++;
			be++;
			ae++;
			
			if (ae > (int)ala.seq.size())
			{
				ae--;
				break;
			}
		}
		
		if (count > best_score)
		{
			best_score = count;
			bstart = bs;
			astart = as;
			length = ae - as;
			
			double my_pct = (i - low) / (high - low);
			best_pct_diff = fabs(my_pct - pct);
		}
		else if (count == best_score)
		{
			double my_pct = (i - low) / (high - low);
			double pct_diff = fabs(my_pct - pct);
			
			if (pct_diff < best_pct_diff)
			{
				best_score = count;
				bstart = bs;
				astart = as;
				length = ae - as;
				best_pct_diff = pct_diff;
			}
		}
	}
	
	if (best_score < threshold)
	{
		ala.mask[pos] = threshold;
		return 0;
	}
	
	for (size_t i = astart; i < astart + length; i++)
	{
		ala.mask[i] = MATCHED;
		ala.map[i] = i - (astart - bstart);
	}

	for (size_t i = bstart; i < bstart + length; i++)
	{
		alb.mask[i] = MATCHED;
		alb.map[i] = i - (bstart - astart);
	}
	
	return best_score;
}

inline bool isIgnored(int mask)
{
	return (mask == IGNORED);
}

inline bool isSNP(int mask)
{
	return (mask == SNP);
}

inline bool isMatched(int mask)
{
	return (mask == MATCHED || mask == IGNORED);
}

inline bool isAdditional(int mask)
{
	return !isSNP(mask) && !isMatched(mask);
}

inline bool isValid(int map1, int map2)
{
	return (map1 != std::string::npos && map2 != std::string::npos);
}

inline bool isContiguous(int map1, int map2)
{
	return map2 == map1 + 1;
}

inline void print_alignments(Alignment &ala, Alignment &alb,
                             std::ostringstream &leftseq, 
                             std::ostringstream &aligned, 
                             std::ostringstream &rightseq,
                             std::vector<int> &indices)
{
	int plus = 0;

	for (size_t i = 0; i < ala.seq.length(); i++)
	{
		size_t j = ala.map[i];
		size_t next_j = j + 1;

		if (i < ala.seq.length() - 1)
		{
			next_j = ala.map[i + 1];
		}

		unsigned char ac = ala.seq[i];
		unsigned char bc = ' ';
		
		/* if we know where we are, use it */
		if (j < alb.seq.size() && j != std::string::npos)
		{
			bc = alb.seq[j];

			if (alb.seq[j] == ' ')
			{
				bc = '?';
			}
		}
		
		if (ac == ' ')
		{
			ac = '?';
		}

		if (isIgnored(ala.mask[i]) ||  // just a space
		    (!isAdditional(ala.mask[i])))// && isValid(j, next_j) &&
		   // (isContiguous(j, next_j) || !isValid(j, next_j))))
		{
			leftseq << ac;
			rightseq << bc;
			plus = 0;

			if (ala.mask[i] == MATCHED)
			{
				aligned << ".";
			}
			else if (ala.mask[i] == SNP)
			{
				aligned << "*";
			}
			else 
			{
				aligned << ".";
			}

			indices.push_back(j);
		}
		if (isValid(j, next_j) && !isContiguous(j, next_j)
		    && !isIgnored(ala.mask[i]))
		{
			/* some in alignment B which is not in alignment A */

			size_t start = j + 1;
			size_t end = next_j;

			for (size_t k = start; k < end; k++)
			{
				leftseq << ' ';
				bc = alb.seq[k];
				rightseq << bc;
				aligned << "-";
				indices.push_back(k);
			}
		}
		else if (!isIgnored(ala.mask[i]) && ala.map[i] == std::string::npos)
		{
			/* have something in A not in B */
			leftseq << ac;
			rightseq << ' ';
			aligned << "+";
			plus++;
			
			if (!isValid(j, j))
			{
				indices.push_back(i + plus);
				continue;
			}

			indices.push_back(j + plus);

			continue;
		}
	}
}

inline void print_masks(Alignment &al)
{
	for (size_t i = 0; i < al.seq.length(); i++)
	{
		if ((int)al.mask[i] == MATCHED)
		{
			std::cout << ".";
		}
		else
		{
			std::cout << (int)al.mask[i];
		}
	}
	std::cout << std::endl;
}

inline void print_map(Alignment &al)
{
	for (size_t i = 0; i < al.seq.length(); i++)
	{
		if (al.map[i] == std::string::npos)
		{
			std::cout << ".";
		}
		else
		{
			std::cout << " " << (int)al.map[i] << " ";
		}
	}
	std::cout << std::endl;
}

inline void loop_alignment(Alignment &ala, Alignment &alb)
{
	int count = 0;
	int threshold = 6;

	while (true)
	{
		int aseed = choose_seed(ala, threshold);

		if (aseed < 0)
		{
			break;
		}

		int match = best_match(ala, alb, aseed, threshold);
		
		count++;

		if (match == 0)
		{
			int more = false;
			for (size_t i = 0; i < ala.seq.length(); i++)
			{
				if (ala.mask[i] == UNTOUCHED || 
				    ala.mask[i] > (char)threshold)
				{
					more = true;
				}
			}
			
			if (!more)
			{
				threshold--;
				if (threshold < 1)
				{
					break;
				}
			}
		}
	}
}

inline void score_alignment(Alignment ala, Alignment alb,
                     int *muts, int *dels)
{
	int diff = ala.seq.length() - alb.seq.length();
	*dels = std::abs(diff);
	
	int total = 0;
	for (size_t i = 0; i < ala.seq.length(); i++)
	{
		if ((int)ala.mask[i] != MATCHED)
		{
			total++;
		}
	}

	for (size_t i = 0; i < alb.seq.length(); i++)
	{
		if ((int)alb.mask[i] != MATCHED)
		{
			total++;
		}
	}
	
	*muts = total;
}

inline void setup_alignment(Alignment *ala, std::string a)
{
	ala->seq = a;
	ala->mask = new char[a.length()];
	ala->map = new size_t[a.length()];
	memset(ala->mask, UNTOUCHED, ala->seq.length() * sizeof(char));
	
	for (size_t i = 0; i < a.length(); i++)
	{
		ala->map[i] = std::string::npos;
	}
}

inline void delete_alignment(Alignment *ala)
{
	delete [] ala->mask;
	delete [] ala->map;
}

inline void compare_sequences_and_alignments(std::string a, std::string b,
                                             int *muts, int *dels, 
                                             Alignment &besta, 
                                             Alignment &bestb, int tries = 10)
{
	int best_mut = a.length() + 1; // to ensure no double deletion
	int improved = 0;

	for (size_t i = 0; i < tries; i++)
	{
		int mut;
	
		Alignment ala, alb;
		setup_alignment(&ala, a);
		setup_alignment(&alb, b);

		loop_alignment(ala, alb);
		score_alignment(ala, alb, &mut, dels);
		
		if (mut < best_mut)
		{
			improved++;
			best_mut = mut;
			delete_alignment(&besta);
			delete_alignment(&bestb);
			besta = ala;
			bestb = alb;
		}
	}
	
	*muts = best_mut;
}

inline void match_spaces(Alignment &ala)
{
	for (size_t i = 0; i < ala.seq.size(); i++)
	{
		if (ala.seq[i] == ' ')
		{
			ala.mask[i] = IGNORED;
		}
	}
}

inline void tidy_alignments(Alignment &ala, Alignment &alb)
{
	match_spaces(ala);
	match_spaces(alb);

	int start = 0;
	for (size_t i = 0; i < ala.seq.size(); i++)
	{
		if (ala.mask[i] != MATCHED)
		{
			start++;
		}
		else
		{
			break;
		}
	}

	for (size_t i = start; i < ala.seq.size(); i++)
	{
		/* nothing to see here */
		if (isMatched(ala.mask[i]))
		{
			if (isIgnored(ala.mask[i]) &&
			    ala.map[i - 1] != std::string::npos)
			{
				ala.map[i] = ala.map[i - 1] + 1;
			}

			continue;
		}

		/* something isn't matching - either del or SNP */ 
		/* find previously identified residue in alb    */

		/* save our position for later */
		int si = i;
		
		/* we may still be in the middle of an A-extension */
		if (ala.map[i - 1] == std::string::npos)
		{
			continue;
		}

		/* pick up from before we branched */
		int j = ala.map[i - 1] + 1;

		int sj = j;
		
		/* stop if our calculated J is off the end of B */
		if (j > alb.seq.size())
		{
			continue;
		}
		
		if (isIgnored(alb.mask[j]))
		{
			ala.mask[i] = IGNORED;
			ala.map[i] = j;

			continue;
		}

		bool failed = false;
		bool was_a = false;
		
		/* now we march forwards, counting who runs out of UNMATCHED first */
		/* MATCHED check comes first */
		while (!failed)
		{
			if (i >= ala.seq.size())
			{
				// blargh, ends, ignore for now
				was_a = true;
				failed = true;
				break;
			}
			if (j >= alb.seq.size())
			{
				failed = true;
				break;
			}
			
			int amask = ala.mask[i];
			int bmask = alb.mask[j];

			if (!isMatched(amask) && !isMatched(bmask))
			{
				/* nothing's touched base yet */

				i++; j++;
			}
			else if (isMatched(amask) && !isMatched(bmask))
			{
				/* woops, a made sense again first */
				was_a = true;
				failed = true;
				continue;
			}
			else if (isMatched(bmask) && !isMatched(amask))
			{
				/* b made sense again first */
				failed = true;
				continue;
			}
			else if (isMatched(bmask) && isMatched(amask))
			{
				/* both rematched simultaneously */
				failed = false;
				break;
			}
		}
		
		/* if we had equal numbers of matched, we must mark them
		 * as SNPS */
		if (!failed)
		{
			int ei = i;
			if (ei == 0)
			{
				continue;
			}

			for (size_t k = si; k < ei && k < ala.seq.size(); k++)
			{
				ala.map[k] = ala.map[k-1] + 1;
				if (ala.mask[k] != IGNORED)
				{
					ala.mask[k] = SNP;
				}
			}

			if (sj == 0)
			{
				continue;
			}

			for (size_t j = sj; j < ei && j < alb.seq.size(); j++)
			{
				alb.map[j] = alb.map[j-1] + 1;
				if (alb.mask[j] != IGNORED)
				{
					alb.mask[j] = SNP;
				}
			}
		}
	}
}

inline void compare_sequences(std::string a, std::string b,
                              int *muts, int *dels)
{
	Alignment besta, bestb;
	setup_alignment(&besta, a);
	setup_alignment(&bestb, b);
	
	compare_sequences_and_alignments(a, b, muts, dels, besta, bestb);

	delete_alignment(&besta);
	delete_alignment(&bestb);
}

#endif

