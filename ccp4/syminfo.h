const char *syminfo = 
"# This file contains a list of spacegroups, each in a\n"\
"# number of settings. Each setting is delimited by\n"\
"# begin_spacegroup / end_spacegroup records. For each\n"\
"# spacegroup setting, the following are listed:\n"\
"#   number = standard spacegroup number\n"\
"#   basisop = change of basis operator\n"\
"#   symbol ccp4 = CCP4 spacegroup number e.g. 1003\n"\
"#                 (0 if not a CCP4 group)\n"\
"#   symbol Hall = Hall symbol\n"\
"#   symbol xHM =  extended Hermann Mauguin symbol\n"\
"#   symbol old =  CCP4 spacegroup name\n"\
"#                 (blank if not a CCP4 group)\n"\
"#   symbol laue = Laue group symbol\n"\
"#   symbol patt = Patterson group symbol\n"\
"#   symbol pgrp = Point group symbol\n"\
"#   hklasu ccp4 = reciprocal space asymmetric unit\n"\
"#                 (with respect to standard setting)\n"\
"#   mapasu ccp4 = CCP4 real space asymmetric unit\n"\
"#                 (with respect to standard setting)\n"\
"#                 (negative ranges if not a CCP4 group)\n"\
"#   mapasu zero = origin based real space asymmetric unit\n"\
"#                 (with respect to current setting)\n"\
"#   mapasu nonz = non-origin based real space asymmetric unit\n"\
"#                 (with respect to current setting)\n"\
"#   cheshire = Cheshire cell\n"\
"#                 (with respect to standard setting)\n"\
"#   symop = list of primitive symmetry operators\n"\
"#   cenop = list of centering operators\n"\
"#\n"\
"begin_spacegroup\n"\
"number  1\n"\
"basisop x,y,z\n"\
"symbol ccp4 1\n"\
"symbol Hall ' P 1'\n"\
"symbol xHM  'P 1'\n"\
"symbol old  'P 1'\n"\
"symbol laue '-P 1' '-1'\n"\
"symbol patt '-P 1' '-1'\n"\
"symbol pgrp ' P 1' '1'\n"\
"hklasu ccp4 'l>0 or (l==0 and (h>0 or (h==0 and k>=0)))'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<1\n"\
"mapasu zero 0<=x<1; 0<=y<1; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<1; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=0; 0<=z<=0\n"\
"symop x,y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  2\n"\
"basisop x,y,z\n"\
"symbol ccp4 2\n"\
"symbol Hall '-P 1'\n"\
"symbol xHM  'P -1'\n"\
"symbol old  'P 1-'\n"\
"symbol laue '-P 1' '-1'\n"\
"symbol patt '-P 1' '-1'\n"\
"symbol pgrp '-P 1' '-1'\n"\
"hklasu ccp4 'l>0 or (l==0 and (h>0 or (h==0 and k>=0)))'\n"\
"mapasu ccp4 0<=x<1; 0<=y<=1/2; 0<=z<1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  3\n"\
"basisop x,y,z\n"\
"symbol ccp4 3\n"\
"symbol Hall ' P 2y'\n"\
"symbol xHM  'P 1 2 1'\n"\
"symbol old  'P 1 2 1'\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-P 2y' '2/m'\n"\
"symbol pgrp ' P 2y' '2'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<1; 0<=z<1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=0; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  3\n"\
"basisop z,x,y\n"\
"symbol ccp4 1003\n"\
"symbol Hall ' P 2y (z,x,y)'\n"\
"symbol xHM  'P 1 1 2'\n"\
"symbol old  'P 1 1 2'\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-P 2' '2/m'\n"\
"symbol pgrp ' P 2' '2'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<1; 0<=z<1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=0; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  3\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2y (y,z,x)'\n"\
"symbol xHM  'P 2 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-P 2x' '2/m'\n"\
"symbol pgrp ' P 2x' '2'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=0; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  4\n"\
"basisop x,y,z\n"\
"symbol ccp4 4\n"\
"symbol Hall ' P 2yb'\n"\
"symbol xHM  'P 1 21 1'\n"\
"symbol old  'P 1 21 1'\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-P 2y' '2/m'\n"\
"symbol pgrp ' P 2y' '2'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1/2; 0<=z<1\n"\
"mapasu zero 0<=x<1; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=0; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  4\n"\
"basisop z,x,y\n"\
"symbol ccp4 1004\n"\
"symbol Hall ' P 2yb (z,x,y)'\n"\
"symbol xHM  'P 1 1 21'\n"\
"symbol old  'P 1 1 21'\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-P 2' '2/m'\n"\
"symbol pgrp ' P 2' '2'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<1/2\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=0; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  4\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2yb (y,z,x)'\n"\
"symbol xHM  'P 21 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-P 2x' '2/m'\n"\
"symbol pgrp ' P 2x' '2'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=0; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  5\n"\
"basisop x,y,z\n"\
"symbol ccp4 5\n"\
"symbol Hall ' C 2y'\n"\
"symbol xHM  'C 1 2 1'\n"\
"symbol old  'C 1 2 1'\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-C 2y' '2/m'\n"\
"symbol pgrp ' P 2y' '2'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=0; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  5\n"\
"basisop z,y,-x\n"\
"symbol ccp4 2005\n"\
"symbol Hall ' C 2y (z,y,-x)'\n"\
"symbol xHM  'A 1 2 1'\n"\
"symbol old  'A 1 2 1' 'A 2'\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-A 2y' '2/m'\n"\
"symbol pgrp ' P 2y' '2'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=0; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  5\n"\
"basisop x,y,-x+z\n"\
"symbol ccp4 4005\n"\
"symbol Hall ' C 2y (x,y,-x+z)'\n"\
"symbol xHM  'I 1 2 1'\n"\
"symbol old  'I 1 2 1' 'I 2'\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-I 2y' '2/m'\n"\
"symbol pgrp ' P 2y' '2'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=0; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  5\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C 2y (z,x,y)'\n"\
"symbol xHM  'A 1 1 2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-A 2' '2/m'\n"\
"symbol pgrp ' P 2' '2'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=0; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  5\n"\
"basisop -x,z,y\n"\
"symbol ccp4 1005\n"\
"symbol Hall ' C 2y (-x,z,y)'\n"\
"symbol xHM  'B 1 1 2'\n"\
"symbol old  'B 1 1 2' 'B 2'\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-B 2' '2/m'\n"\
"symbol pgrp ' P 2' '2'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<1; 0<=y<=1/2; 0<=z<1/2\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=0; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  5\n"\
"basisop -x+z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C 2y (-x+z,x,y)'\n"\
"symbol xHM  'I 1 1 2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-I 2' '2/m'\n"\
"symbol pgrp ' P 2' '2'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=0; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  5\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C 2y (y,z,x)'\n"\
"symbol xHM  'B 2 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-B 2x' '2/m'\n"\
"symbol pgrp ' P 2x' '2'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=0; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  5\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C 2y (y,-x,z)'\n"\
"symbol xHM  'C 2 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-C 2x' '2/m'\n"\
"symbol pgrp ' P 2x' '2'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=0; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  5\n"\
"basisop y,-x+z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C 2y (y,-x+z,x)'\n"\
"symbol xHM  'I 2 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-I 2x' '2/m'\n"\
"symbol pgrp ' P 2x' '2'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=0; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  6\n"\
"basisop x,y,z\n"\
"symbol ccp4 6\n"\
"symbol Hall ' P -2y'\n"\
"symbol xHM  'P 1 m 1'\n"\
"symbol old  'P 1 m 1'\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-P 2y' '2/m'\n"\
"symbol pgrp ' P -2y' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  6\n"\
"basisop z,x,y\n"\
"symbol ccp4 1006\n"\
"symbol Hall ' P -2y (z,x,y)'\n"\
"symbol xHM  'P 1 1 m'\n"\
"symbol old  'P 1 1 m'\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-P 2' '2/m'\n"\
"symbol pgrp ' P -2' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1; 0<=y<1; 0<=z<=1/2\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  6\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P -2y (y,z,x)'\n"\
"symbol xHM  'P m 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-P 2x' '2/m'\n"\
"symbol pgrp ' P -2x' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  7\n"\
"basisop x,y,z\n"\
"symbol ccp4 7\n"\
"symbol Hall ' P -2yc'\n"\
"symbol xHM  'P 1 c 1'\n"\
"symbol old  'P 1 c 1'\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-P 2y' '2/m'\n"\
"symbol pgrp ' P -2y' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  7\n"\
"basisop x-z,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P -2yc (x-z,y,z)'\n"\
"symbol xHM  'P 1 n 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-P 2y' '2/m'\n"\
"symbol pgrp ' P -2y' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  7\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P -2yc (z,y,-x)'\n"\
"symbol xHM  'P 1 a 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-P 2y' '2/m'\n"\
"symbol pgrp ' P -2y' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  7\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P -2yc (z,x,y)'\n"\
"symbol xHM  'P 1 1 a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-P 2' '2/m'\n"\
"symbol pgrp ' P -2' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<1; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<1; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  7\n"\
"basisop z,x-z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P -2yc (z,x-z,y)'\n"\
"symbol xHM  'P 1 1 n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-P 2' '2/m'\n"\
"symbol pgrp ' P -2' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x+1/2,y+1/2,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  7\n"\
"basisop -x,z,y\n"\
"symbol ccp4 1007\n"\
"symbol Hall ' P -2yc (-x,z,y)'\n"\
"symbol xHM  'P 1 1 b'\n"\
"symbol old  'P 1 1 b'\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-P 2' '2/m'\n"\
"symbol pgrp ' P -2' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,y+1/2,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  7\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P -2yc (y,z,x)'\n"\
"symbol xHM  'P b 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-P 2x' '2/m'\n"\
"symbol pgrp ' P -2x' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  7\n"\
"basisop y,z,x-z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P -2yc (y,z,x-z)'\n"\
"symbol xHM  'P n 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-P 2x' '2/m'\n"\
"symbol pgrp ' P -2x' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  7\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P -2yc (y,-x,z)'\n"\
"symbol xHM  'P c 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-P 2x' '2/m'\n"\
"symbol pgrp ' P -2x' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  8\n"\
"basisop x,y,z\n"\
"symbol ccp4 8\n"\
"symbol Hall ' C -2y'\n"\
"symbol xHM  'C 1 m 1'\n"\
"symbol old  'C 1 m 1'\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-C 2y' '2/m'\n"\
"symbol pgrp ' P -2y' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  8\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C -2y (z,y,-x)'\n"\
"symbol xHM  'A 1 m 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-A 2y' '2/m'\n"\
"symbol pgrp ' P -2y' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  8\n"\
"basisop x,y,-x+z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C -2y (x,y,-x+z)'\n"\
"symbol xHM  'I 1 m 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-I 2y' '2/m'\n"\
"symbol pgrp ' P -2y' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  8\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C -2y (z,x,y)'\n"\
"symbol xHM  'A 1 1 m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-A 2' '2/m'\n"\
"symbol pgrp ' P -2' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1; 0<=y<1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,y,-z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  8\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C -2y (-x,z,y)'\n"\
"symbol xHM  'B 1 1 m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-B 2' '2/m'\n"\
"symbol pgrp ' P -2' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<1; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1/2; 0<=y<1; 0<=z<=1/2\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,y,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  8\n"\
"basisop -x+z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C -2y (-x+z,x,y)'\n"\
"symbol xHM  'I 1 1 m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-I 2' '2/m'\n"\
"symbol pgrp ' P -2' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1; 0<=y<1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,y,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  8\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C -2y (y,z,x)'\n"\
"symbol xHM  'B m 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-B 2x' '2/m'\n"\
"symbol pgrp ' P -2x' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  8\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C -2y (y,-x,z)'\n"\
"symbol xHM  'C m 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-C 2x' '2/m'\n"\
"symbol pgrp ' P -2x' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  8\n"\
"basisop y,-x+z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C -2y (y,-x+z,x)'\n"\
"symbol xHM  'I m 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-I 2x' '2/m'\n"\
"symbol pgrp ' P -2x' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  9\n"\
"basisop x,y,z\n"\
"symbol ccp4 9\n"\
"symbol Hall ' C -2yc'\n"\
"symbol xHM  'C 1 c 1'\n"\
"symbol old  'C 1 c 1'\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-C 2y' '2/m'\n"\
"symbol pgrp ' P -2y' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  9\n"\
"basisop z,y,-x+z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C -2yc (z,y,-x+z)'\n"\
"symbol xHM  'A 1 n 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-A 2y' '2/m'\n"\
"symbol pgrp ' P -2y' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  9\n"\
"basisop x+z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C -2yc (x+z,y,-x)'\n"\
"symbol xHM  'I 1 a 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-I 2y' '2/m'\n"\
"symbol pgrp ' P -2y' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  9\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C -2yc (z,y,-x)'\n"\
"symbol xHM  'A 1 a 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-A 2y' '2/m'\n"\
"symbol pgrp ' P -2y' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  9\n"\
"basisop x+1/4,y+1/4,z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C -2yc (x+1/4,y+1/4,z)'\n"\
"symbol xHM  'C 1 n 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-C 2y' '2/m'\n"\
"symbol pgrp ' P -2y' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  9\n"\
"basisop x,y,-x+z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C -2yc (x,y,-x+z)'\n"\
"symbol xHM  'I 1 c 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-I 2y' '2/m'\n"\
"symbol pgrp ' P -2y' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  9\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C -2yc (z,x,y)'\n"\
"symbol xHM  'A 1 1 a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-A 2' '2/m'\n"\
"symbol pgrp ' P -2' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  9\n"\
"basisop -x+z,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C -2yc (-x+z,z,y)'\n"\
"symbol xHM  'B 1 1 n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-B 2' '2/m'\n"\
"symbol pgrp ' P -2' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x+1/2,y+1/2,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  9\n"\
"basisop -x,x+z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C -2yc (-x,x+z,y)'\n"\
"symbol xHM  'I 1 1 b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-I 2' '2/m'\n"\
"symbol pgrp ' P -2' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,y+1/2,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  9\n"\
"basisop -x,z,y\n"\
"symbol ccp4 1009\n"\
"symbol Hall ' C -2yc (-x,z,y)'\n"\
"symbol xHM  'B 1 1 b'\n"\
"symbol old  'B 1 1 b'\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-B 2' '2/m'\n"\
"symbol pgrp ' P -2' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,y+1/2,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  9\n"\
"basisop z,x-z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C -2yc (z,x-z,y)'\n"\
"symbol xHM  'A 1 1 n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-A 2' '2/m'\n"\
"symbol pgrp ' P -2' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x+1/2,y+1/2,-z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  9\n"\
"basisop -x+z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C -2yc (-x+z,x,y)'\n"\
"symbol xHM  'I 1 1 a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-I 2' '2/m'\n"\
"symbol pgrp ' P -2' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  9\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C -2yc (y,z,x)'\n"\
"symbol xHM  'B b 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-B 2x' '2/m'\n"\
"symbol pgrp ' P -2x' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  9\n"\
"basisop y,-x+z,z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C -2yc (y,-x+z,z)'\n"\
"symbol xHM  'C n 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-C 2x' '2/m'\n"\
"symbol pgrp ' P -2x' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1/2; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  9\n"\
"basisop y,-x,x+z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C -2yc (y,-x,x+z)'\n"\
"symbol xHM  'I c 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-I 2x' '2/m'\n"\
"symbol pgrp ' P -2x' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  9\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C -2yc (y,-x,z)'\n"\
"symbol xHM  'C c 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-C 2x' '2/m'\n"\
"symbol pgrp ' P -2x' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  9\n"\
"basisop y,z,x-z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C -2yc (y,z,x-z)'\n"\
"symbol xHM  'B n 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-B 2x' '2/m'\n"\
"symbol pgrp ' P -2x' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  9\n"\
"basisop y,-x+z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C -2yc (y,-x+z,x)'\n"\
"symbol xHM  'I b 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-I 2x' '2/m'\n"\
"symbol pgrp ' P -2x' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1/2; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1; 0<=z<1\n"\
"cheshire 0<=x<=0; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  10\n"\
"basisop x,y,z\n"\
"symbol ccp4 10\n"\
"symbol Hall '-P 2y'\n"\
"symbol xHM  'P 1 2/m 1'\n"\
"symbol old  'P 1 2/m 1'\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-P 2y' '2/m'\n"\
"symbol pgrp '-P 2y' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop -x,-y,-z\n"\
"symop x,-y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  10\n"\
"basisop z,x,y\n"\
"symbol ccp4 1010\n"\
"symbol Hall '-P 2y (z,x,y)'\n"\
"symbol xHM  'P 1 1 2/m'\n"\
"symbol old  'P 1 1 2/m'\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-P 2' '2/m'\n"\
"symbol pgrp '-P 2' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop -x,-y,-z\n"\
"symop x,y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  10\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2y (y,z,x)'\n"\
"symbol xHM  'P 2/m 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-P 2x' '2/m'\n"\
"symbol pgrp '-P 2x' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop -x,-y,-z\n"\
"symop -x,y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  11\n"\
"basisop x,y,z\n"\
"symbol ccp4 11\n"\
"symbol Hall '-P 2yb'\n"\
"symbol xHM  'P 1 21/m 1'\n"\
"symbol old  'P 1 21/m 1'\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-P 2y' '2/m'\n"\
"symbol pgrp '-P 2y' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop x,-y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  11\n"\
"basisop z,x,y\n"\
"symbol ccp4 1011\n"\
"symbol Hall '-P 2yb (z,x,y)'\n"\
"symbol xHM  'P 1 1 21/m'\n"\
"symbol old  'P 1 1 21/m'\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-P 2' '2/m'\n"\
"symbol pgrp '-P 2' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/2; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,y,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  11\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2yb (y,z,x)'\n"\
"symbol xHM  'P 21/m 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-P 2x' '2/m'\n"\
"symbol pgrp '-P 2x' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,-z\n"\
"symop -x,-y,-z\n"\
"symop -x+1/2,y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  12\n"\
"basisop x,y,z\n"\
"symbol ccp4 12\n"\
"symbol Hall '-C 2y'\n"\
"symbol xHM  'C 1 2/m 1'\n"\
"symbol old  'C 1 2/m 1'\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-C 2y' '2/m'\n"\
"symbol pgrp '-P 2y' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop -x,-y,-z\n"\
"symop x,-y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  12\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2y (z,y,-x)'\n"\
"symbol xHM  'A 1 2/m 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-A 2y' '2/m'\n"\
"symbol pgrp '-P 2y' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop -x,-y,-z\n"\
"symop x,-y,z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  12\n"\
"basisop x,y,-x+z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2y (x,y,-x+z)'\n"\
"symbol xHM  'I 1 2/m 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-I 2y' '2/m'\n"\
"symbol pgrp '-P 2y' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop -x,-y,-z\n"\
"symop x,-y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  12\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2y (z,x,y)'\n"\
"symbol xHM  'A 1 1 2/m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-A 2' '2/m'\n"\
"symbol pgrp '-P 2' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop -x,-y,-z\n"\
"symop x,y,-z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  12\n"\
"basisop -x,z,y\n"\
"symbol ccp4 1012\n"\
"symbol Hall '-C 2y (-x,z,y)'\n"\
"symbol xHM  'B 1 1 2/m'\n"\
"symbol old  'B 1 1 2/m'\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-B 2' '2/m'\n"\
"symbol pgrp '-P 2' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop -x,-y,-z\n"\
"symop x,y,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  12\n"\
"basisop -x+z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2y (-x+z,x,y)'\n"\
"symbol xHM  'I 1 1 2/m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-I 2' '2/m'\n"\
"symbol pgrp '-P 2' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop -x,-y,-z\n"\
"symop x,y,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  12\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2y (y,z,x)'\n"\
"symbol xHM  'B 2/m 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-B 2x' '2/m'\n"\
"symbol pgrp '-P 2x' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop -x,-y,-z\n"\
"symop -x,y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  12\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2y (y,-x,z)'\n"\
"symbol xHM  'C 2/m 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-C 2x' '2/m'\n"\
"symbol pgrp '-P 2x' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop -x,-y,-z\n"\
"symop -x,y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  12\n"\
"basisop y,-x+z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2y (y,-x+z,x)'\n"\
"symbol xHM  'I 2/m 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-I 2x' '2/m'\n"\
"symbol pgrp '-P 2x' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop -x,-y,-z\n"\
"symop -x,y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  13\n"\
"basisop x,y,z\n"\
"symbol ccp4 13\n"\
"symbol Hall '-P 2yc'\n"\
"symbol xHM  'P 1 2/c 1'\n"\
"symbol old  'P 1 2/c 1'\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-P 2y' '2/m'\n"\
"symbol pgrp '-P 2y' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  13\n"\
"basisop x-z,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2yc (x-z,y,z)'\n"\
"symbol xHM  'P 1 2/n 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-P 2y' '2/m'\n"\
"symbol pgrp '-P 2y' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,-y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  13\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2yc (z,y,-x)'\n"\
"symbol xHM  'P 1 2/a 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-P 2y' '2/m'\n"\
"symbol pgrp '-P 2y' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,y,-z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,-y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  13\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2yc (z,x,y)'\n"\
"symbol xHM  'P 1 1 2/a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-P 2' '2/m'\n"\
"symbol pgrp '-P 2' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y,z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  13\n"\
"basisop z,x-z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2yc (z,x-z,y)'\n"\
"symbol xHM  'P 1 1 2/n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-P 2' '2/m'\n"\
"symbol pgrp '-P 2' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y+1/2,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  13\n"\
"basisop -x,z,y\n"\
"symbol ccp4 1013\n"\
"symbol Hall '-P 2yc (-x,z,y)'\n"\
"symbol xHM  'P 1 1 2/b'\n"\
"symbol old  'P 1 1 2/b'\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-P 2' '2/m'\n"\
"symbol pgrp '-P 2' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y+1/2,z\n"\
"symop -x,-y,-z\n"\
"symop x,y+1/2,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  13\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2yc (y,z,x)'\n"\
"symbol xHM  'P 2/b 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-P 2x' '2/m'\n"\
"symbol pgrp '-P 2x' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop -x,y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  13\n"\
"basisop y,z,x-z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2yc (y,z,x-z)'\n"\
"symbol xHM  'P 2/n 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-P 2x' '2/m'\n"\
"symbol pgrp '-P 2x' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop -x,y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  13\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2yc (y,-x,z)'\n"\
"symbol xHM  'P 2/c 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-P 2x' '2/m'\n"\
"symbol pgrp '-P 2x' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop -x,y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  14\n"\
"basisop x,y,z\n"\
"symbol ccp4 14\n"\
"symbol Hall '-P 2ybc'\n"\
"symbol xHM  'P 1 21/c 1'\n"\
"symbol old  'P 1 21/c 1'\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-P 2y' '2/m'\n"\
"symbol pgrp '-P 2y' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,-y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  14\n"\
"basisop x-z,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ybc (x-z,y,z)'\n"\
"symbol xHM  'P 1 21/n 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-P 2y' '2/m'\n"\
"symbol pgrp '-P 2y' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,y+1/2,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  14\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ybc (z,y,-x)'\n"\
"symbol xHM  'P 1 21/a 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-P 2y' '2/m'\n"\
"symbol pgrp '-P 2y' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,y+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,-y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  14\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ybc (z,x,y)'\n"\
"symbol xHM  'P 1 1 21/a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-P 2' '2/m'\n"\
"symbol pgrp '-P 2' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  14\n"\
"basisop z,x-z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ybc (z,x-z,y)'\n"\
"symbol xHM  'P 1 1 21/n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-P 2' '2/m'\n"\
"symbol pgrp '-P 2' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y+1/2,z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y+1/2,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  14\n"\
"basisop -x,z,y\n"\
"symbol ccp4 1014\n"\
"symbol Hall '-P 2ybc (-x,z,y)'\n"\
"symbol xHM  'P 1 1 21/b'\n"\
"symbol old  'P 1 1 21/b'\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-P 2' '2/m'\n"\
"symbol pgrp '-P 2' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y+1/2,z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,y+1/2,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  14\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ybc (y,z,x)'\n"\
"symbol xHM  'P 21/b 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-P 2x' '2/m'\n"\
"symbol pgrp '-P 2x' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop -x+1/2,y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  14\n"\
"basisop y,z,x-z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ybc (y,z,x-z)'\n"\
"symbol xHM  'P 21/n 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-P 2x' '2/m'\n"\
"symbol pgrp '-P 2x' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x+1/2,-y+1/2,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop -x+1/2,y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  14\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ybc (y,-x,z)'\n"\
"symbol xHM  'P 21/c 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-P 2x' '2/m'\n"\
"symbol pgrp '-P 2x' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop -x+1/2,y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  15\n"\
"basisop x,y,z\n"\
"symbol ccp4 15\n"\
"symbol Hall '-C 2yc'\n"\
"symbol xHM  'C 1 2/c 1'\n"\
"symbol old  'C 1 2/c 1'\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-C 2y' '2/m'\n"\
"symbol pgrp '-P 2y' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  15\n"\
"basisop z,y,-x+z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2yc (z,y,-x+z)'\n"\
"symbol xHM  'A 1 2/n 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-A 2y' '2/m'\n"\
"symbol pgrp '-P 2y' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,-y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  15\n"\
"basisop x+z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2yc (x+z,y,-x)'\n"\
"symbol xHM  'I 1 2/a 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-I 2y' '2/m'\n"\
"symbol pgrp '-P 2y' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<=3/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,y,-z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,-y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  15\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2yc (z,y,-x)'\n"\
"symbol xHM  'A 1 2/a 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-A 2y' '2/m'\n"\
"symbol pgrp '-P 2y' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,y,-z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,-y,z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  15\n"\
"basisop x+1/4,y-1/4,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2yc (x+1/4,y-1/4,z)'\n"\
"symbol xHM  'C 1 2/n 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-C 2y' '2/m'\n"\
"symbol pgrp '-P 2y' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1/2; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<=3/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop -x+1/2,-y+1/2,-z\n"\
"symop x,-y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  15\n"\
"basisop x,y,-x+z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2yc (x,y,-x+z)'\n"\
"symbol xHM  'I 1 2/c 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-I 2y' '2/m'\n"\
"symbol pgrp '-P 2y' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  15\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2yc (z,x,y)'\n"\
"symbol xHM  'A 1 1 2/a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-A 2' '2/m'\n"\
"symbol pgrp '-P 2' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y,z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  15\n"\
"basisop -x+z,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2yc (-x+z,z,y)'\n"\
"symbol xHM  'B 1 1 2/n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-B 2' '2/m'\n"\
"symbol pgrp '-P 2' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y+1/2,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  15\n"\
"basisop -x,x+z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2yc (-x,x+z,y)'\n"\
"symbol xHM  'I 1 1 2/b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-I 2' '2/m'\n"\
"symbol pgrp '-P 2' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y+1/2,z\n"\
"symop -x,-y,-z\n"\
"symop x,y+1/2,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  15\n"\
"basisop -x,z,y\n"\
"symbol ccp4 1015\n"\
"symbol Hall '-C 2yc (-x,z,y)'\n"\
"symbol xHM  'B 1 1 2/b'\n"\
"symbol old  'B 1 1 2/b'\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-B 2' '2/m'\n"\
"symbol pgrp '-P 2' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y+1/2,z\n"\
"symop -x,-y,-z\n"\
"symop x,y+1/2,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  15\n"\
"basisop z,x-z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2yc (z,x-z,y)'\n"\
"symbol xHM  'A 1 1 2/n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-A 2' '2/m'\n"\
"symbol pgrp '-P 2' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y+1/2,-z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  15\n"\
"basisop -x+z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2yc (-x+z,x,y)'\n"\
"symbol xHM  'I 1 1 2/a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-I 2' '2/m'\n"\
"symbol pgrp '-P 2' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y,z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  15\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2yc (y,z,x)'\n"\
"symbol xHM  'B 2/b 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-B 2x' '2/m'\n"\
"symbol pgrp '-P 2x' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop -x,y+1/2,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  15\n"\
"basisop y,-x+z,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2yc (y,-x+z,z)'\n"\
"symbol xHM  'C 2/n 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-C 2x' '2/m'\n"\
"symbol pgrp '-P 2x' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<=3/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop -x,y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  15\n"\
"basisop y,-x,x+z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2yc (y,-x,x+z)'\n"\
"symbol xHM  'I 2/c 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-I 2x' '2/m'\n"\
"symbol pgrp '-P 2x' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop -x,y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  15\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2yc (y,-x,z)'\n"\
"symbol xHM  'C 2/c 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-C 2x' '2/m'\n"\
"symbol pgrp '-P 2x' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop -x,y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  15\n"\
"basisop y,z,x-z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2yc (y,z,x-z)'\n"\
"symbol xHM  'B 2/n 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-B 2x' '2/m'\n"\
"symbol pgrp '-P 2x' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop -x,y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  15\n"\
"basisop y,-x+z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2yc (y,-x+z,x)'\n"\
"symbol xHM  'I 2/b 1 1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2x' '2/m'\n"\
"symbol patt '-I 2x' '2/m'\n"\
"symbol pgrp '-P 2x' '2/m'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1/2\n"\
"mapasu nonz 1/4<=x<=3/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop -x,y+1/2,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  16\n"\
"basisop x,y,z\n"\
"symbol ccp4 16\n"\
"symbol Hall ' P 2 2'\n"\
"symbol xHM  'P 2 2 2'\n"\
"symbol old  'P 2 2 2'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 2' '222'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z\n"\
"symop -x,y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  17\n"\
"basisop x,y,z\n"\
"symbol ccp4 17\n"\
"symbol Hall ' P 2c 2'\n"\
"symbol xHM  'P 2 2 21'\n"\
"symbol old  'P 2 2 21'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 2' '222'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z+1/2\n"\
"symop x,-y,-z\n"\
"symop -x,y,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  17\n"\
"basisop z,x,y\n"\
"symbol ccp4 1017\n"\
"symbol Hall ' P 2c 2 (z,x,y)'\n"\
"symbol xHM  'P 21 2 2'\n"\
"symbol old  'P 21 2 2'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 2' '222'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<1; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,-z\n"\
"symop -x,y,-z\n"\
"symop -x+1/2,-y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  17\n"\
"basisop y,z,x\n"\
"symbol ccp4 2017\n"\
"symbol Hall ' P 2c 2 (y,z,x)'\n"\
"symbol xHM  'P 2 21 2'\n"\
"symbol old  'P 2 21 2'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 2' '222'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<1; 0<=z<=1/2\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,-z\n"\
"symop -x,-y,z\n"\
"symop x,-y+1/2,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  18\n"\
"basisop x,y,z\n"\
"symbol ccp4 18\n"\
"symbol Hall ' P 2 2ab'\n"\
"symbol xHM  'P 21 21 2'\n"\
"symbol old  'P 21 21 2'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 2' '222'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop -x+1/2,y+1/2,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  18\n"\
"basisop z,x,y\n"\
"symbol ccp4 3018\n"\
"symbol Hall ' P 2 2ab (z,x,y)'\n"\
"symbol xHM  'P 2 21 21'\n"\
"symbol old  'P 2 21 21'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 2' '222'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<=1/4\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop -x,-y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  18\n"\
"basisop y,z,x\n"\
"symbol ccp4 2018\n"\
"symbol Hall ' P 2 2ab (y,z,x)'\n"\
"symbol xHM  'P 21 2 21'\n"\
"symbol old  'P 21 2 21'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 2' '222'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/4; 0<=y<1; 0<=z<1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop x+1/2,-y,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  19\n"\
"basisop x,y,z\n"\
"symbol ccp4 19\n"\
"symbol Hall ' P 2ac 2ab'\n"\
"symbol xHM  'P 21 21 21'\n"\
"symbol old  'P 21 21 21'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 2' '222'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<=1/4\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop -x,y+1/2,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  20\n"\
"basisop x,y,z\n"\
"symbol ccp4 20\n"\
"symbol Hall ' C 2c 2'\n"\
"symbol xHM  'C 2 2 21'\n"\
"symbol old  'C 2 2 21'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 2' '222'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z+1/2\n"\
"symop x,-y,-z\n"\
"symop -x,y,-z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  20\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C 2c 2 (z,x,y)'\n"\
"symbol xHM  'A 21 2 2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 2' '222'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,-z\n"\
"symop -x,y,-z\n"\
"symop -x+1/2,-y,z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  20\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C 2c 2 (y,z,x)'\n"\
"symbol xHM  'B 2 21 2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 2' '222'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,-z\n"\
"symop -x,-y,z\n"\
"symop x,-y+1/2,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  21\n"\
"basisop x,y,z\n"\
"symbol ccp4 21\n"\
"symbol Hall ' C 2 2'\n"\
"symbol xHM  'C 2 2 2'\n"\
"symbol old  'C 2 2 2'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 2' '222'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z\n"\
"symop -x,y,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  21\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C 2 2 (z,x,y)'\n"\
"symbol xHM  'A 2 2 2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 2' '222'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop -x,y,-z\n"\
"symop -x,-y,z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  21\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C 2 2 (y,z,x)'\n"\
"symbol xHM  'B 2 2 2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 2' '222'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  22\n"\
"basisop x,y,z\n"\
"symbol ccp4 22\n"\
"symbol Hall ' F 2 2'\n"\
"symbol xHM  'F 2 2 2'\n"\
"symbol old  'F 2 2 2'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-F 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 2' '222'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z\n"\
"symop -x,y,-z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"cenop x+1/2,y,z+1/2\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  23\n"\
"basisop x,y,z\n"\
"symbol ccp4 23\n"\
"symbol Hall ' I 2 2'\n"\
"symbol xHM  'I 2 2 2'\n"\
"symbol old  'I 2 2 2'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 2' '222'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z\n"\
"symop -x,y,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  24\n"\
"basisop x,y,z\n"\
"symbol ccp4 24\n"\
"symbol Hall ' I 2b 2c'\n"\
"symbol xHM  'I 21 21 21'\n"\
"symbol old  'I 21 21 21'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 2' '222'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y+1/2,z\n"\
"symop x,-y,-z+1/2\n"\
"symop -x,y+1/2,-z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  25\n"\
"basisop x,y,z\n"\
"symbol ccp4 25\n"\
"symbol Hall ' P 2 -2'\n"\
"symbol xHM  'P m m 2'\n"\
"symbol old  'P m m 2'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop -x,y,z\n"\
"symop x,-y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  25\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2 -2 (z,x,y)'\n"\
"symbol xHM  'P 2 m m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop x,-y,z\n"\
"symop x,y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  25\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2 -2 (y,z,x)'\n"\
"symbol xHM  'P m 2 m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop x,y,-z\n"\
"symop -x,y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  26\n"\
"basisop x,y,z\n"\
"symbol ccp4 26\n"\
"symbol Hall ' P 2c -2'\n"\
"symbol xHM  'P m c 21'\n"\
"symbol old  'P m c 21'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z+1/2\n"\
"symop -x,y,z\n"\
"symop x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  26\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2c -2 (y,-x,z)'\n"\
"symbol xHM  'P c m 21'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z+1/2\n"\
"symop x,-y,z\n"\
"symop -x,y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  26\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2c -2 (z,x,y)'\n"\
"symbol xHM  'P 21 m a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,-z\n"\
"symop x,-y,z\n"\
"symop x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  26\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2c -2 (z,y,-x)'\n"\
"symbol xHM  'P 21 a m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,-z\n"\
"symop x,y,-z\n"\
"symop x+1/2,-y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  26\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2c -2 (y,z,x)'\n"\
"symbol xHM  'P b 21 m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1; 0<=y<1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,-z\n"\
"symop x,y,-z\n"\
"symop -x,y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  26\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2c -2 (-x,z,y)'\n"\
"symbol xHM  'P m 21 b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,-z\n"\
"symop -x,y,z\n"\
"symop x,y+1/2,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  27\n"\
"basisop x,y,z\n"\
"symbol ccp4 27\n"\
"symbol Hall ' P 2 -2c'\n"\
"symbol xHM  'P c c 2'\n"\
"symbol old  'P c c 2'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop -x,y,z+1/2\n"\
"symop x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  27\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2 -2c (z,x,y)'\n"\
"symbol xHM  'P 2 a a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop x+1/2,-y,z\n"\
"symop x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  27\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2 -2c (y,z,x)'\n"\
"symbol xHM  'P b 2 b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop x,y+1/2,-z\n"\
"symop -x,y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  28\n"\
"basisop x,y,z\n"\
"symbol ccp4 28\n"\
"symbol Hall ' P 2 -2a'\n"\
"symbol xHM  'P m a 2'\n"\
"symbol old  'P m a 2'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop -x+1/2,y,z\n"\
"symop x+1/2,-y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  28\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2 -2a (y,-x,z)'\n"\
"symbol xHM  'P b m 2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y+1/2,z\n"\
"symop -x,y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  28\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2 -2a (z,x,y)'\n"\
"symbol xHM  'P 2 m b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop x,-y+1/2,z\n"\
"symop x,y+1/2,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  28\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2 -2a (z,y,-x)'\n"\
"symbol xHM  'P 2 c m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/2; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop x,y,-z+1/2\n"\
"symop x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  28\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2 -2a (y,z,x)'\n"\
"symbol xHM  'P c 2 m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop x,y,-z+1/2\n"\
"symop -x,y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  28\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2 -2a (-x,z,y)'\n"\
"symbol xHM  'P m 2 a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop -x+1/2,y,z\n"\
"symop x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  29\n"\
"basisop x,y,z\n"\
"symbol ccp4 29\n"\
"symbol Hall ' P 2c -2ac'\n"\
"symbol xHM  'P c a 21'\n"\
"symbol old  'P c a 21'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z+1/2\n"\
"symop -x+1/2,y,z+1/2\n"\
"symop x+1/2,-y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  29\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2c -2ac (y,-x,z)'\n"\
"symbol xHM  'P b c 21'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z+1/2\n"\
"symop x,-y+1/2,z+1/2\n"\
"symop -x,y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  29\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2c -2ac (z,x,y)'\n"\
"symbol xHM  'P 21 a b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,-z\n"\
"symop x+1/2,-y+1/2,z\n"\
"symop x,y+1/2,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  29\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2c -2ac (z,y,-x)'\n"\
"symbol xHM  'P 21 c a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,-z\n"\
"symop x+1/2,y,-z+1/2\n"\
"symop x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  29\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2c -2ac (y,z,x)'\n"\
"symbol xHM  'P c 21 b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,-z\n"\
"symop x,y+1/2,-z+1/2\n"\
"symop -x,y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  29\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2c -2ac (-x,z,y)'\n"\
"symbol xHM  'P b 21 a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,-z\n"\
"symop -x+1/2,y+1/2,z\n"\
"symop x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  30\n"\
"basisop x,y,z\n"\
"symbol ccp4 30\n"\
"symbol Hall ' P 2 -2bc'\n"\
"symbol xHM  'P n c 2'\n"\
"symbol old  'P n c 2'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop -x,y+1/2,z+1/2\n"\
"symop x,-y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  30\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2 -2bc (y,-x,z)'\n"\
"symbol xHM  'P c n 2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x+1/2,-y,z+1/2\n"\
"symop -x+1/2,y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  30\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2 -2bc (z,x,y)'\n"\
"symbol xHM  'P 2 n a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop x+1/2,-y,z+1/2\n"\
"symop x+1/2,y,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  30\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2 -2bc (z,y,-x)'\n"\
"symbol xHM  'P 2 a n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop x+1/2,-y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  30\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2 -2bc (y,z,x)'\n"\
"symbol xHM  'P b 2 n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -x+1/2,y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  30\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2 -2bc (-x,z,y)'\n"\
"symbol xHM  'P n 2 b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop -x,y+1/2,z+1/2\n"\
"symop x,y+1/2,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  31\n"\
"basisop x,y,z\n"\
"symbol ccp4 31\n"\
"symbol Hall ' P 2ac -2'\n"\
"symbol xHM  'P m n 21'\n"\
"symbol old  'P m n 21'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop -x,y,z\n"\
"symop x+1/2,-y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  31\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2ac -2 (y,-x,z)'\n"\
"symbol xHM  'P n m 21'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y+1/2,z+1/2\n"\
"symop x,-y,z\n"\
"symop -x,y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  31\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2ac -2 (z,x,y)'\n"\
"symbol xHM  'P 21 m n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop x,-y,z\n"\
"symop x+1/2,y+1/2,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  31\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2ac -2 (z,y,-x)'\n"\
"symbol xHM  'P 21 n m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,-z+1/2\n"\
"symop x,y,-z\n"\
"symop x+1/2,-y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  31\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2ac -2 (y,z,x)'\n"\
"symbol xHM  'P n 21 m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1; 0<=y<1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop x,y,-z\n"\
"symop -x,y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  31\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2ac -2 (-x,z,y)'\n"\
"symbol xHM  'P m 21 n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x+1/2,y+1/2,-z\n"\
"symop -x,y,z\n"\
"symop x+1/2,y+1/2,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  32\n"\
"basisop x,y,z\n"\
"symbol ccp4 32\n"\
"symbol Hall ' P 2 -2ab'\n"\
"symbol xHM  'P b a 2'\n"\
"symbol old  'P b a 2'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop -x+1/2,y+1/2,z\n"\
"symop x+1/2,-y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  32\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2 -2ab (z,x,y)'\n"\
"symbol xHM  'P 2 c b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop x,-y+1/2,z+1/2\n"\
"symop x,y+1/2,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  32\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2 -2ab (y,z,x)'\n"\
"symbol xHM  'P c 2 a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop x+1/2,y,-z+1/2\n"\
"symop -x+1/2,y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  33\n"\
"basisop x,y,z\n"\
"symbol ccp4 33\n"\
"symbol Hall ' P 2c -2n'\n"\
"symbol xHM  'P n a 21'\n"\
"symbol old  'P n a 21'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z+1/2\n"\
"symop -x+1/2,y+1/2,z+1/2\n"\
"symop x+1/2,-y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  33\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2c -2n (y,-x,z)'\n"\
"symbol xHM  'P b n 21'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z+1/2\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"symop -x+1/2,y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  33\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2c -2n (z,x,y)'\n"\
"symbol xHM  'P 21 n b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,-z\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"symop x,y+1/2,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  33\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2c -2n (z,y,-x)'\n"\
"symbol xHM  'P 21 c n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,-z\n"\
"symop x+1/2,y+1/2,-z+1/2\n"\
"symop x,-y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  33\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2c -2n (y,z,x)'\n"\
"symbol xHM  'P c 21 n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1/2; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,-z\n"\
"symop x+1/2,y+1/2,-z+1/2\n"\
"symop -x+1/2,y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  33\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2c -2n (-x,z,y)'\n"\
"symbol xHM  'P n 21 a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,-z\n"\
"symop -x+1/2,y+1/2,z+1/2\n"\
"symop x+1/2,y,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  34\n"\
"basisop x,y,z\n"\
"symbol ccp4 34\n"\
"symbol Hall ' P 2 -2n'\n"\
"symbol xHM  'P n n 2'\n"\
"symbol old  'P n n 2'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop -x+1/2,y+1/2,z+1/2\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  34\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2 -2n (z,x,y)'\n"\
"symbol xHM  'P 2 n n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"symop x+1/2,y+1/2,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  34\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' P 2 -2n (y,z,x)'\n"\
"symbol xHM  'P n 2 n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop x+1/2,y+1/2,-z+1/2\n"\
"symop -x+1/2,y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  35\n"\
"basisop x,y,z\n"\
"symbol ccp4 35\n"\
"symbol Hall ' C 2 -2'\n"\
"symbol xHM  'C m m 2'\n"\
"symbol old  'C m m 2'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop -x,y,z\n"\
"symop x,-y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  35\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C 2 -2 (z,x,y)'\n"\
"symbol xHM  'A 2 m m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop x,-y,z\n"\
"symop x,y,-z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  35\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C 2 -2 (y,z,x)'\n"\
"symbol xHM  'B m 2 m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop x,y,-z\n"\
"symop -x,y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  36\n"\
"basisop x,y,z\n"\
"symbol ccp4 36\n"\
"symbol Hall ' C 2c -2'\n"\
"symbol xHM  'C m c 21'\n"\
"symbol old  'C m c 21'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z+1/2\n"\
"symop -x,y,z\n"\
"symop x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  36\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C 2c -2 (y,-x,z)'\n"\
"symbol xHM  'C c m 21'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z+1/2\n"\
"symop x,-y,z\n"\
"symop -x,y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  36\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C 2c -2 (z,x,y)'\n"\
"symbol xHM  'A 21 m a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,-z\n"\
"symop x,-y,z\n"\
"symop x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  36\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C 2c -2 (z,y,-x)'\n"\
"symbol xHM  'A 21 a m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,-z\n"\
"symop x,y,-z\n"\
"symop x+1/2,-y,z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  36\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C 2c -2 (y,z,x)'\n"\
"symbol xHM  'B b 21 m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1/2; 0<=y<1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,-z\n"\
"symop x,y,-z\n"\
"symop -x,y+1/2,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  36\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C 2c -2 (-x,z,y)'\n"\
"symbol xHM  'B m 21 b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,-z\n"\
"symop -x,y,z\n"\
"symop x,y+1/2,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  37\n"\
"basisop x,y,z\n"\
"symbol ccp4 37\n"\
"symbol Hall ' C 2 -2c'\n"\
"symbol xHM  'C c c 2'\n"\
"symbol old  'C c c 2'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop -x,y,z+1/2\n"\
"symop x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  37\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C 2 -2c (z,x,y)'\n"\
"symbol xHM  'A 2 a a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop x+1/2,-y,z\n"\
"symop x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  37\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' C 2 -2c (y,z,x)'\n"\
"symbol xHM  'B b 2 b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop x,y+1/2,-z\n"\
"symop -x,y+1/2,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  38\n"\
"basisop x,y,z\n"\
"symbol ccp4 38\n"\
"symbol Hall ' A 2 -2'\n"\
"symbol xHM  'A m m 2'\n"\
"symbol old  'A m m 2'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop -x,y,z\n"\
"symop x,-y,z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  38\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' A 2 -2 (y,-x,z)'\n"\
"symbol xHM  'B m m 2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y,z\n"\
"symop -x,y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  38\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' A 2 -2 (z,x,y)'\n"\
"symbol xHM  'B 2 m m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop x,-y,z\n"\
"symop x,y,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  38\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' A 2 -2 (z,y,-x)'\n"\
"symbol xHM  'C 2 m m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop x,y,-z\n"\
"symop x,-y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  38\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' A 2 -2 (y,z,x)'\n"\
"symbol xHM  'C m 2 m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop x,y,-z\n"\
"symop -x,y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  38\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' A 2 -2 (-x,z,y)'\n"\
"symbol xHM  'A m 2 m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop -x,y,z\n"\
"symop x,y,-z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  39\n"\
"basisop x,y,z\n"\
"symbol ccp4 39\n"\
"symbol Hall ' A 2 -2b'\n"\
"symbol xHM  'A b m 2'\n"\
"symbol old  'A b m 2'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop -x,y+1/2,z\n"\
"symop x,-y+1/2,z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  39\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' A 2 -2b (y,-x,z)'\n"\
"symbol xHM  'B m a 2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x+1/2,-y,z\n"\
"symop -x+1/2,y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  39\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' A 2 -2b (z,x,y)'\n"\
"symbol xHM  'B 2 c m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/2; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop x,-y,z+1/2\n"\
"symop x,y,-z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  39\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' A 2 -2b (z,y,-x)'\n"\
"symbol xHM  'C 2 m b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop x,y+1/2,-z\n"\
"symop x,-y+1/2,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  39\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' A 2 -2b (y,z,x)'\n"\
"symbol xHM  'C m 2 a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop x+1/2,y,-z\n"\
"symop -x+1/2,y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  39\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' A 2 -2b (-x,z,y)'\n"\
"symbol xHM  'A c 2 m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1/2; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1/2; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop -x,y,z+1/2\n"\
"symop x,y,-z+1/2\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  40\n"\
"basisop x,y,z\n"\
"symbol ccp4 40\n"\
"symbol Hall ' A 2 -2a'\n"\
"symbol xHM  'A m a 2'\n"\
"symbol old  'A m a 2'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop -x+1/2,y,z\n"\
"symop x+1/2,-y,z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  40\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' A 2 -2a (y,-x,z)'\n"\
"symbol xHM  'B b m 2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y+1/2,z\n"\
"symop -x,y+1/2,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  40\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' A 2 -2a (z,x,y)'\n"\
"symbol xHM  'B 2 m b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop x,-y+1/2,z\n"\
"symop x,y+1/2,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  40\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' A 2 -2a (z,y,-x)'\n"\
"symbol xHM  'C 2 c m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1/2; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop x,y,-z+1/2\n"\
"symop x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  40\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' A 2 -2a (y,z,x)'\n"\
"symbol xHM  'C c 2 m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1/2; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1/2; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop x,y,-z+1/2\n"\
"symop -x,y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  40\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' A 2 -2a (-x,z,y)'\n"\
"symbol xHM  'A m 2 a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop -x+1/2,y,z\n"\
"symop x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  41\n"\
"basisop x,y,z\n"\
"symbol ccp4 41\n"\
"symbol Hall ' A 2 -2ab'\n"\
"symbol xHM  'A b a 2'\n"\
"symbol old  'A b a 2'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop -x+1/2,y+1/2,z\n"\
"symop x+1/2,-y+1/2,z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  41\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' A 2 -2ab (y,-x,z)'\n"\
"symbol xHM  'B b a 2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x+1/2,-y+1/2,z\n"\
"symop -x+1/2,y+1/2,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  41\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' A 2 -2ab (z,x,y)'\n"\
"symbol xHM  'B 2 c b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop x,-y+1/2,z+1/2\n"\
"symop x,y+1/2,-z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  41\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' A 2 -2ab (z,y,-x)'\n"\
"symbol xHM  'C 2 c b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop x,y+1/2,-z+1/2\n"\
"symop x,-y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  41\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' A 2 -2ab (y,z,x)'\n"\
"symbol xHM  'C c 2 a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop x+1/2,y,-z+1/2\n"\
"symop -x+1/2,y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  41\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' A 2 -2ab (-x,z,y)'\n"\
"symbol xHM  'A c 2 a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop -x+1/2,y,z+1/2\n"\
"symop x+1/2,y,-z+1/2\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  42\n"\
"basisop x,y,z\n"\
"symbol ccp4 42\n"\
"symbol Hall ' F 2 -2'\n"\
"symbol xHM  'F m m 2'\n"\
"symbol old  'F m m 2'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-F 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop -x,y,z\n"\
"symop x,-y,z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"cenop x+1/2,y,z+1/2\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  42\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' F 2 -2 (z,x,y)'\n"\
"symbol xHM  'F 2 m m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-F 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop x,-y,z\n"\
"symop x,y,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"cenop x+1/2,y+1/2,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  42\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' F 2 -2 (y,z,x)'\n"\
"symbol xHM  'F m 2 m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-F 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop x,y,-z\n"\
"symop -x,y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  43\n"\
"basisop x,y,z\n"\
"symbol ccp4 43\n"\
"symbol Hall ' F 2 -2d'\n"\
"symbol xHM  'F d d 2'\n"\
"symbol old  'F d d 2'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-F 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/8; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/8; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop -x+1/4,y+1/4,z+1/4\n"\
"symop x+3/4,-y+3/4,z+1/4\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"cenop x+1/2,y,z+1/2\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  43\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' F 2 -2d (z,x,y)'\n"\
"symbol xHM  'F 2 d d'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-F 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/8; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/8; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop x+1/4,-y+1/4,z+1/4\n"\
"symop x+1/4,y+3/4,-z+3/4\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"cenop x+1/2,y+1/2,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  43\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' F 2 -2d (y,z,x)'\n"\
"symbol xHM  'F d 2 d'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-F 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop x+1/4,y+1/4,-z+1/4\n"\
"symop -x+3/4,y+1/4,z+3/4\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  44\n"\
"basisop x,y,z\n"\
"symbol ccp4 44\n"\
"symbol Hall ' I 2 -2'\n"\
"symbol xHM  'I m m 2'\n"\
"symbol old  'I m m 2'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop -x,y,z\n"\
"symop x,-y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  44\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' I 2 -2 (z,x,y)'\n"\
"symbol xHM  'I 2 m m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop x,-y,z\n"\
"symop x,y,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  44\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' I 2 -2 (y,z,x)'\n"\
"symbol xHM  'I m 2 m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop x,y,-z\n"\
"symop -x,y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  45\n"\
"basisop x,y,z\n"\
"symbol ccp4 45\n"\
"symbol Hall ' I 2 -2c'\n"\
"symbol xHM  'I b a 2'\n"\
"symbol old  'I b a 2'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop -x,y,z+1/2\n"\
"symop x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  45\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' I 2 -2c (z,x,y)'\n"\
"symbol xHM  'I 2 c b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop x+1/2,-y,z\n"\
"symop x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  45\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' I 2 -2c (y,z,x)'\n"\
"symbol xHM  'I c 2 a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop x,y+1/2,-z\n"\
"symop -x,y+1/2,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  46\n"\
"basisop x,y,z\n"\
"symbol ccp4 46\n"\
"symbol Hall ' I 2 -2a'\n"\
"symbol xHM  'I m a 2'\n"\
"symbol old  'I m a 2'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<=3/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop -x+1/2,y,z\n"\
"symop x+1/2,-y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  46\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall ' I 2 -2a (y,-x,z)'\n"\
"symbol xHM  'I b m 2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<=3/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y+1/2,z\n"\
"symop -x,y+1/2,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  46\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' I 2 -2a (z,x,y)'\n"\
"symbol xHM  'I 2 m b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop x,-y+1/2,z\n"\
"symop x,y+1/2,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  46\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' I 2 -2a (z,y,-x)'\n"\
"symbol xHM  'I 2 c m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<1; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop x,y,-z+1/2\n"\
"symop x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  46\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall ' I 2 -2a (y,z,x)'\n"\
"symbol xHM  'I c 2 m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1/2; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1/2; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop x,y,-z+1/2\n"\
"symop -x,y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  46\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall ' I 2 -2a (-x,z,y)'\n"\
"symbol xHM  'I m 2 a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp ' P -2 -2' 'mm2'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop -x+1/2,y,z\n"\
"symop x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  47\n"\
"basisop x,y,z\n"\
"symbol ccp4 47\n"\
"symbol Hall '-P 2 2'\n"\
"symbol xHM  'P m m m'\n"\
"symbol old  'P 2/m 2/m 2/m' 'P m m m'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z\n"\
"symop -x,y,-z\n"\
"symop -x,-y,-z\n"\
"symop x,y,-z\n"\
"symop -x,y,z\n"\
"symop x,-y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  48\n"\
"basisop x-1/4,y-1/4,z-1/4\n"\
"symbol ccp4 48\n"\
"symbol Hall '-P 2ab 2bc (x-1/4,y-1/4,z-1/4)'\n"\
"symbol xHM  'P n n n :1'\n"\
"symbol old  'P 2/n 2/n 2/n'  'P n n n'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z\n"\
"symop -x,y,-z\n"\
"symop -x+1/2,-y+1/2,-z+1/2\n"\
"symop x+1/2,y+1/2,-z+1/2\n"\
"symop -x+1/2,y+1/2,z+1/2\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  48\n"\
"basisop x,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ab 2bc'\n"\
"symbol xHM  'P n n n :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<=3/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -x,y+1/2,z+1/2\n"\
"symop x+1/2,-y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  49\n"\
"basisop x,y,z\n"\
"symbol ccp4 49\n"\
"symbol Hall '-P 2 2c'\n"\
"symbol xHM  'P c c m'\n"\
"symbol old  'P 2/c 2/c 2/m' 'P c c m'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z+1/2\n"\
"symop -x,y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,y,-z\n"\
"symop -x,y,z+1/2\n"\
"symop x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  49\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2 2c (z,x,y)'\n"\
"symbol xHM  'P m a a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop -x+1/2,y,-z\n"\
"symop -x+1/2,-y,z\n"\
"symop -x,-y,-z\n"\
"symop -x,y,z\n"\
"symop x+1/2,-y,z\n"\
"symop x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  49\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2 2c (y,z,x)'\n"\
"symbol xHM  'P b m b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop -x,-y+1/2,z\n"\
"symop x,-y+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop x,-y,z\n"\
"symop x,y+1/2,-z\n"\
"symop -x,y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  50\n"\
"basisop x-1/4,y-1/4,z\n"\
"symbol ccp4 50\n"\
"symbol Hall '-P 2ab 2b (x-1/4,y-1/4,z)'\n"\
"symbol xHM  'P b a n :1'\n"\
"symbol old  'P 2/b 2/a 2/n' 'P b a n'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z\n"\
"symop -x,y,-z\n"\
"symop -x+1/2,-y+1/2,-z\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -x+1/2,y+1/2,z\n"\
"symop x+1/2,-y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  50\n"\
"basisop x,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ab 2b'\n"\
"symbol xHM  'P b a n :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<=3/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop x,-y+1/2,-z\n"\
"symop -x+1/2,y,-z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -x,y+1/2,z\n"\
"symop x+1/2,-y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  50\n"\
"basisop z,x-1/4,y-1/4\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ab 2b (z,x-1/4,y-1/4)'\n"\
"symbol xHM  'P n c b :1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop -x,y,-z\n"\
"symop -x,-y,z\n"\
"symop -x,-y+1/2,-z+1/2\n"\
"symop -x,y+1/2,z+1/2\n"\
"symop x,-y+1/2,z+1/2\n"\
"symop x,y+1/2,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  50\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ab 2b (z,x,y)'\n"\
"symbol xHM  'P n c b :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop -x,y,-z+1/2\n"\
"symop -x,-y+1/2,z\n"\
"symop -x,-y,-z\n"\
"symop -x,y+1/2,z+1/2\n"\
"symop x,-y,z+1/2\n"\
"symop x,y+1/2,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  50\n"\
"basisop y-1/4,z,x-1/4\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ab 2b (y-1/4,z,x-1/4)'\n"\
"symbol xHM  'P c n a :1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z\n"\
"symop -x+1/2,-y,-z+1/2\n"\
"symop x+1/2,-y,z+1/2\n"\
"symop x+1/2,y,-z+1/2\n"\
"symop -x+1/2,y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  50\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ab 2b (y,z,x)'\n"\
"symbol xHM  'P c n a :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop -x+1/2,-y,z\n"\
"symop x,-y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,-y,z+1/2\n"\
"symop x+1/2,y,-z\n"\
"symop -x,y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  51\n"\
"basisop x,y,z\n"\
"symbol ccp4 51\n"\
"symbol Hall '-P 2a 2a'\n"\
"symbol xHM  'P m m a'\n"\
"symbol old  'P 21/m 2/m 2/a'  'P m m a'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y,z\n"\
"symop x+1/2,-y,-z\n"\
"symop -x,y,-z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y,-z\n"\
"symop -x+1/2,y,z\n"\
"symop x,-y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  51\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2a 2a (y,-x,z)'\n"\
"symbol xHM  'P m m b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y+1/2,z\n"\
"symop -x,y+1/2,-z\n"\
"symop x,-y,-z\n"\
"symop -x,-y,-z\n"\
"symop x,y+1/2,-z\n"\
"symop x,-y+1/2,z\n"\
"symop -x,y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  51\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2a 2a (z,x,y)'\n"\
"symbol xHM  'P b m m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y+1/2,-z\n"\
"symop -x,y+1/2,-z\n"\
"symop -x,-y,z\n"\
"symop -x,-y,-z\n"\
"symop -x,y+1/2,z\n"\
"symop x,-y+1/2,z\n"\
"symop x,y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  51\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2a 2a (z,y,-x)'\n"\
"symbol xHM  'P c m m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z+1/2\n"\
"symop -x,-y,z+1/2\n"\
"symop -x,y,-z\n"\
"symop -x,-y,-z\n"\
"symop -x,y,z+1/2\n"\
"symop x,y,-z+1/2\n"\
"symop x,-y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  51\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2a 2a (y,z,x)'\n"\
"symbol xHM  'P m c m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z+1/2\n"\
"symop -x,-y,z+1/2\n"\
"symop x,-y,-z\n"\
"symop -x,-y,-z\n"\
"symop x,-y,z+1/2\n"\
"symop x,y,-z+1/2\n"\
"symop -x,y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  51\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2a 2a (-x,z,y)'\n"\
"symbol xHM  'P m a m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,y,-z\n"\
"symop x+1/2,-y,-z\n"\
"symop -x,-y,z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,-y,z\n"\
"symop -x+1/2,y,z\n"\
"symop x,y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  52\n"\
"basisop x,y,z\n"\
"symbol ccp4 52\n"\
"symbol Hall '-P 2a 2bc'\n"\
"symbol xHM  'P n n a'\n"\
"symbol old  'P 2/n 21/n 2/a'  'P n n a'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y,z\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop -x+1/2,y+1/2,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y,-z\n"\
"symop -x,y+1/2,z+1/2\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  52\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2a 2bc (y,-x,z)'\n"\
"symbol xHM  'P n n b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y+1/2,z\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop x+1/2,-y+1/2,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,y+1/2,-z\n"\
"symop x+1/2,-y,z+1/2\n"\
"symop -x+1/2,y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  52\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2a 2bc (z,x,y)'\n"\
"symbol xHM  'P b n n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1/2; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<=3/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y+1/2,-z\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop -x+1/2,-y+1/2,z+1/2\n"\
"symop -x,-y,-z\n"\
"symop -x,y+1/2,z\n"\
"symop x+1/2,-y,z+1/2\n"\
"symop x+1/2,y+1/2,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  52\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2a 2bc (z,y,-x)'\n"\
"symbol xHM  'P c n n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z+1/2\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop -x+1/2,y+1/2,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop -x,y,z+1/2\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  52\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2a 2bc (y,z,x)'\n"\
"symbol xHM  'P n c n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z+1/2\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop x+1/2,-y+1/2,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,-y,z+1/2\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -x+1/2,y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  52\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2a 2bc (-x,z,y)'\n"\
"symbol xHM  'P n a n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<=3/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,y,-z\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop -x+1/2,-y+1/2,z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,-y,z\n"\
"symop -x,y+1/2,z+1/2\n"\
"symop x+1/2,y+1/2,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  53\n"\
"basisop x,y,z\n"\
"symbol ccp4 53\n"\
"symbol Hall '-P 2ac 2'\n"\
"symbol xHM  'P m n a'\n"\
"symbol old  'P 2/m 2/n 21/a' 'P m n a'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop x,-y,-z\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y,-z+1/2\n"\
"symop -x,y,z\n"\
"symop x+1/2,-y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  53\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ac 2 (y,-x,z)'\n"\
"symbol xHM  'P n m b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y+1/2,z+1/2\n"\
"symop -x,y,-z\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,y+1/2,-z+1/2\n"\
"symop x,-y,z\n"\
"symop -x,y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  53\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ac 2 (z,x,y)'\n"\
"symbol xHM  'P b m n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop -x,y,-z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop -x,-y,-z\n"\
"symop -x+1/2,y+1/2,z\n"\
"symop x,-y,z\n"\
"symop x+1/2,y+1/2,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  53\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ac 2 (z,y,-x)'\n"\
"symbol xHM  'P c n m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,-z+1/2\n"\
"symop -x,-y,z\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop -x+1/2,y,z+1/2\n"\
"symop x,y,-z\n"\
"symop x+1/2,-y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  53\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ac 2 (y,z,x)'\n"\
"symbol xHM  'P n c m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop -x,-y,z\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,-y+1/2,z+1/2\n"\
"symop x,y,-z\n"\
"symop -x,y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  53\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ac 2 (-x,z,y)'\n"\
"symbol xHM  'P m a n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,y+1/2,-z\n"\
"symop x,-y,-z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,-y+1/2,z\n"\
"symop -x,y,z\n"\
"symop x+1/2,y+1/2,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  54\n"\
"basisop x,y,z\n"\
"symbol ccp4 54\n"\
"symbol Hall '-P 2a 2ac'\n"\
"symbol xHM  'P c c a'\n"\
"symbol old  'P 21/c 2/c 2/a' 'P c c a'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y,z\n"\
"symop x+1/2,-y,-z+1/2\n"\
"symop -x,y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y,-z\n"\
"symop -x+1/2,y,z+1/2\n"\
"symop x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  54\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2a 2ac (y,-x,z)'\n"\
"symbol xHM  'P c c b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y+1/2,z\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop x,-y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,y+1/2,-z\n"\
"symop x,-y+1/2,z+1/2\n"\
"symop -x,y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  54\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2a 2ac (z,x,y)'\n"\
"symbol xHM  'P b a a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y+1/2,-z\n"\
"symop -x+1/2,y+1/2,-z\n"\
"symop -x+1/2,-y,z\n"\
"symop -x,-y,-z\n"\
"symop -x,y+1/2,z\n"\
"symop x+1/2,-y+1/2,z\n"\
"symop x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  54\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2a 2ac (z,y,-x)'\n"\
"symbol xHM  'P c a a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z+1/2\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop -x+1/2,y,-z\n"\
"symop -x,-y,-z\n"\
"symop -x,y,z+1/2\n"\
"symop x+1/2,y,-z+1/2\n"\
"symop x+1/2,-y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  54\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2a 2ac (y,z,x)'\n"\
"symbol xHM  'P b c b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z+1/2\n"\
"symop -x,-y+1/2,z+1/2\n"\
"symop x,-y+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop x,-y,z+1/2\n"\
"symop x,y+1/2,-z+1/2\n"\
"symop -x,y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  54\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2a 2ac (-x,z,y)'\n"\
"symbol xHM  'P b a b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,y,-z\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop -x,-y+1/2,z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,-y,z\n"\
"symop -x+1/2,y+1/2,z\n"\
"symop x,y+1/2,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  55\n"\
"basisop x,y,z\n"\
"symbol ccp4 55\n"\
"symbol Hall '-P 2 2ab'\n"\
"symbol xHM  'P b a m'\n"\
"symbol old  'P 21/b 21/a 2/m' 'P b a m'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop -x+1/2,y+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop x,y,-z\n"\
"symop -x+1/2,y+1/2,z\n"\
"symop x+1/2,-y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  55\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2 2ab (z,x,y)'\n"\
"symbol xHM  'P m c b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop -x,-y+1/2,z+1/2\n"\
"symop -x,-y,-z\n"\
"symop -x,y,z\n"\
"symop x,-y+1/2,z+1/2\n"\
"symop x,y+1/2,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  55\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2 2ab (y,z,x)'\n"\
"symbol xHM  'P c m a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop x+1/2,-y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,-y,z\n"\
"symop x+1/2,y,-z+1/2\n"\
"symop -x+1/2,y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  56\n"\
"basisop x,y,z\n"\
"symbol ccp4 56\n"\
"symbol Hall '-P 2ab 2ac'\n"\
"symbol xHM  'P c c n'\n"\
"symbol old  'P 21/c 21/c 2/n' 'P c c n'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<=3/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop x+1/2,-y,-z+1/2\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -x+1/2,y,z+1/2\n"\
"symop x,-y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  56\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ab 2ac (z,x,y)'\n"\
"symbol xHM  'P n a a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop -x+1/2,y+1/2,-z\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop -x,-y,-z\n"\
"symop -x,y+1/2,z+1/2\n"\
"symop x+1/2,-y+1/2,z\n"\
"symop x+1/2,y,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  56\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ab 2ac (y,z,x)'\n"\
"symbol xHM  'P b n b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop -x,-y+1/2,z+1/2\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,-y,z+1/2\n"\
"symop x,y+1/2,-z+1/2\n"\
"symop -x+1/2,y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  57\n"\
"basisop x,y,z\n"\
"symbol ccp4 57\n"\
"symbol Hall '-P 2c 2b'\n"\
"symbol xHM  'P b c m'\n"\
"symbol old  'P 2/b 21/c 21/m' 'P b c m'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1/2; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z+1/2\n"\
"symop x,-y+1/2,-z\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,y,-z+1/2\n"\
"symop -x,y+1/2,z\n"\
"symop x,-y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  57\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2c 2b (y,-x,z)'\n"\
"symbol xHM  'P c a m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z+1/2\n"\
"symop -x+1/2,y,-z\n"\
"symop x+1/2,-y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,y,-z+1/2\n"\
"symop x+1/2,-y,z\n"\
"symop -x+1/2,y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  57\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2c 2b (z,x,y)'\n"\
"symbol xHM  'P m c a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,-z\n"\
"symop -x,y,-z+1/2\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop -x,-y,-z\n"\
"symop -x+1/2,y,z\n"\
"symop x,-y,z+1/2\n"\
"symop x+1/2,y,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  57\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2c 2b (z,y,-x)'\n"\
"symbol xHM  'P m a b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,-z\n"\
"symop -x,-y+1/2,z\n"\
"symop -x+1/2,y+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop -x+1/2,y,z\n"\
"symop x,y+1/2,-z\n"\
"symop x+1/2,-y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  57\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2c 2b (y,z,x)'\n"\
"symbol xHM  'P b m a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,-z\n"\
"symop -x+1/2,-y,z\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop x,-y+1/2,z\n"\
"symop x+1/2,y,-z\n"\
"symop -x+1/2,y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  57\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2c 2b (-x,z,y)'\n"\
"symbol xHM  'P c m b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,-z\n"\
"symop x,-y,-z+1/2\n"\
"symop -x,-y+1/2,z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,-y+1/2,z\n"\
"symop -x,y,z+1/2\n"\
"symop x,y+1/2,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  58\n"\
"basisop x,y,z\n"\
"symbol ccp4 58\n"\
"symbol Hall '-P 2 2n'\n"\
"symbol xHM  'P n n m'\n"\
"symbol old  'P 21/n 21/n 2/m' 'P n n m'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x+1/2,-y+1/2,-z+1/2\n"\
"symop -x+1/2,y+1/2,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,y,-z\n"\
"symop -x+1/2,y+1/2,z+1/2\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  58\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2 2n (z,x,y)'\n"\
"symbol xHM  'P m n n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop -x+1/2,y+1/2,-z+1/2\n"\
"symop -x+1/2,-y+1/2,z+1/2\n"\
"symop -x,-y,-z\n"\
"symop -x,y,z\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"symop x+1/2,y+1/2,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  58\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2 2n (y,z,x)'\n"\
"symbol xHM  'P n m n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop -x+1/2,-y+1/2,z+1/2\n"\
"symop x+1/2,-y+1/2,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,-y,z\n"\
"symop x+1/2,y+1/2,-z+1/2\n"\
"symop -x+1/2,y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  59\n"\
"basisop x-1/4,y-1/4,z\n"\
"symbol ccp4 59\n"\
"symbol Hall '-P 2ab 2a (x-1/4,y-1/4,z)'\n"\
"symbol xHM  'P m m n :1'\n"\
"symbol old  'P 21/m 21/m 2/n' 'P m m n'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop -x+1/2,y+1/2,-z\n"\
"symop -x+1/2,-y+1/2,-z\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -x,y,z\n"\
"symop x,-y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  59\n"\
"basisop x,y,z\n"\
"symbol ccp4 1059\n"\
"symbol Hall '-P 2ab 2a'\n"\
"symbol xHM  'P m m n :2'\n"\
"symbol old  'P 21/m 21/m 2/n a'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<=3/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop x+1/2,-y,-z\n"\
"symop -x,y+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -x+1/2,y,z\n"\
"symop x,-y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  59\n"\
"basisop z,x-1/4,y-1/4\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ab 2a (z,x-1/4,y-1/4)'\n"\
"symbol xHM  'P n m m :1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop -x,-y+1/2,z+1/2\n"\
"symop -x,-y+1/2,-z+1/2\n"\
"symop -x,y+1/2,z+1/2\n"\
"symop x,-y,z\n"\
"symop x,y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  59\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ab 2a (z,x,y)'\n"\
"symbol xHM  'P n m m :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop -x,y+1/2,-z\n"\
"symop -x,-y,z+1/2\n"\
"symop -x,-y,-z\n"\
"symop -x,y+1/2,z+1/2\n"\
"symop x,-y+1/2,z\n"\
"symop x,y,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  59\n"\
"basisop y-1/4,z,x-1/4\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ab 2a (y-1/4,z,x-1/4)'\n"\
"symbol xHM  'P m n m :1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop x+1/2,-y,-z+1/2\n"\
"symop -x+1/2,-y,-z+1/2\n"\
"symop x+1/2,-y,z+1/2\n"\
"symop x,y,-z\n"\
"symop -x,y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  59\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ab 2a (y,z,x)'\n"\
"symbol xHM  'P m n m :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop -x,-y,z+1/2\n"\
"symop x+1/2,-y,-z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,-y,z+1/2\n"\
"symop x,y,-z+1/2\n"\
"symop -x+1/2,y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  60\n"\
"basisop x,y,z\n"\
"symbol ccp4 60\n"\
"symbol Hall '-P 2n 2ab'\n"\
"symbol xHM  'P b c n'\n"\
"symbol old  'P 21/b 2/c 21/n' 'P b c n'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y+1/2,z+1/2\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop -x,y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y+1/2,-z+1/2\n"\
"symop -x+1/2,y+1/2,z\n"\
"symop x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  60\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2n 2ab (y,-x,z)'\n"\
"symbol xHM  'P c a n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y+1/2,z+1/2\n"\
"symop -x+1/2,y+1/2,-z\n"\
"symop x,-y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y+1/2,-z+1/2\n"\
"symop x+1/2,-y+1/2,z\n"\
"symop -x,y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  60\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2n 2ab (z,x,y)'\n"\
"symbol xHM  'P n c a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x+1/2,-y+1/2,-z+1/2\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop -x+1/2,-y,z\n"\
"symop -x,-y,-z\n"\
"symop -x+1/2,y+1/2,z+1/2\n"\
"symop x,-y+1/2,z+1/2\n"\
"symop x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  60\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2n 2ab (z,y,-x)'\n"\
"symbol xHM  'P n a b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x+1/2,-y+1/2,-z+1/2\n"\
"symop -x,-y+1/2,z+1/2\n"\
"symop -x+1/2,y,-z\n"\
"symop -x,-y,-z\n"\
"symop -x+1/2,y+1/2,z+1/2\n"\
"symop x,y+1/2,-z+1/2\n"\
"symop x+1/2,-y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  60\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2n 2ab (y,z,x)'\n"\
"symbol xHM  'P b n a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,y+1/2,-z+1/2\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop x,-y+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"symop x+1/2,y,-z+1/2\n"\
"symop -x,y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  60\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2n 2ab (-x,z,y)'\n"\
"symbol xHM  'P c n b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,y+1/2,-z+1/2\n"\
"symop x+1/2,-y,-z+1/2\n"\
"symop -x,-y+1/2,z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"symop -x+1/2,y,z+1/2\n"\
"symop x,y+1/2,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  61\n"\
"basisop x,y,z\n"\
"symbol ccp4 61\n"\
"symbol Hall '-P 2ac 2ab'\n"\
"symbol xHM  'P b c a'\n"\
"symbol old  'P 21/b 21/c 21/a' 'P b c a'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y,-z+1/2\n"\
"symop -x+1/2,y+1/2,z\n"\
"symop x,-y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  61\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ac 2ab (z,y,-x)'\n"\
"symbol xHM  'P c a b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,-z+1/2\n"\
"symop -x,-y+1/2,z+1/2\n"\
"symop -x+1/2,y+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop -x+1/2,y,z+1/2\n"\
"symop x,y+1/2,-z+1/2\n"\
"symop x+1/2,-y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  62\n"\
"basisop x,y,z\n"\
"symbol ccp4 62\n"\
"symbol Hall '-P 2ac 2n'\n"\
"symbol xHM  'P n m a'\n"\
"symbol old  'P 21/n 21/m 21/a' 'P n m a'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop x+1/2,-y+1/2,-z+1/2\n"\
"symop -x,y+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y,-z+1/2\n"\
"symop -x+1/2,y+1/2,z+1/2\n"\
"symop x,-y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  62\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ac 2n (y,-x,z)'\n"\
"symbol xHM  'P m n b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y+1/2,z+1/2\n"\
"symop -x+1/2,y+1/2,-z+1/2\n"\
"symop x+1/2,-y,-z\n"\
"symop -x,-y,-z\n"\
"symop x,y+1/2,-z+1/2\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"symop -x+1/2,y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  62\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ac 2n (z,x,y)'\n"\
"symbol xHM  'P b n m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1/2; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop -x+1/2,y+1/2,-z+1/2\n"\
"symop -x,-y,z+1/2\n"\
"symop -x,-y,-z\n"\
"symop -x+1/2,y+1/2,z\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"symop x,y,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  62\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ac 2n (z,y,-x)'\n"\
"symbol xHM  'P c m n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<=3/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,-z+1/2\n"\
"symop -x+1/2,-y+1/2,z+1/2\n"\
"symop -x,y+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop -x+1/2,y,z+1/2\n"\
"symop x+1/2,y+1/2,-z+1/2\n"\
"symop x,-y+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  62\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ac 2n (y,z,x)'\n"\
"symbol xHM  'P m c n'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<=3/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop -x+1/2,-y+1/2,z+1/2\n"\
"symop x+1/2,-y,-z\n"\
"symop -x,-y,-z\n"\
"symop x,-y+1/2,z+1/2\n"\
"symop x+1/2,y+1/2,-z+1/2\n"\
"symop -x+1/2,y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  62\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ac 2n (-x,z,y)'\n"\
"symbol xHM  'P n a m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<1; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,y+1/2,-z\n"\
"symop x+1/2,-y+1/2,-z+1/2\n"\
"symop -x,-y,z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,-y+1/2,z\n"\
"symop -x+1/2,y+1/2,z+1/2\n"\
"symop x,y,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  63\n"\
"basisop x,y,z\n"\
"symbol ccp4 63\n"\
"symbol Hall '-C 2c 2'\n"\
"symbol xHM  'C m c m'\n"\
"symbol old  'C 2/m 2/c 21/m' 'C m c m'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1/2; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z+1/2\n"\
"symop x,-y,-z\n"\
"symop -x,y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,y,-z+1/2\n"\
"symop -x,y,z\n"\
"symop x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  63\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2c 2 (y,-x,z)'\n"\
"symbol xHM  'C c m m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z+1/2\n"\
"symop -x,y,-z\n"\
"symop x,-y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,y,-z+1/2\n"\
"symop x,-y,z\n"\
"symop -x,y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  63\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2c 2 (z,x,y)'\n"\
"symbol xHM  'A m m a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,-z\n"\
"symop -x,y,-z\n"\
"symop -x+1/2,-y,z\n"\
"symop -x,-y,-z\n"\
"symop -x+1/2,y,z\n"\
"symop x,-y,z\n"\
"symop x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  63\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2c 2 (z,y,-x)'\n"\
"symbol xHM  'A m a m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,-z\n"\
"symop -x,-y,z\n"\
"symop -x+1/2,y,-z\n"\
"symop -x,-y,-z\n"\
"symop -x+1/2,y,z\n"\
"symop x,y,-z\n"\
"symop x+1/2,-y,z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  63\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2c 2 (y,z,x)'\n"\
"symbol xHM  'B b m m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,-z\n"\
"symop -x,-y,z\n"\
"symop x,-y+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop x,-y+1/2,z\n"\
"symop x,y,-z\n"\
"symop -x,y+1/2,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  63\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2c 2 (-x,z,y)'\n"\
"symbol xHM  'B m m b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,-z\n"\
"symop x,-y,-z\n"\
"symop -x,-y+1/2,z\n"\
"symop -x,-y,-z\n"\
"symop x,-y+1/2,z\n"\
"symop -x,y,z\n"\
"symop x,y+1/2,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  64\n"\
"basisop x,y,z\n"\
"symbol ccp4 64\n"\
"symbol Hall '-C 2ac 2'\n"\
"symbol xHM  'C m c a'\n"\
"symbol old  'C 2/m 2/c 21/a' 'C m c a'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop x,-y,-z\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y,-z+1/2\n"\
"symop -x,y,z\n"\
"symop x+1/2,-y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  64\n"\
"basisop y,-x,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2ac 2 (y,-x,z)'\n"\
"symbol xHM  'C c m b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y+1/2,z+1/2\n"\
"symop -x,y,-z\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,y+1/2,-z+1/2\n"\
"symop x,-y,z\n"\
"symop -x,y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  64\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2ac 2 (z,x,y)'\n"\
"symbol xHM  'A b m a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop -x,y,-z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop -x,-y,-z\n"\
"symop -x+1/2,y+1/2,z\n"\
"symop x,-y,z\n"\
"symop x+1/2,y+1/2,-z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  64\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2ac 2 (z,y,-x)'\n"\
"symbol xHM  'A c a m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x+1/2,-y,-z+1/2\n"\
"symop -x,-y,z\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop -x+1/2,y,z+1/2\n"\
"symop x,y,-z\n"\
"symop x+1/2,-y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  64\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2ac 2 (y,z,x)'\n"\
"symbol xHM  'B b c m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop -x,-y,z\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,-y+1/2,z+1/2\n"\
"symop x,y,-z\n"\
"symop -x,y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  64\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2ac 2 (-x,z,y)'\n"\
"symbol xHM  'B m a b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,y+1/2,-z\n"\
"symop x,-y,-z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,-y+1/2,z\n"\
"symop -x,y,z\n"\
"symop x+1/2,y+1/2,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  65\n"\
"basisop x,y,z\n"\
"symbol ccp4 65\n"\
"symbol Hall '-C 2 2'\n"\
"symbol xHM  'C m m m'\n"\
"symbol old  'C 2/m 2/m 2/m'  'C m m m'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z\n"\
"symop -x,y,-z\n"\
"symop -x,-y,-z\n"\
"symop x,y,-z\n"\
"symop -x,y,z\n"\
"symop x,-y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  65\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2 2 (z,x,y)'\n"\
"symbol xHM  'A m m m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop -x,y,-z\n"\
"symop -x,-y,z\n"\
"symop -x,-y,-z\n"\
"symop -x,y,z\n"\
"symop x,-y,z\n"\
"symop x,y,-z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  65\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2 2 (y,z,x)'\n"\
"symbol xHM  'B m m m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z\n"\
"symop -x,-y,-z\n"\
"symop x,-y,z\n"\
"symop x,y,-z\n"\
"symop -x,y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  66\n"\
"basisop x,y,z\n"\
"symbol ccp4 66\n"\
"symbol Hall '-C 2 2c'\n"\
"symbol xHM  'C c c m'\n"\
"symbol old  'C 2/c 2/c 2/m'  'C c c m'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z+1/2\n"\
"symop -x,y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,y,-z\n"\
"symop -x,y,z+1/2\n"\
"symop x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  66\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2 2c (z,x,y)'\n"\
"symbol xHM  'A m a a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop -x+1/2,y,-z\n"\
"symop -x+1/2,-y,z\n"\
"symop -x,-y,-z\n"\
"symop -x,y,z\n"\
"symop x+1/2,-y,z\n"\
"symop x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  66\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2 2c (y,z,x)'\n"\
"symbol xHM  'B b m b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop -x,-y+1/2,z\n"\
"symop x,-y+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop x,-y,z\n"\
"symop x,y+1/2,-z\n"\
"symop -x,y+1/2,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  67\n"\
"basisop x,y,z\n"\
"symbol ccp4 67\n"\
"symbol Hall '-C 2a 2'\n"\
"symbol xHM  'C m m a'\n"\
"symbol old  'C 2/m 2/m 2/a' 'C m m a'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y,z\n"\
"symop x,-y,-z\n"\
"symop -x+1/2,y,-z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y,-z\n"\
"symop -x,y,z\n"\
"symop x+1/2,-y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  67\n"\
"basisop x+1/4,y+1/4,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2a 2 (x+1/4,y+1/4,z)'\n"\
"symbol xHM  'C m m b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y+1/2,z\n"\
"symop x,-y+1/2,-z\n"\
"symop -x,y,-z\n"\
"symop -x+1/2,-y+1/2,-z\n"\
"symop x+1/2,y,-z\n"\
"symop -x+1/2,y,z\n"\
"symop x+1/2,-y+1/2,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  67\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2a 2 (z,x,y)'\n"\
"symbol xHM  'A b m m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y+1/2,-z\n"\
"symop -x,y,-z\n"\
"symop -x,-y+1/2,z\n"\
"symop -x,-y,-z\n"\
"symop -x,y+1/2,z\n"\
"symop x,-y,z\n"\
"symop x,y+1/2,-z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  67\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2a 2 (z,y,-x)'\n"\
"symbol xHM  'A c m m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z+1/2\n"\
"symop -x,-y,z\n"\
"symop -x,y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop -x,y,z+1/2\n"\
"symop x,y,-z\n"\
"symop x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  67\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2a 2 (y,z,x)'\n"\
"symbol xHM  'B m c m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z+1/2\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,-y,z+1/2\n"\
"symop x,y,-z\n"\
"symop -x,y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  67\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2a 2 (-x,z,y)'\n"\
"symbol xHM  'B m a m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/2; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,y,-z\n"\
"symop x,-y,-z\n"\
"symop -x+1/2,-y,z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,-y,z\n"\
"symop -x,y,z\n"\
"symop x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  68\n"\
"basisop x-1/2,y-1/4,z+1/4\n"\
"symbol ccp4 68\n"\
"symbol Hall '-C 2a 2ac (x-1/2,y-1/4,z+1/4)'\n"\
"symbol xHM  'C c c a :1'\n"\
"symbol old  'C c c a' 'C 2/c 2/c 2/a'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop -x,y,-z\n"\
"symop -x,-y+1/2,-z+1/2\n"\
"symop x+1/2,y,-z+1/2\n"\
"symop -x+1/2,y,z+1/2\n"\
"symop x,-y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  68\n"\
"basisop x,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2a 2ac'\n"\
"symbol xHM  'C c c a :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y,z\n"\
"symop x+1/2,-y,-z+1/2\n"\
"symop -x,y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y,-z\n"\
"symop -x+1/2,y,z+1/2\n"\
"symop x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  68\n"\
"basisop x-1/2,y-1/4,z+1/4\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2a 2ac (x-1/2,y-1/4,z+1/4)'\n"\
"symbol xHM  'C c c b :1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop -x,y,-z\n"\
"symop -x,-y+1/2,-z+1/2\n"\
"symop x+1/2,y,-z+1/2\n"\
"symop -x+1/2,y,z+1/2\n"\
"symop x,-y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  68\n"\
"basisop x-1/4,y-1/4,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2a 2ac (x-1/4,y-1/4,z)'\n"\
"symbol xHM  'C c c b :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y+1/2,z\n"\
"symop x+1/2,-y+1/2,-z+1/2\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop -x+1/2,-y+1/2,-z\n"\
"symop x+1/2,y,-z\n"\
"symop -x,y,z+1/2\n"\
"symop x,-y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  68\n"\
"basisop z+1/4,x-1/2,y-1/4\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2a 2ac (z+1/4,x-1/2,y-1/4)'\n"\
"symbol xHM  'A b a a :1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop -x,-y,z\n"\
"symop -x+1/2,-y,-z+1/2\n"\
"symop -x+1/2,y+1/2,z\n"\
"symop x+1/2,-y+1/2,z\n"\
"symop x+1/2,y,-z+1/2\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  68\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2a 2ac (z,x,y)'\n"\
"symbol xHM  'A b a a :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y+1/2,-z\n"\
"symop -x+1/2,y+1/2,-z\n"\
"symop -x+1/2,-y,z\n"\
"symop -x,-y,-z\n"\
"symop -x,y+1/2,z\n"\
"symop x+1/2,-y+1/2,z\n"\
"symop x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  68\n"\
"basisop z+1/4,x-1/2,y-1/4\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2a 2ac (z+1/4,x-1/2,y-1/4)'\n"\
"symbol xHM  'A c a a :1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop -x,-y,z\n"\
"symop -x+1/2,-y,-z+1/2\n"\
"symop -x+1/2,y+1/2,z\n"\
"symop x+1/2,-y+1/2,z\n"\
"symop x+1/2,y,-z+1/2\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  68\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2a 2ac (z,y,-x)'\n"\
"symbol xHM  'A c a a :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-A 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z+1/2\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop -x+1/2,y,-z\n"\
"symop -x,-y,-z\n"\
"symop -x,y,z+1/2\n"\
"symop x+1/2,y,-z+1/2\n"\
"symop x+1/2,-y,z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  68\n"\
"basisop y-1/4,z+1/4,x-1/2\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2a 2ac (y-1/4,z+1/4,x-1/2)'\n"\
"symbol xHM  'B b c b :1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop x,-y,-z\n"\
"symop -x+1/2,-y+1/2,-z\n"\
"symop x,-y+1/2,z+1/2\n"\
"symop x,y+1/2,-z+1/2\n"\
"symop -x+1/2,y+1/2,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  68\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2a 2ac (y,z,x)'\n"\
"symbol xHM  'B b c b :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z+1/2\n"\
"symop -x,-y+1/2,z+1/2\n"\
"symop x,-y+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop x,-y,z+1/2\n"\
"symop x,y+1/2,-z+1/2\n"\
"symop -x,y+1/2,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  68\n"\
"basisop y-1/4,z+1/4,x-1/2\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2a 2ac (y-1/4,z+1/4,x-1/2)'\n"\
"symbol xHM  'B b a b :1'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop x,-y,-z\n"\
"symop -x+1/2,-y+1/2,-z\n"\
"symop x,-y+1/2,z+1/2\n"\
"symop x,y+1/2,-z+1/2\n"\
"symop -x+1/2,y+1/2,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  68\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-C 2a 2ac (-x,z,y)'\n"\
"symbol xHM  'B b a b :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-B 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,y,-z\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop -x,-y+1/2,z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,-y,z\n"\
"symop -x+1/2,y+1/2,z\n"\
"symop x,y+1/2,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  69\n"\
"basisop x,y,z\n"\
"symbol ccp4 69\n"\
"symbol Hall '-F 2 2'\n"\
"symbol xHM  'F m m m'\n"\
"symbol old  'F 2/m 2/m 2/m' 'F m m m'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-F 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z\n"\
"symop -x,y,-z\n"\
"symop -x,-y,-z\n"\
"symop x,y,-z\n"\
"symop -x,y,z\n"\
"symop x,-y,z\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"cenop x+1/2,y,z+1/2\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  70\n"\
"basisop x+1/8,y+1/8,z+1/8\n"\
"symbol ccp4 70\n"\
"symbol Hall '-F 2uv 2vw (x+1/8,y+1/8,z+1/8)'\n"\
"symbol xHM  'F d d d :1'\n"\
"symbol old  'F 2/d 2/d 2/d' 'F d d d'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-F 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/8; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/8; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop -x+1/4,-y+1/4,-z+1/4\n"\
"symop x+3/4,y+3/4,-z+1/4\n"\
"symop -x+1/4,y+3/4,z+3/4\n"\
"symop x+3/4,-y+1/4,z+3/4\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"cenop x+1/2,y,z+1/2\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  70\n"\
"basisop x,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-F 2uv 2vw'\n"\
"symbol xHM  'F d d d :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-F 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/8; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/8; 1/8<=y<=3/8; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/4,-y+1/4,z\n"\
"symop x,-y+1/4,-z+1/4\n"\
"symop -x+1/4,y,-z+1/4\n"\
"symop -x,-y,-z\n"\
"symop x+3/4,y+3/4,-z\n"\
"symop -x,y+3/4,z+3/4\n"\
"symop x+3/4,-y,z+3/4\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"cenop x+1/2,y,z+1/2\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  71\n"\
"basisop x,y,z\n"\
"symbol ccp4 71\n"\
"symbol Hall '-I 2 2'\n"\
"symbol xHM  'I m m m'\n"\
"symbol old  'I 2/m 2/m 2/m' 'I m m m'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z\n"\
"symop -x,y,-z\n"\
"symop -x,-y,-z\n"\
"symop x,y,-z\n"\
"symop -x,y,z\n"\
"symop x,-y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  72\n"\
"basisop x,y,z\n"\
"symbol ccp4 72\n"\
"symbol Hall '-I 2 2c'\n"\
"symbol xHM  'I b a m'\n"\
"symbol old  'I 2/b 2/a 2/m' 'I b a m'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z+1/2\n"\
"symop -x,y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,y,-z\n"\
"symop -x,y,z+1/2\n"\
"symop x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  72\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-I 2 2c (z,x,y)'\n"\
"symbol xHM  'I m c b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z\n"\
"symop -x+1/2,y,-z\n"\
"symop -x+1/2,-y,z\n"\
"symop -x,-y,-z\n"\
"symop -x,y,z\n"\
"symop x+1/2,-y,z\n"\
"symop x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  72\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-I 2 2c (y,z,x)'\n"\
"symbol xHM  'I c m a'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z\n"\
"symop -x,-y+1/2,z\n"\
"symop x,-y+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop x,-y,z\n"\
"symop x,y+1/2,-z\n"\
"symop -x,y+1/2,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  73\n"\
"basisop x,y,z\n"\
"symbol ccp4 73\n"\
"symbol Hall '-I 2b 2c'\n"\
"symbol xHM  'I b c a'\n"\
"symbol old  'I 21/b 21/c 21/a' 'I b c a'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y+1/2,z\n"\
"symop x,-y,-z+1/2\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,y+1/2,-z\n"\
"symop -x,y,z+1/2\n"\
"symop x,-y+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  73\n"\
"basisop x+1/4,y-1/4,z+1/4\n"\
"symbol ccp4 0\n"\
"symbol Hall '-I 2b 2c (x+1/4,y-1/4,z+1/4)'\n"\
"symbol xHM  'I c a b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y,z\n"\
"symop x,-y+1/2,-z\n"\
"symop -x+1/2,y+1/2,-z\n"\
"symop -x+1/2,-y+1/2,-z+1/2\n"\
"symop x,y+1/2,-z+1/2\n"\
"symop -x+1/2,y,z+1/2\n"\
"symop x,-y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  74\n"\
"basisop x,y,z\n"\
"symbol ccp4 74\n"\
"symbol Hall '-I 2b 2'\n"\
"symbol xHM  'I m m a'\n"\
"symbol old  'I 21/m 21/m 21/a' 'I m m a'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y+1/2,z\n"\
"symop x,-y,-z\n"\
"symop -x,y+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop x,y+1/2,-z\n"\
"symop -x,y,z\n"\
"symop x,-y+1/2,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  74\n"\
"basisop x+1/4,y-1/4,z+1/4\n"\
"symbol ccp4 0\n"\
"symbol Hall '-I 2b 2 (x+1/4,y-1/4,z+1/4)'\n"\
"symbol xHM  'I m m b'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y,z\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop -x+1/2,y+1/2,-z+1/2\n"\
"symop -x+1/2,-y+1/2,-z+1/2\n"\
"symop x,y+1/2,-z+1/2\n"\
"symop -x+1/2,y,z\n"\
"symop x,-y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  74\n"\
"basisop z,x,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-I 2b 2 (z,x,y)'\n"\
"symbol xHM  'I b m m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y,-z+1/2\n"\
"symop -x,y,-z\n"\
"symop -x,-y,z+1/2\n"\
"symop -x,-y,-z\n"\
"symop -x,y,z+1/2\n"\
"symop x,-y,z\n"\
"symop x,y,-z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  74\n"\
"basisop z,y,-x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-I 2b 2 (z,y,-x)'\n"\
"symbol xHM  'I c m m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<=3/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x,-y+1/2,-z\n"\
"symop -x,-y,z\n"\
"symop -x,y+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop -x,y+1/2,z\n"\
"symop x,y,-z\n"\
"symop x,-y+1/2,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  74\n"\
"basisop y,z,x\n"\
"symbol ccp4 0\n"\
"symbol Hall '-I 2b 2 (y,z,x)'\n"\
"symbol xHM  'I m c m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<=3/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,y,-z\n"\
"symop -x,-y,z\n"\
"symop x+1/2,-y,-z\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,-y,z\n"\
"symop x,y,-z\n"\
"symop -x+1/2,y,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  74\n"\
"basisop -x,z,y\n"\
"symbol ccp4 0\n"\
"symbol Hall '-I 2b 2 (-x,z,y)'\n"\
"symbol xHM  'I m a m'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp '-P 2 2' 'mmm'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,y,-z+1/2\n"\
"symop x,-y,-z\n"\
"symop -x,-y,z+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,-y,z+1/2\n"\
"symop -x,y,z\n"\
"symop x,y,-z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  75\n"\
"basisop x,y,z\n"\
"symbol ccp4 75\n"\
"symbol Hall ' P 4'\n"\
"symbol xHM  'P 4'\n"\
"symbol old  'P 4'\n"\
"symbol laue '-P 4' '4/m'\n"\
"symbol patt '-P 4' '4/m'\n"\
"symbol pgrp ' P 4' '4'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  76\n"\
"basisop x,y,z\n"\
"symbol ccp4 76\n"\
"symbol Hall ' P 4w'\n"\
"symbol xHM  'P 41'\n"\
"symbol old  'P 41'\n"\
"symbol laue '-P 4' '4/m'\n"\
"symbol patt '-P 4' '4/m'\n"\
"symbol pgrp ' P 4' '4'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<1/4\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x,z+1/4\n"\
"symop -x,-y,z+1/2\n"\
"symop y,-x,z+3/4\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  77\n"\
"basisop x,y,z\n"\
"symbol ccp4 77\n"\
"symbol Hall ' P 4c'\n"\
"symbol xHM  'P 42'\n"\
"symbol old  'P 42'\n"\
"symbol laue '-P 4' '4/m'\n"\
"symbol patt '-P 4' '4/m'\n"\
"symbol pgrp ' P 4' '4'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<1; 0<=z<1/2\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x,z+1/2\n"\
"symop -x,-y,z\n"\
"symop y,-x,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  78\n"\
"basisop x,y,z\n"\
"symbol ccp4 78\n"\
"symbol Hall ' P 4cw'\n"\
"symbol xHM  'P 43'\n"\
"symbol old  'P 43'\n"\
"symbol laue '-P 4' '4/m'\n"\
"symbol patt '-P 4' '4/m'\n"\
"symbol pgrp ' P 4' '4'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<1/4\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x,z+3/4\n"\
"symop -x,-y,z+1/2\n"\
"symop y,-x,z+1/4\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  79\n"\
"basisop x,y,z\n"\
"symbol ccp4 79\n"\
"symbol Hall ' I 4'\n"\
"symbol xHM  'I 4'\n"\
"symbol old  'I 4'\n"\
"symbol laue '-P 4' '4/m'\n"\
"symbol patt '-I 4' '4/m'\n"\
"symbol pgrp ' P 4' '4'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  80\n"\
"basisop x,y,z\n"\
"symbol ccp4 80\n"\
"symbol Hall ' I 4bw'\n"\
"symbol xHM  'I 41'\n"\
"symbol old  'I 41'\n"\
"symbol laue '-P 4' '4/m'\n"\
"symbol patt '-I 4' '4/m'\n"\
"symbol pgrp ' P 4' '4'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<1; 0<=z<1/4\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<1/4\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<=3/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x+1/2,z+1/4\n"\
"symop -x+1/2,-y+1/2,z+1/2\n"\
"symop y+1/2,-x,z+3/4\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  81\n"\
"basisop x,y,z\n"\
"symbol ccp4 81\n"\
"symbol Hall ' P -4'\n"\
"symbol xHM  'P -4'\n"\
"symbol old  'P -4'\n"\
"symbol laue '-P 4' '4/m'\n"\
"symbol patt '-P 4' '4/m'\n"\
"symbol pgrp ' P -4' '-4'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop y,-x,-z\n"\
"symop -x,-y,z\n"\
"symop -y,x,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  82\n"\
"basisop x,y,z\n"\
"symbol ccp4 82\n"\
"symbol Hall ' I -4'\n"\
"symbol xHM  'I -4'\n"\
"symbol old  'I -4'\n"\
"symbol laue '-P 4' '4/m'\n"\
"symbol patt '-I 4' '4/m'\n"\
"symbol pgrp ' P -4' '-4'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop y,-x,-z\n"\
"symop -x,-y,z\n"\
"symop -y,x,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  83\n"\
"basisop x,y,z\n"\
"symbol ccp4 83\n"\
"symbol Hall '-P 4'\n"\
"symbol xHM  'P 4/m'\n"\
"symbol old  'P 4/m'\n"\
"symbol laue '-P 4' '4/m'\n"\
"symbol patt '-P 4' '4/m'\n"\
"symbol pgrp '-P 4' '4/m'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop -x,-y,-z\n"\
"symop y,-x,-z\n"\
"symop x,y,-z\n"\
"symop -y,x,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  84\n"\
"basisop x,y,z\n"\
"symbol ccp4 84\n"\
"symbol Hall '-P 4c'\n"\
"symbol xHM  'P 42/m'\n"\
"symbol old  'P 42/m'\n"\
"symbol laue '-P 4' '4/m'\n"\
"symbol patt '-P 4' '4/m'\n"\
"symbol pgrp '-P 4' '4/m'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x,z+1/2\n"\
"symop -x,-y,z\n"\
"symop y,-x,z+1/2\n"\
"symop -x,-y,-z\n"\
"symop y,-x,-z+1/2\n"\
"symop x,y,-z\n"\
"symop -y,x,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  85\n"\
"basisop x-1/4,y+1/4,z\n"\
"symbol ccp4 85\n"\
"symbol Hall '-P 4a (x-1/4,y+1/4,z)'\n"\
"symbol xHM  'P 4/n :1'\n"\
"symbol old  'P 4/n'\n"\
"symbol laue '-P 4' '4/m'\n"\
"symbol patt '-P 4' '4/m'\n"\
"symbol pgrp '-P 4' '4/m'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/2,x+1/2,z\n"\
"symop -x,-y,z\n"\
"symop y+1/2,-x+1/2,z\n"\
"symop -x+1/2,-y+1/2,-z\n"\
"symop y,-x,-z\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -y,x,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  85\n"\
"basisop x,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 4a'\n"\
"symbol xHM  'P 4/n :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 4' '4/m'\n"\
"symbol patt '-P 4' '4/m'\n"\
"symbol pgrp '-P 4' '4/m'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<=3/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/2,x,z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop y,-x+1/2,z\n"\
"symop -x,-y,-z\n"\
"symop y+1/2,-x,-z\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -y,x+1/2,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  86\n"\
"basisop x+1/4,y+1/4,z+1/4\n"\
"symbol ccp4 86\n"\
"symbol Hall '-P 4bc (x+1/4,y+1/4,z+1/4)'\n"\
"symbol xHM  'P 42/n :1'\n"\
"symbol old  'P 42/n'\n"\
"symbol laue '-P 4' '4/m'\n"\
"symbol patt '-P 4' '4/m'\n"\
"symbol pgrp '-P 4' '4/m'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/2,x+1/2,z+1/2\n"\
"symop -x,-y,z\n"\
"symop y+1/2,-x+1/2,z+1/2\n"\
"symop -x+1/2,-y+1/2,-z+1/2\n"\
"symop y,-x,-z\n"\
"symop x+1/2,y+1/2,-z+1/2\n"\
"symop -y,x,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  86\n"\
"basisop x,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 4bc'\n"\
"symbol xHM  'P 42/n :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 4' '4/m'\n"\
"symbol patt '-P 4' '4/m'\n"\
"symbol pgrp '-P 4' '4/m'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<=3/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x+1/2,z+1/2\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop y+1/2,-x,z+1/2\n"\
"symop -x,-y,-z\n"\
"symop y,-x+1/2,-z+1/2\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -y+1/2,x,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  87\n"\
"basisop x,y,z\n"\
"symbol ccp4 87\n"\
"symbol Hall '-I 4'\n"\
"symbol xHM  'I 4/m'\n"\
"symbol old  'I 4/m'\n"\
"symbol laue '-P 4' '4/m'\n"\
"symbol patt '-I 4' '4/m'\n"\
"symbol pgrp '-P 4' '4/m'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/4\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop -x,-y,-z\n"\
"symop y,-x,-z\n"\
"symop x,y,-z\n"\
"symop -y,x,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  88\n"\
"basisop x,y+1/4,z+1/8\n"\
"symbol ccp4 88\n"\
"symbol Hall '-I 4ad (x,y+1/4,z+1/8)'\n"\
"symbol xHM  'I 41/a :1'\n"\
"symbol old  'I 41/a'\n"\
"symbol laue '-P 4' '4/m'\n"\
"symbol patt '-I 4' '4/m'\n"\
"symbol pgrp '-P 4' '4/m'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x+1/2,z+1/4\n"\
"symop -x+1/2,-y+1/2,z+1/2\n"\
"symop y+1/2,-x,z+3/4\n"\
"symop -x,-y+1/2,-z+1/4\n"\
"symop y,-x,-z\n"\
"symop x+1/2,y,-z+3/4\n"\
"symop -y+1/2,x+1/2,-z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  88\n"\
"basisop x,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-I 4ad'\n"\
"symbol xHM  'I 41/a :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 4' '4/m'\n"\
"symbol patt '-I 4' '4/m'\n"\
"symbol pgrp '-P 4' '4/m'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+3/4,x+1/4,z+1/4\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop y+3/4,-x+3/4,z+3/4\n"\
"symop -x,-y,-z\n"\
"symop y+1/4,-x+3/4,-z+3/4\n"\
"symop x+1/2,y,-z+1/2\n"\
"symop -y+1/4,x+1/4,-z+1/4\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  89\n"\
"basisop x,y,z\n"\
"symbol ccp4 89\n"\
"symbol Hall ' P 4 2'\n"\
"symbol xHM  'P 4 2 2'\n"\
"symbol old  'P 4 2 2'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P 4 2' '422'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop x,-y,-z\n"\
"symop y,x,-z\n"\
"symop -x,y,-z\n"\
"symop -y,-x,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  90\n"\
"basisop x,y,z\n"\
"symbol ccp4 90\n"\
"symbol Hall ' P 4ab 2ab'\n"\
"symbol xHM  'P 4 21 2'\n"\
"symbol old  'P 4 21 2'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P 4 2' '422'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/2,x+1/2,z\n"\
"symop -x,-y,z\n"\
"symop y+1/2,-x+1/2,z\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop y,x,-z\n"\
"symop -x+1/2,y+1/2,-z\n"\
"symop -y,-x,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  91\n"\
"basisop x,y,z\n"\
"symbol ccp4 91\n"\
"symbol Hall ' P 4w 2c'\n"\
"symbol xHM  'P 41 2 2'\n"\
"symbol old  'P 41 2 2'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P 4 2' '422'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<=1/8\n"\
"mapasu zero 0<=x<1; 0<=y<1; 0<=z<=1/8\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; -1/8<=z<=3/8\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x,z+1/4\n"\
"symop -x,-y,z+1/2\n"\
"symop y,-x,z+3/4\n"\
"symop x,-y,-z+1/2\n"\
"symop y,x,-z+3/4\n"\
"symop -x,y,-z\n"\
"symop -y,-x,-z+1/4\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  92\n"\
"basisop x,y,z\n"\
"symbol ccp4 92\n"\
"symbol Hall ' P 4abw 2nw'\n"\
"symbol xHM  'P 41 21 2'\n"\
"symbol old  'P 41 21 2'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P 4 2' '422'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<=1/8\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/2,x+1/2,z+1/4\n"\
"symop -x,-y,z+1/2\n"\
"symop y+1/2,-x+1/2,z+3/4\n"\
"symop x+1/2,-y+1/2,-z+3/4\n"\
"symop y,x,-z\n"\
"symop -x+1/2,y+1/2,-z+1/4\n"\
"symop -y,-x,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  93\n"\
"basisop x,y,z\n"\
"symbol ccp4 93\n"\
"symbol Hall ' P 4c 2'\n"\
"symbol xHM  'P 42 2 2'\n"\
"symbol old  'P 42 2 2'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P 4 2' '422'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<1; 0<=z<=1/4\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x,z+1/2\n"\
"symop -x,-y,z\n"\
"symop y,-x,z+1/2\n"\
"symop x,-y,-z\n"\
"symop y,x,-z+1/2\n"\
"symop -x,y,-z\n"\
"symop -y,-x,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  94\n"\
"basisop x,y,z\n"\
"symbol ccp4 94\n"\
"symbol Hall ' P 4n 2n'\n"\
"symbol xHM  'P 42 21 2'\n"\
"symbol old  'P 42 21 2'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P 4 2' '422'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/2,x+1/2,z+1/2\n"\
"symop -x,-y,z\n"\
"symop y+1/2,-x+1/2,z+1/2\n"\
"symop x+1/2,-y+1/2,-z+1/2\n"\
"symop y,x,-z\n"\
"symop -x+1/2,y+1/2,-z+1/2\n"\
"symop -y,-x,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  95\n"\
"basisop x,y,z\n"\
"symbol ccp4 95\n"\
"symbol Hall ' P 4cw 2c'\n"\
"symbol xHM  'P 43 2 2'\n"\
"symbol old  'P 43 2 2'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P 4 2' '422'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<=1/8\n"\
"mapasu zero 0<=x<1; 0<=y<1; 0<=z<=1/8\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 1/8<=z<=5/8\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x,z+3/4\n"\
"symop -x,-y,z+1/2\n"\
"symop y,-x,z+1/4\n"\
"symop x,-y,-z+1/2\n"\
"symop y,x,-z+1/4\n"\
"symop -x,y,-z\n"\
"symop -y,-x,-z+3/4\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  96\n"\
"basisop x,y,z\n"\
"symbol ccp4 96\n"\
"symbol Hall ' P 4nw 2abw'\n"\
"symbol xHM  'P 43 21 2'\n"\
"symbol old  'P 43 21 2'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P 4 2' '422'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<=1/8\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/2,x+1/2,z+3/4\n"\
"symop -x,-y,z+1/2\n"\
"symop y+1/2,-x+1/2,z+1/4\n"\
"symop x+1/2,-y+1/2,-z+1/4\n"\
"symop y,x,-z\n"\
"symop -x+1/2,y+1/2,-z+3/4\n"\
"symop -y,-x,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  97\n"\
"basisop x,y,z\n"\
"symbol ccp4 97\n"\
"symbol Hall ' I 4 2'\n"\
"symbol xHM  'I 4 2 2'\n"\
"symbol old  'I 4 2 2'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-I 4 2' '4/mmm'\n"\
"symbol pgrp ' P 4 2' '422'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/4\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/4\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop x,-y,-z\n"\
"symop y,x,-z\n"\
"symop -x,y,-z\n"\
"symop -y,-x,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  98\n"\
"basisop x,y,z\n"\
"symbol ccp4 98\n"\
"symbol Hall ' I 4bw 2bw'\n"\
"symbol xHM  'I 41 2 2'\n"\
"symbol old  'I 41 2 2'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-I 4 2' '4/mmm'\n"\
"symbol pgrp ' P 4 2' '422'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<1; 0<=z<=1/8\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/4; -1/4<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x+1/2,z+1/4\n"\
"symop -x+1/2,-y+1/2,z+1/2\n"\
"symop y+1/2,-x,z+3/4\n"\
"symop x,-y+1/2,-z+1/4\n"\
"symop y+1/2,x+1/2,-z+1/2\n"\
"symop -x+1/2,y,-z+3/4\n"\
"symop -y,-x,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  99\n"\
"basisop x,y,z\n"\
"symbol ccp4 99\n"\
"symbol Hall ' P 4 -2'\n"\
"symbol xHM  'P 4 m m'\n"\
"symbol old  'P 4 m m'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P 4 -2' '4mm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop -x,y,z\n"\
"symop -y,-x,z\n"\
"symop x,-y,z\n"\
"symop y,x,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  100\n"\
"basisop x,y,z\n"\
"symbol ccp4 100\n"\
"symbol Hall ' P 4 -2ab'\n"\
"symbol xHM  'P 4 b m'\n"\
"symbol old  'P 4 b m'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P 4 -2' '4mm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<3/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop -x+1/2,y+1/2,z\n"\
"symop -y+1/2,-x+1/2,z\n"\
"symop x+1/2,-y+1/2,z\n"\
"symop y+1/2,x+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  101\n"\
"basisop x,y,z\n"\
"symbol ccp4 101\n"\
"symbol Hall ' P 4c -2c'\n"\
"symbol xHM  'P 42 c m'\n"\
"symbol old  'P 42 c m'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P 4 -2' '4mm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x,z+1/2\n"\
"symop -x,-y,z\n"\
"symop y,-x,z+1/2\n"\
"symop -x,y,z+1/2\n"\
"symop -y,-x,z\n"\
"symop x,-y,z+1/2\n"\
"symop y,x,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  102\n"\
"basisop x,y,z\n"\
"symbol ccp4 102\n"\
"symbol Hall ' P 4n -2n'\n"\
"symbol xHM  'P 42 n m'\n"\
"symbol old  'P 42 n m'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P 4 -2' '4mm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<3/4; 1/4<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y+1/2,x+1/2,z+1/2\n"\
"symop -x,-y,z\n"\
"symop y+1/2,-x+1/2,z+1/2\n"\
"symop -x+1/2,y+1/2,z+1/2\n"\
"symop -y,-x,z\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"symop y,x,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  103\n"\
"basisop x,y,z\n"\
"symbol ccp4 103\n"\
"symbol Hall ' P 4 -2c'\n"\
"symbol xHM  'P 4 c c'\n"\
"symbol old  'P 4 c c'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P 4 -2' '4mm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop -x,y,z+1/2\n"\
"symop -y,-x,z+1/2\n"\
"symop x,-y,z+1/2\n"\
"symop y,x,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  104\n"\
"basisop x,y,z\n"\
"symbol ccp4 104\n"\
"symbol Hall ' P 4 -2n'\n"\
"symbol xHM  'P 4 n c'\n"\
"symbol old  'P 4 n c'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P 4 -2' '4mm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop -x+1/2,y+1/2,z+1/2\n"\
"symop -y+1/2,-x+1/2,z+1/2\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"symop y+1/2,x+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  105\n"\
"basisop x,y,z\n"\
"symbol ccp4 105\n"\
"symbol Hall ' P 4c -2'\n"\
"symbol xHM  'P 42 m c'\n"\
"symbol old  'P 42 m c'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P 4 -2' '4mm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x,z+1/2\n"\
"symop -x,-y,z\n"\
"symop y,-x,z+1/2\n"\
"symop -x,y,z\n"\
"symop -y,-x,z+1/2\n"\
"symop x,-y,z\n"\
"symop y,x,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  106\n"\
"basisop x,y,z\n"\
"symbol ccp4 106\n"\
"symbol Hall ' P 4c -2ab'\n"\
"symbol xHM  'P 42 b c'\n"\
"symbol old  'P 42 b c'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P 4 -2' '4mm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1/2\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x,z+1/2\n"\
"symop -x,-y,z\n"\
"symop y,-x,z+1/2\n"\
"symop -x+1/2,y+1/2,z\n"\
"symop -y+1/2,-x+1/2,z+1/2\n"\
"symop x+1/2,-y+1/2,z\n"\
"symop y+1/2,x+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  107\n"\
"basisop x,y,z\n"\
"symbol ccp4 107\n"\
"symbol Hall ' I 4 -2'\n"\
"symbol xHM  'I 4 m m'\n"\
"symbol old  'I 4 m m'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-I 4 2' '4/mmm'\n"\
"symbol pgrp ' P 4 -2' '4mm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop -x,y,z\n"\
"symop -y,-x,z\n"\
"symop x,-y,z\n"\
"symop y,x,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  108\n"\
"basisop x,y,z\n"\
"symbol ccp4 108\n"\
"symbol Hall ' I 4 -2c'\n"\
"symbol xHM  'I 4 c m'\n"\
"symbol old  'I 4 c m'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-I 4 2' '4/mmm'\n"\
"symbol pgrp ' P 4 -2' '4mm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<3/4; 0<=y<=1/4; 0<=z<1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop -x,y,z+1/2\n"\
"symop -y,-x,z+1/2\n"\
"symop x,-y,z+1/2\n"\
"symop y,x,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  109\n"\
"basisop x,y,z\n"\
"symbol ccp4 109\n"\
"symbol Hall ' I 4bw -2'\n"\
"symbol xHM  'I 41 m d'\n"\
"symbol old  'I 41 m d'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-I 4 2' '4/mmm'\n"\
"symbol pgrp ' P 4 -2' '4mm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x+1/2,z+1/4\n"\
"symop -x+1/2,-y+1/2,z+1/2\n"\
"symop y+1/2,-x,z+3/4\n"\
"symop -x,y,z\n"\
"symop -y,-x+1/2,z+1/4\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"symop y+1/2,x,z+3/4\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  110\n"\
"basisop x,y,z\n"\
"symbol ccp4 110\n"\
"symbol Hall ' I 4bw -2c'\n"\
"symbol xHM  'I 41 c d'\n"\
"symbol old  'I 41 c d'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-I 4 2' '4/mmm'\n"\
"symbol pgrp ' P 4 -2' '4mm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x+1/2,z+1/4\n"\
"symop -x+1/2,-y+1/2,z+1/2\n"\
"symop y+1/2,-x,z+3/4\n"\
"symop -x,y,z+1/2\n"\
"symop -y,-x+1/2,z+3/4\n"\
"symop x+1/2,-y+1/2,z\n"\
"symop y+1/2,x,z+1/4\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  111\n"\
"basisop x,y,z\n"\
"symbol ccp4 111\n"\
"symbol Hall ' P -4 2'\n"\
"symbol xHM  'P -4 2 m'\n"\
"symbol old  'P -4 2 m'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P -4 2' '-4m2'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop y,-x,-z\n"\
"symop -x,-y,z\n"\
"symop -y,x,-z\n"\
"symop x,-y,-z\n"\
"symop -y,-x,z\n"\
"symop -x,y,-z\n"\
"symop y,x,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  112\n"\
"basisop x,y,z\n"\
"symbol ccp4 112\n"\
"symbol Hall ' P -4 2c'\n"\
"symbol xHM  'P -4 2 c'\n"\
"symbol old  'P -4 2 c'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P -4 2' '-4m2'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop y,-x,-z\n"\
"symop -x,-y,z\n"\
"symop -y,x,-z\n"\
"symop x,-y,-z+1/2\n"\
"symop -y,-x,z+1/2\n"\
"symop -x,y,-z+1/2\n"\
"symop y,x,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  113\n"\
"basisop x,y,z\n"\
"symbol ccp4 113\n"\
"symbol Hall ' P -4 2ab'\n"\
"symbol xHM  'P -4 21 m'\n"\
"symbol old  'P -4 21 m'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P -4 2' '-4m2'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<3/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop y,-x,-z\n"\
"symop -x,-y,z\n"\
"symop -y,x,-z\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop -y+1/2,-x+1/2,z\n"\
"symop -x+1/2,y+1/2,-z\n"\
"symop y+1/2,x+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  114\n"\
"basisop x,y,z\n"\
"symbol ccp4 114\n"\
"symbol Hall ' P -4 2n'\n"\
"symbol xHM  'P -4 21 c'\n"\
"symbol old  'P -4 21 c'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P -4 2' '-4m2'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop y,-x,-z\n"\
"symop -x,-y,z\n"\
"symop -y,x,-z\n"\
"symop x+1/2,-y+1/2,-z+1/2\n"\
"symop -y+1/2,-x+1/2,z+1/2\n"\
"symop -x+1/2,y+1/2,-z+1/2\n"\
"symop y+1/2,x+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  115\n"\
"basisop x,y,z\n"\
"symbol ccp4 115\n"\
"symbol Hall ' P -4 -2'\n"\
"symbol xHM  'P -4 m 2'\n"\
"symbol old  'P -4 m 2'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P -4 -2' '-42m'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop y,-x,-z\n"\
"symop -x,-y,z\n"\
"symop -y,x,-z\n"\
"symop -x,y,z\n"\
"symop y,x,-z\n"\
"symop x,-y,z\n"\
"symop -y,-x,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  116\n"\
"basisop x,y,z\n"\
"symbol ccp4 116\n"\
"symbol Hall ' P -4 -2c'\n"\
"symbol xHM  'P -4 c 2'\n"\
"symbol old  'P -4 c 2'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P -4 -2' '-42m'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop y,-x,-z\n"\
"symop -x,-y,z\n"\
"symop -y,x,-z\n"\
"symop -x,y,z+1/2\n"\
"symop y,x,-z+1/2\n"\
"symop x,-y,z+1/2\n"\
"symop -y,-x,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  117\n"\
"basisop x,y,z\n"\
"symbol ccp4 117\n"\
"symbol Hall ' P -4 -2ab'\n"\
"symbol xHM  'P -4 b 2'\n"\
"symbol old  'P -4 b 2'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P -4 -2' '-42m'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop y,-x,-z\n"\
"symop -x,-y,z\n"\
"symop -y,x,-z\n"\
"symop -x+1/2,y+1/2,z\n"\
"symop y+1/2,x+1/2,-z\n"\
"symop x+1/2,-y+1/2,z\n"\
"symop -y+1/2,-x+1/2,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  118\n"\
"basisop x,y,z\n"\
"symbol ccp4 118\n"\
"symbol Hall ' P -4 -2n'\n"\
"symbol xHM  'P -4 n 2'\n"\
"symbol old  'P -4 n 2'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P -4 -2' '-42m'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop y,-x,-z\n"\
"symop -x,-y,z\n"\
"symop -y,x,-z\n"\
"symop -x+1/2,y+1/2,z+1/2\n"\
"symop y+1/2,x+1/2,-z+1/2\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"symop -y+1/2,-x+1/2,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  119\n"\
"basisop x,y,z\n"\
"symbol ccp4 119\n"\
"symbol Hall ' I -4 -2'\n"\
"symbol xHM  'I -4 m 2'\n"\
"symbol old  'I -4 m 2'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-I 4 2' '4/mmm'\n"\
"symbol pgrp ' P -4 -2' '-42m'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/4\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop y,-x,-z\n"\
"symop -x,-y,z\n"\
"symop -y,x,-z\n"\
"symop -x,y,z\n"\
"symop y,x,-z\n"\
"symop x,-y,z\n"\
"symop -y,-x,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  120\n"\
"basisop x,y,z\n"\
"symbol ccp4 120\n"\
"symbol Hall ' I -4 -2c'\n"\
"symbol xHM  'I -4 c 2'\n"\
"symbol old  'I -4 c 2'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-I 4 2' '4/mmm'\n"\
"symbol pgrp ' P -4 -2' '-42m'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<=1/4\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop y,-x,-z\n"\
"symop -x,-y,z\n"\
"symop -y,x,-z\n"\
"symop -x,y,z+1/2\n"\
"symop y,x,-z+1/2\n"\
"symop x,-y,z+1/2\n"\
"symop -y,-x,-z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  121\n"\
"basisop x,y,z\n"\
"symbol ccp4 121\n"\
"symbol Hall ' I -4 2'\n"\
"symbol xHM  'I -4 2 m'\n"\
"symbol old  'I -4 2 m'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-I 4 2' '4/mmm'\n"\
"symbol pgrp ' P -4 2' '-4m2'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<3/4; 1/4<=y<=1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop y,-x,-z\n"\
"symop -x,-y,z\n"\
"symop -y,x,-z\n"\
"symop x,-y,-z\n"\
"symop -y,-x,z\n"\
"symop -x,y,-z\n"\
"symop y,x,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  122\n"\
"basisop x,y,z\n"\
"symbol ccp4 122\n"\
"symbol Hall ' I -4 2bw'\n"\
"symbol xHM  'I -4 2 d'\n"\
"symbol old  'I -4 2 d'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-I 4 2' '4/mmm'\n"\
"symbol pgrp ' P -4 2' '-4m2'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop y,-x,-z\n"\
"symop -x,-y,z\n"\
"symop -y,x,-z\n"\
"symop x,-y+1/2,-z+1/4\n"\
"symop -y+1/2,-x,z+3/4\n"\
"symop -x,y+1/2,-z+1/4\n"\
"symop y+1/2,x,z+3/4\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  123\n"\
"basisop x,y,z\n"\
"symbol ccp4 123\n"\
"symbol Hall '-P 4 2'\n"\
"symbol xHM  'P 4/m m m'\n"\
"symbol old  'P 4/m 2/m 2/m' 'P4/m m m'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop x,-y,-z\n"\
"symop y,x,-z\n"\
"symop -x,y,-z\n"\
"symop -y,-x,-z\n"\
"symop -x,-y,-z\n"\
"symop y,-x,-z\n"\
"symop x,y,-z\n"\
"symop -y,x,-z\n"\
"symop -x,y,z\n"\
"symop -y,-x,z\n"\
"symop x,-y,z\n"\
"symop y,x,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  124\n"\
"basisop x,y,z\n"\
"symbol ccp4 124\n"\
"symbol Hall '-P 4 2c'\n"\
"symbol xHM  'P 4/m c c'\n"\
"symbol old  'P 4/m 2/c 2/c' 'P4/m c c'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/4\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop x,-y,-z+1/2\n"\
"symop y,x,-z+1/2\n"\
"symop -x,y,-z+1/2\n"\
"symop -y,-x,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop y,-x,-z\n"\
"symop x,y,-z\n"\
"symop -y,x,-z\n"\
"symop -x,y,z+1/2\n"\
"symop -y,-x,z+1/2\n"\
"symop x,-y,z+1/2\n"\
"symop y,x,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  125\n"\
"basisop x-1/4,y-1/4,z\n"\
"symbol ccp4 125\n"\
"symbol Hall '-P 4a 2b (x-1/4,y-1/4,z)'\n"\
"symbol xHM  'P 4/n b m :1'\n"\
"symbol old  'P 4/n 2/b 2/m' 'P4/n b m'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<3/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop x,-y,-z\n"\
"symop y,x,-z\n"\
"symop -x,y,-z\n"\
"symop -y,-x,-z\n"\
"symop -x+1/2,-y+1/2,-z\n"\
"symop y+1/2,-x+1/2,-z\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -y+1/2,x+1/2,-z\n"\
"symop -x+1/2,y+1/2,z\n"\
"symop -y+1/2,-x+1/2,z\n"\
"symop x+1/2,-y+1/2,z\n"\
"symop y+1/2,x+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  125\n"\
"basisop x,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 4a 2b'\n"\
"symbol xHM  'P 4/n b m :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<1; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/2,x,z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop y,-x+1/2,z\n"\
"symop x,-y+1/2,-z\n"\
"symop y,x,-z\n"\
"symop -x+1/2,y,-z\n"\
"symop -y+1/2,-x+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop y+1/2,-x,-z\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -y,x+1/2,-z\n"\
"symop -x,y+1/2,z\n"\
"symop -y,-x,z\n"\
"symop x+1/2,-y,z\n"\
"symop y+1/2,x+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  126\n"\
"basisop x-1/4,y-1/4,z-1/4\n"\
"symbol ccp4 126\n"\
"symbol Hall '-P 4a 2bc (x-1/4,y-1/4,z-1/4)'\n"\
"symbol xHM  'P 4/n n c :1'\n"\
"symbol old  'P 4/n 2/n 2/c' 'P4/n n c'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop x,-y,-z\n"\
"symop y,x,-z\n"\
"symop -x,y,-z\n"\
"symop -y,-x,-z\n"\
"symop -x+1/2,-y+1/2,-z+1/2\n"\
"symop y+1/2,-x+1/2,-z+1/2\n"\
"symop x+1/2,y+1/2,-z+1/2\n"\
"symop -y+1/2,x+1/2,-z+1/2\n"\
"symop -x+1/2,y+1/2,z+1/2\n"\
"symop -y+1/2,-x+1/2,z+1/2\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"symop y+1/2,x+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  126\n"\
"basisop x,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 4a 2bc'\n"\
"symbol xHM  'P 4/n n c :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<=3/4; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/2,x,z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop y,-x+1/2,z\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop y,x,-z+1/2\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop -y+1/2,-x+1/2,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop y+1/2,-x,-z\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -y,x+1/2,-z\n"\
"symop -x,y+1/2,z+1/2\n"\
"symop -y,-x,z+1/2\n"\
"symop x+1/2,-y,z+1/2\n"\
"symop y+1/2,x+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  127\n"\
"basisop x,y,z\n"\
"symbol ccp4 127\n"\
"symbol Hall '-P 4 2ab'\n"\
"symbol xHM  'P 4/m b m'\n"\
"symbol old  'P 4/m 21/b 2/m' 'P4/m b m'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<3/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop y+1/2,x+1/2,-z\n"\
"symop -x+1/2,y+1/2,-z\n"\
"symop -y+1/2,-x+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop y,-x,-z\n"\
"symop x,y,-z\n"\
"symop -y,x,-z\n"\
"symop -x+1/2,y+1/2,z\n"\
"symop -y+1/2,-x+1/2,z\n"\
"symop x+1/2,-y+1/2,z\n"\
"symop y+1/2,x+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  128\n"\
"basisop x,y,z\n"\
"symbol ccp4 128\n"\
"symbol Hall '-P 4 2n'\n"\
"symbol xHM  'P 4/m n c'\n"\
"symbol old  'P 4/m 21/n 2/c' 'P4/m n c'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/4\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop x+1/2,-y+1/2,-z+1/2\n"\
"symop y+1/2,x+1/2,-z+1/2\n"\
"symop -x+1/2,y+1/2,-z+1/2\n"\
"symop -y+1/2,-x+1/2,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop y,-x,-z\n"\
"symop x,y,-z\n"\
"symop -y,x,-z\n"\
"symop -x+1/2,y+1/2,z+1/2\n"\
"symop -y+1/2,-x+1/2,z+1/2\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"symop y+1/2,x+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  129\n"\
"basisop x-1/4,y+1/4,z\n"\
"symbol ccp4 129\n"\
"symbol Hall '-P 4a 2a (x-1/4,y+1/4,z)'\n"\
"symbol xHM  'P 4/n m m :1'\n"\
"symbol old  'P 4/n 21/m 2/m' 'P4/n m m'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/2,x+1/2,z\n"\
"symop -x,-y,z\n"\
"symop y+1/2,-x+1/2,z\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop y,x,-z\n"\
"symop -x+1/2,y+1/2,-z\n"\
"symop -y,-x,-z\n"\
"symop -x+1/2,-y+1/2,-z\n"\
"symop y,-x,-z\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -y,x,-z\n"\
"symop -x,y,z\n"\
"symop -y+1/2,-x+1/2,z\n"\
"symop x,-y,z\n"\
"symop y+1/2,x+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  129\n"\
"basisop x,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 4a 2a'\n"\
"symbol xHM  'P 4/n m m :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<=3/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/2,x,z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop y,-x+1/2,z\n"\
"symop x+1/2,-y,-z\n"\
"symop y+1/2,x+1/2,-z\n"\
"symop -x,y+1/2,-z\n"\
"symop -y,-x,-z\n"\
"symop -x,-y,-z\n"\
"symop y+1/2,-x,-z\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -y,x+1/2,-z\n"\
"symop -x+1/2,y,z\n"\
"symop -y+1/2,-x+1/2,z\n"\
"symop x,-y+1/2,z\n"\
"symop y,x,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  130\n"\
"basisop x-1/4,y+1/4,z\n"\
"symbol ccp4 130\n"\
"symbol Hall '-P 4a 2ac (x-1/4,y+1/4,z)'\n"\
"symbol xHM  'P 4/n c c :1'\n"\
"symbol old  'P 4/n 2/c 2/c' 'P4/n c c'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/2,x+1/2,z\n"\
"symop -x,-y,z\n"\
"symop y+1/2,-x+1/2,z\n"\
"symop x+1/2,-y+1/2,-z+1/2\n"\
"symop y,x,-z+1/2\n"\
"symop -x+1/2,y+1/2,-z+1/2\n"\
"symop -y,-x,-z+1/2\n"\
"symop -x+1/2,-y+1/2,-z\n"\
"symop y,-x,-z\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -y,x,-z\n"\
"symop -x,y,z+1/2\n"\
"symop -y+1/2,-x+1/2,z+1/2\n"\
"symop x,-y,z+1/2\n"\
"symop y+1/2,x+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  130\n"\
"basisop x,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 4a 2ac'\n"\
"symbol xHM  'P 4/n c c :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<=3/4; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/2,x,z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop y,-x+1/2,z\n"\
"symop x+1/2,-y,-z+1/2\n"\
"symop y+1/2,x+1/2,-z+1/2\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop -y,-x,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop y+1/2,-x,-z\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -y,x+1/2,-z\n"\
"symop -x+1/2,y,z+1/2\n"\
"symop -y+1/2,-x+1/2,z+1/2\n"\
"symop x,-y+1/2,z+1/2\n"\
"symop y,x,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  131\n"\
"basisop x,y,z\n"\
"symbol ccp4 131\n"\
"symbol Hall '-P 4c 2'\n"\
"symbol xHM  'P 42/m m c'\n"\
"symbol old  'P 42/m 2/m 2/c' 'P42/m m c'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/4\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x,z+1/2\n"\
"symop -x,-y,z\n"\
"symop y,-x,z+1/2\n"\
"symop x,-y,-z\n"\
"symop y,x,-z+1/2\n"\
"symop -x,y,-z\n"\
"symop -y,-x,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop y,-x,-z+1/2\n"\
"symop x,y,-z\n"\
"symop -y,x,-z+1/2\n"\
"symop -x,y,z\n"\
"symop -y,-x,z+1/2\n"\
"symop x,-y,z\n"\
"symop y,x,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  132\n"\
"basisop x,y,z\n"\
"symbol ccp4 132\n"\
"symbol Hall '-P 4c 2c'\n"\
"symbol xHM  'P 42/m c m'\n"\
"symbol old  'P 42/m 2/c 2/m' 'P42/m c m'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x,z+1/2\n"\
"symop -x,-y,z\n"\
"symop y,-x,z+1/2\n"\
"symop x,-y,-z+1/2\n"\
"symop y,x,-z\n"\
"symop -x,y,-z+1/2\n"\
"symop -y,-x,-z\n"\
"symop -x,-y,-z\n"\
"symop y,-x,-z+1/2\n"\
"symop x,y,-z\n"\
"symop -y,x,-z+1/2\n"\
"symop -x,y,z+1/2\n"\
"symop -y,-x,z\n"\
"symop x,-y,z+1/2\n"\
"symop y,x,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  133\n"\
"basisop x-1/4,y+1/4,z+1/4\n"\
"symbol ccp4 133\n"\
"symbol Hall '-P 4ac 2b (x-1/4,y+1/4,z+1/4)'\n"\
"symbol xHM  'P 42/n b c :1'\n"\
"symbol old  'P 42/n 2/b 2/c' 'P42/n b c'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/2,x+1/2,z+1/2\n"\
"symop -x,-y,z\n"\
"symop y+1/2,-x+1/2,z+1/2\n"\
"symop x,-y,-z+1/2\n"\
"symop y+1/2,x+1/2,-z\n"\
"symop -x,y,-z+1/2\n"\
"symop -y+1/2,-x+1/2,-z\n"\
"symop -x+1/2,-y+1/2,-z+1/2\n"\
"symop y,-x,-z\n"\
"symop x+1/2,y+1/2,-z+1/2\n"\
"symop -y,x,-z\n"\
"symop -x+1/2,y+1/2,z\n"\
"symop -y,-x,z+1/2\n"\
"symop x+1/2,-y+1/2,z\n"\
"symop y,x,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  133\n"\
"basisop x,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 4ac 2b'\n"\
"symbol xHM  'P 42/n b c :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<=3/4; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/2,x,z+1/2\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop y,-x+1/2,z+1/2\n"\
"symop x,-y+1/2,-z\n"\
"symop y,x,-z+1/2\n"\
"symop -x+1/2,y,-z\n"\
"symop -y+1/2,-x+1/2,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop y+1/2,-x,-z+1/2\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -y,x+1/2,-z+1/2\n"\
"symop -x,y+1/2,z\n"\
"symop -y,-x,z+1/2\n"\
"symop x+1/2,-y,z\n"\
"symop y+1/2,x+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  134\n"\
"basisop x-1/4,y+1/4,z-1/4\n"\
"symbol ccp4 134\n"\
"symbol Hall '-P 4ac 2bc (x-1/4,y+1/4,z-1/4)'\n"\
"symbol xHM  'P 42/n n m :1'\n"\
"symbol old  'P 42/n 2/n 2/m' 'P42/n n m'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/2,x+1/2,z+1/2\n"\
"symop -x,-y,z\n"\
"symop y+1/2,-x+1/2,z+1/2\n"\
"symop x,-y,-z\n"\
"symop y+1/2,x+1/2,-z+1/2\n"\
"symop -x,y,-z\n"\
"symop -y+1/2,-x+1/2,-z+1/2\n"\
"symop -x+1/2,-y+1/2,-z+1/2\n"\
"symop y,-x,-z\n"\
"symop x+1/2,y+1/2,-z+1/2\n"\
"symop -y,x,-z\n"\
"symop -x+1/2,y+1/2,z+1/2\n"\
"symop -y,-x,z\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"symop y,x,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  134\n"\
"basisop x,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 4ac 2bc'\n"\
"symbol xHM  'P 42/n n m :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<=3/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/2,x,z+1/2\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop y,-x+1/2,z+1/2\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop y,x,-z\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop -y+1/2,-x+1/2,-z\n"\
"symop -x,-y,-z\n"\
"symop y+1/2,-x,-z+1/2\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -y,x+1/2,-z+1/2\n"\
"symop -x,y+1/2,z+1/2\n"\
"symop -y,-x,z\n"\
"symop x+1/2,-y,z+1/2\n"\
"symop y+1/2,x+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  135\n"\
"basisop x,y,z\n"\
"symbol ccp4 135\n"\
"symbol Hall '-P 4c 2ab'\n"\
"symbol xHM  'P 42/m b c'\n"\
"symbol old  'P 42/m 21/b 2/c' 'P42/m b c'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/4; 0<=z<=1/4\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x,z+1/2\n"\
"symop -x,-y,z\n"\
"symop y,-x,z+1/2\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop y+1/2,x+1/2,-z+1/2\n"\
"symop -x+1/2,y+1/2,-z\n"\
"symop -y+1/2,-x+1/2,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop y,-x,-z+1/2\n"\
"symop x,y,-z\n"\
"symop -y,x,-z+1/2\n"\
"symop -x+1/2,y+1/2,z\n"\
"symop -y+1/2,-x+1/2,z+1/2\n"\
"symop x+1/2,-y+1/2,z\n"\
"symop y+1/2,x+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  136\n"\
"basisop x,y,z\n"\
"symbol ccp4 136\n"\
"symbol Hall '-P 4n 2n'\n"\
"symbol xHM  'P 42/m n m'\n"\
"symbol old  'P 42/m 21/n 2/m' 'P42/m n m'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<3/4; 1/4<=y<=1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/2,x+1/2,z+1/2\n"\
"symop -x,-y,z\n"\
"symop y+1/2,-x+1/2,z+1/2\n"\
"symop x+1/2,-y+1/2,-z+1/2\n"\
"symop y,x,-z\n"\
"symop -x+1/2,y+1/2,-z+1/2\n"\
"symop -y,-x,-z\n"\
"symop -x,-y,-z\n"\
"symop y+1/2,-x+1/2,-z+1/2\n"\
"symop x,y,-z\n"\
"symop -y+1/2,x+1/2,-z+1/2\n"\
"symop -x+1/2,y+1/2,z+1/2\n"\
"symop -y,-x,z\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"symop y,x,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  137\n"\
"basisop x-1/4,y+1/4,z+1/4\n"\
"symbol ccp4 137\n"\
"symbol Hall '-P 4ac 2a (x-1/4,y+1/4,z+1/4)'\n"\
"symbol xHM  'P 42/n m c :1'\n"\
"symbol old  'P 42/n 21/m 2/c' 'P42/n m c'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/2,x+1/2,z+1/2\n"\
"symop -x,-y,z\n"\
"symop y+1/2,-x+1/2,z+1/2\n"\
"symop x+1/2,-y+1/2,-z+1/2\n"\
"symop y,x,-z\n"\
"symop -x+1/2,y+1/2,-z+1/2\n"\
"symop -y,-x,-z\n"\
"symop -x+1/2,-y+1/2,-z+1/2\n"\
"symop y,-x,-z\n"\
"symop x+1/2,y+1/2,-z+1/2\n"\
"symop -y,x,-z\n"\
"symop -x,y,z\n"\
"symop -y+1/2,-x+1/2,z+1/2\n"\
"symop x,-y,z\n"\
"symop y+1/2,x+1/2,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  137\n"\
"basisop x,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 4ac 2a'\n"\
"symbol xHM  'P 42/n m c :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 1/4<=y<=3/4; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/2,x,z+1/2\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop y,-x+1/2,z+1/2\n"\
"symop x+1/2,-y,-z\n"\
"symop y+1/2,x+1/2,-z+1/2\n"\
"symop -x,y+1/2,-z\n"\
"symop -y,-x,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop y+1/2,-x,-z+1/2\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -y,x+1/2,-z+1/2\n"\
"symop -x+1/2,y,z\n"\
"symop -y+1/2,-x+1/2,z+1/2\n"\
"symop x,-y+1/2,z\n"\
"symop y,x,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  138\n"\
"basisop x-1/4,y+1/4,z-1/4\n"\
"symbol ccp4 138\n"\
"symbol Hall '-P 4ac 2ac (x-1/4,y+1/4,z-1/4)'\n"\
"symbol xHM  'P 42/n c m :1'\n"\
"symbol old  'P 42/n 21/c 2/m' 'P42/n c m'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<3/4; 0<=y<=1/4; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/2,x+1/2,z+1/2\n"\
"symop -x,-y,z\n"\
"symop y+1/2,-x+1/2,z+1/2\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop y,x,-z+1/2\n"\
"symop -x+1/2,y+1/2,-z\n"\
"symop -y,-x,-z+1/2\n"\
"symop -x+1/2,-y+1/2,-z+1/2\n"\
"symop y,-x,-z\n"\
"symop x+1/2,y+1/2,-z+1/2\n"\
"symop -y,x,-z\n"\
"symop -x,y,z+1/2\n"\
"symop -y+1/2,-x+1/2,z\n"\
"symop x,-y,z+1/2\n"\
"symop y+1/2,x+1/2,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  138\n"\
"basisop x,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 4ac 2ac'\n"\
"symbol xHM  'P 42/n c m :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1/2\n"\
"mapasu nonz 0<x<=3/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/2,x,z+1/2\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop y,-x+1/2,z+1/2\n"\
"symop x+1/2,-y,-z+1/2\n"\
"symop y+1/2,x+1/2,-z\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop -y,-x,-z\n"\
"symop -x,-y,-z\n"\
"symop y+1/2,-x,-z+1/2\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -y,x+1/2,-z+1/2\n"\
"symop -x+1/2,y,z+1/2\n"\
"symop -y+1/2,-x+1/2,z\n"\
"symop x,-y+1/2,z+1/2\n"\
"symop y,x,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  139\n"\
"basisop x,y,z\n"\
"symbol ccp4 139\n"\
"symbol Hall '-I 4 2'\n"\
"symbol xHM  'I 4/m m m'\n"\
"symbol old  'I 4/m 2/m 2/m' 'I4/m m m'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-I 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/4\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop x,-y,-z\n"\
"symop y,x,-z\n"\
"symop -x,y,-z\n"\
"symop -y,-x,-z\n"\
"symop -x,-y,-z\n"\
"symop y,-x,-z\n"\
"symop x,y,-z\n"\
"symop -y,x,-z\n"\
"symop -x,y,z\n"\
"symop -y,-x,z\n"\
"symop x,-y,z\n"\
"symop y,x,z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  140\n"\
"basisop x,y,z\n"\
"symbol ccp4 140\n"\
"symbol Hall '-I 4 2c'\n"\
"symbol xHM  'I 4/m c m'\n"\
"symbol old  'I 4/m 2/c 2/m' 'I4/m c m'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-I 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<3/4; 0<=y<=1/4; 0<=z<=1/4\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop x,-y,-z+1/2\n"\
"symop y,x,-z+1/2\n"\
"symop -x,y,-z+1/2\n"\
"symop -y,-x,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop y,-x,-z\n"\
"symop x,y,-z\n"\
"symop -y,x,-z\n"\
"symop -x,y,z+1/2\n"\
"symop -y,-x,z+1/2\n"\
"symop x,-y,z+1/2\n"\
"symop y,x,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  141\n"\
"basisop x-1/2,y+1/4,z+1/8\n"\
"symbol ccp4 141\n"\
"symbol Hall '-I 4bd 2 (x-1/2,y+1/4,z+1/8)'\n"\
"symbol xHM  'I 41/a m d :1'\n"\
"symbol old  'I 41/a 2/m 2/d' 'I41/a m d'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-I 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x+1/2,z+1/4\n"\
"symop -x+1/2,-y+1/2,z+1/2\n"\
"symop y+1/2,-x,z+3/4\n"\
"symop x,-y+1/2,-z+1/4\n"\
"symop y+1/2,x+1/2,-z+1/2\n"\
"symop -x+1/2,y,-z+3/4\n"\
"symop -y,-x,-z\n"\
"symop -x,-y+1/2,-z+1/4\n"\
"symop y,-x,-z\n"\
"symop x+1/2,y,-z+3/4\n"\
"symop -y+1/2,x+1/2,-z+1/2\n"\
"symop -x,y,z\n"\
"symop -y+1/2,-x,z+3/4\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"symop y,x+1/2,z+1/4\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  141\n"\
"basisop x,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-I 4bd 2'\n"\
"symbol xHM  'I 41/a m d :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-I 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; -1/8<=z<=3/8\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/4,x+3/4,z+1/4\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop y+1/4,-x+1/4,z+3/4\n"\
"symop x,-y,-z\n"\
"symop y+1/4,x+3/4,-z+1/4\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop -y+1/4,-x+1/4,-z+3/4\n"\
"symop -x,-y,-z\n"\
"symop y+3/4,-x+1/4,-z+3/4\n"\
"symop x+1/2,y,-z+1/2\n"\
"symop -y+3/4,x+3/4,-z+1/4\n"\
"symop -x,y,z\n"\
"symop -y+3/4,-x+1/4,z+3/4\n"\
"symop x+1/2,-y,z+1/2\n"\
"symop y+3/4,x+3/4,z+1/4\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  142\n"\
"basisop x-1/2,y+1/4,z-3/8\n"\
"symbol ccp4 142\n"\
"symbol Hall '-I 4bd 2c (x-1/2,y+1/4,z-3/8)'\n"\
"symbol xHM  'I 41/a c d :1'\n"\
"symbol old  'I41/a c d' 'I 41/a 2/c 2/d'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-I 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/8\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x+1/2,z+1/4\n"\
"symop -x+1/2,-y+1/2,z+1/2\n"\
"symop y+1/2,-x,z+3/4\n"\
"symop x,-y+1/2,-z+3/4\n"\
"symop y+1/2,x+1/2,-z\n"\
"symop -x+1/2,y,-z+1/4\n"\
"symop -y,-x,-z+1/2\n"\
"symop -x,-y+1/2,-z+1/4\n"\
"symop y,-x,-z\n"\
"symop x+1/2,y,-z+3/4\n"\
"symop -y+1/2,x+1/2,-z+1/2\n"\
"symop -x,y,z+1/2\n"\
"symop -y+1/2,-x,z+1/4\n"\
"symop x+1/2,-y+1/2,z\n"\
"symop y,x+1/2,z+3/4\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  142\n"\
"basisop x,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-I 4bd 2c'\n"\
"symbol xHM  'I 41/a c d :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-I 4 2' '4/mmm'\n"\
"symbol pgrp '-P 4 2' '4/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/8\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 1/8<=z<=5/8\n"\
"cheshire 0<=x<=1; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/4,x+3/4,z+1/4\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop y+1/4,-x+1/4,z+3/4\n"\
"symop x,-y,-z+1/2\n"\
"symop y+1/4,x+3/4,-z+3/4\n"\
"symop -x+1/2,y,-z\n"\
"symop -y+1/4,-x+1/4,-z+1/4\n"\
"symop -x,-y,-z\n"\
"symop y+3/4,-x+1/4,-z+3/4\n"\
"symop x+1/2,y,-z+1/2\n"\
"symop -y+3/4,x+3/4,-z+1/4\n"\
"symop -x,y,z+1/2\n"\
"symop -y+3/4,-x+1/4,z+1/4\n"\
"symop x+1/2,-y,z\n"\
"symop y+3/4,x+3/4,z+3/4\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  143\n"\
"basisop x,y,z\n"\
"symbol ccp4 143\n"\
"symbol Hall ' P 3'\n"\
"symbol xHM  'P 3'\n"\
"symbol old  'P 3'\n"\
"symbol laue '-P 3' '-3'\n"\
"symbol patt '-P 3' '-3'\n"\
"symbol pgrp ' P 3' '3'\n"\
"hklasu ccp4 '(h>=0 and k>0) or (h=0 and k=0 and l>=0)'\n"\
"mapasu ccp4 0<=x<=2/3; 0<=y<=2/3; 0<=z<1\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=2/3; 0<=z<1\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=2/3; 0<=z<1\n"\
"cheshire 0<=x<=2/3; 0<=y<=2/3; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x-y,z\n"\
"symop -x+y,-x,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  144\n"\
"basisop x,y,z\n"\
"symbol ccp4 144\n"\
"symbol Hall ' P 31'\n"\
"symbol xHM  'P 31'\n"\
"symbol old  'P 31'\n"\
"symbol laue '-P 3' '-3'\n"\
"symbol patt '-P 3' '-3'\n"\
"symbol pgrp ' P 3' '3'\n"\
"hklasu ccp4 '(h>=0 and k>0) or (h=0 and k=0 and l>=0)'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<1/3\n"\
"mapasu zero 0<=x<1; 0<=y<1; 0<=z<1/3\n"\
"mapasu nonz 0<=x<1; 0<=y<1; 0<=z<1/3\n"\
"cheshire 0<=x<=2/3; 0<=y<=2/3; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x-y,z+1/3\n"\
"symop -x+y,-x,z+2/3\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  145\n"\
"basisop x,y,z\n"\
"symbol ccp4 145\n"\
"symbol Hall ' P 32'\n"\
"symbol xHM  'P 32'\n"\
"symbol old  'P 32'\n"\
"symbol laue '-P 3' '-3'\n"\
"symbol patt '-P 3' '-3'\n"\
"symbol pgrp ' P 3' '3'\n"\
"hklasu ccp4 '(h>=0 and k>0) or (h=0 and k=0 and l>=0)'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<1/3\n"\
"mapasu zero 0<=x<1; 0<=y<1; 0<=z<1/3\n"\
"mapasu nonz 0<=x<1; 0<=y<1; 0<=z<1/3\n"\
"cheshire 0<=x<=2/3; 0<=y<=2/3; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x-y,z+2/3\n"\
"symop -x+y,-x,z+1/3\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  146\n"\
"basisop x,y,z\n"\
"symbol ccp4 146\n"\
"symbol Hall ' R 3'\n"\
"symbol xHM  'R 3 :H'\n"\
"symbol old  'H 3'\n"\
"symbol laue '-P 3' '-3'\n"\
"symbol patt '-R 3' '-3'\n"\
"symbol pgrp ' P 3' '3'\n"\
"hklasu ccp4 '(h>=0 and k>0) or (h=0 and k=0 and l>=0)'\n"\
"mapasu ccp4 0<=x<=2/3; 0<=y<=2/3; 0<=z<1/3\n"\
"mapasu zero 0<=x<=1/3; 0<=y<=1/3; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/3; 0<=y<=1/3; 0<=z<1\n"\
"cheshire 0<=x<=2/3; 0<=y<=2/3; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x-y,z\n"\
"symop -x+y,-x,z\n"\
"cenop x,y,z\n"\
"cenop x+2/3,y+1/3,z+1/3\n"\
"cenop x+1/3,y+2/3,z+2/3\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  146\n"\
"basisop -y+z,x+z,-x+y+z\n"\
"symbol ccp4 1146\n"\
"symbol Hall ' R 3 (-y+z,x+z,-x+y+z)'\n"\
"symbol xHM  'R 3 :R'\n"\
"symbol old  'R 3'\n"\
"symbol laue '-P 3*' '-3'\n"\
"symbol patt '-P 3*' '-3'\n"\
"symbol pgrp ' P 3*' '3'\n"\
"hklasu ccp4 '(h>=0 and k>0) or (h=0 and k=0 and l>=0)'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<1; 0<=z<1\n"\
"cheshire 0<=x<=2/3; 0<=y<=2/3; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop z,x,y\n"\
"symop y,z,x\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  147\n"\
"basisop x,y,z\n"\
"symbol ccp4 147\n"\
"symbol Hall '-P 3'\n"\
"symbol xHM  'P -3'\n"\
"symbol old  'P -3'\n"\
"symbol laue '-P 3' '-3'\n"\
"symbol patt '-P 3' '-3'\n"\
"symbol pgrp '-P 3' '-3'\n"\
"hklasu ccp4 '(h>=0 and k>0) or (h=0 and k=0 and l>=0)'\n"\
"mapasu ccp4 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=1/3; 0<=z<1\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=1/3; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x-y,z\n"\
"symop -x+y,-x,z\n"\
"symop -x,-y,-z\n"\
"symop y,-x+y,-z\n"\
"symop x-y,x,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  148\n"\
"basisop x,y,z\n"\
"symbol ccp4 148\n"\
"symbol Hall '-R 3'\n"\
"symbol xHM  'R -3 :H'\n"\
"symbol old  'H -3'\n"\
"symbol laue '-P 3' '-3'\n"\
"symbol patt '-R 3' '-3'\n"\
"symbol pgrp '-P 3' '-3'\n"\
"hklasu ccp4 '(h>=0 and k>0) or (h=0 and k=0 and l>=0)'\n"\
"mapasu ccp4 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/6\n"\
"mapasu zero 0<=x<=1/3; 0<=y<=1/3; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/3; -1/6<=y<=0; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x-y,z\n"\
"symop -x+y,-x,z\n"\
"symop -x,-y,-z\n"\
"symop y,-x+y,-z\n"\
"symop x-y,x,-z\n"\
"cenop x,y,z\n"\
"cenop x+2/3,y+1/3,z+1/3\n"\
"cenop x+1/3,y+2/3,z+2/3\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  148\n"\
"basisop -y+z,x+z,-x+y+z\n"\
"symbol ccp4 1148\n"\
"symbol Hall '-R 3 (-y+z,x+z,-x+y+z)'\n"\
"symbol xHM  'R -3 :R'\n"\
"symbol old  'R -3'\n"\
"symbol laue '-P 3*' '-3'\n"\
"symbol patt '-P 3*' '-3'\n"\
"symbol pgrp '-P 3*' '-3'\n"\
"hklasu ccp4 '(h>=0 and k>0) or (h=0 and k=0 and l>=0)'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop z,x,y\n"\
"symop y,z,x\n"\
"symop -x,-y,-z\n"\
"symop -z,-x,-y\n"\
"symop -y,-z,-x\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  149\n"\
"basisop x,y,z\n"\
"symbol ccp4 149\n"\
"symbol Hall ' P 3 2'\n"\
"symbol xHM  'P 3 1 2'\n"\
"symbol old  'P 3 1 2'\n"\
"symbol laue '-P 3 2' '-31m'\n"\
"symbol patt '-P 3 2' '-31m'\n"\
"symbol pgrp ' P 3 2' '312'\n"\
"hklasu ccp4 'h>=k and k>=0 and (k>0 or l>=0)'\n"\
"mapasu ccp4 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/2\n"\
"cheshire 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x-y,z\n"\
"symop -x+y,-x,z\n"\
"symop -y,-x,-z\n"\
"symop x,x-y,-z\n"\
"symop -x+y,y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  150\n"\
"basisop x,y,z\n"\
"symbol ccp4 150\n"\
"symbol Hall ' P 3 2\"'\n"\
"symbol xHM  'P 3 2 1'\n"\
"symbol old  'P 3 2 1'\n"\
"symbol laue '-P 3 2\"' '-3m1'\n"\
"symbol patt '-P 3 2\"' '-3m1'\n"\
"symbol pgrp ' P 3 2\"' '321'\n"\
"hklasu ccp4 'h>=k and k>=0 and (h>k or l>=0)'\n"\
"mapasu ccp4 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=1/3; 0<=z<1\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=1/3; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x-y,z\n"\
"symop -x+y,-x,z\n"\
"symop y,x,-z\n"\
"symop -x,-x+y,-z\n"\
"symop x-y,-y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  151\n"\
"basisop x,y,z\n"\
"symbol ccp4 151\n"\
"symbol Hall ' P 31 2 (x,y,z+1/3)'\n"\
"symbol xHM  'P 31 1 2'\n"\
"symbol old  'P 31 1 2'\n"\
"symbol laue '-P 3 2' '-31m'\n"\
"symbol patt '-P 3 2' '-31m'\n"\
"symbol pgrp ' P 3 2' '312'\n"\
"hklasu ccp4 'h>=k and k>=0 and (k>0 or l>=0)'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<=1/6\n"\
"mapasu zero 0<=x<1; 0<=y<1; 0<=z<=1/6\n"\
"mapasu nonz 0<=x<1; 0<=y<1; 0<=z<=1/6\n"\
"cheshire 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x-y,z+1/3\n"\
"symop -x+y,-x,z+2/3\n"\
"symop -y,-x,-z+2/3\n"\
"symop x,x-y,-z\n"\
"symop -x+y,y,-z+1/3\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  152\n"\
"basisop x,y,z\n"\
"symbol ccp4 152\n"\
"symbol Hall ' P 31 2\"'\n"\
"symbol xHM  'P 31 2 1'\n"\
"symbol old  'P 31 2 1'\n"\
"symbol laue '-P 3 2\"' '-3m1'\n"\
"symbol patt '-P 3 2\"' '-3m1'\n"\
"symbol pgrp ' P 3 2\"' '321'\n"\
"hklasu ccp4 'h>=k and k>=0 and (h>k or l>=0)'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<=1/6\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1; 0<=z<=1/3\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1; 0<=z<=1/3\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x-y,z+1/3\n"\
"symop -x+y,-x,z+2/3\n"\
"symop y,x,-z\n"\
"symop -x,-x+y,-z+1/3\n"\
"symop x-y,-y,-z+2/3\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  153\n"\
"basisop x,y,z\n"\
"symbol ccp4 153\n"\
"symbol Hall ' P 32 2 (x,y,z+1/6)'\n"\
"symbol xHM  'P 32 1 2'\n"\
"symbol old  'P 32 1 2'\n"\
"symbol laue '-P 3 2' '-31m'\n"\
"symbol patt '-P 3 2' '-31m'\n"\
"symbol pgrp ' P 3 2' '312'\n"\
"hklasu ccp4 'h>=k and k>=0 and (k>0 or l>=0)'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<=1/6\n"\
"mapasu zero 0<=x<1; 0<=y<1; 0<=z<=1/6\n"\
"mapasu nonz 0<=x<1; 0<=y<1; 0<=z<=1/6\n"\
"cheshire 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x-y,z+2/3\n"\
"symop -x+y,-x,z+1/3\n"\
"symop -y,-x,-z+1/3\n"\
"symop x,x-y,-z\n"\
"symop -x+y,y,-z+2/3\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  154\n"\
"basisop x,y,z\n"\
"symbol ccp4 154\n"\
"symbol Hall ' P 32 2\"'\n"\
"symbol xHM  'P 32 2 1'\n"\
"symbol old  'P 32 2 1'\n"\
"symbol laue '-P 3 2\"' '-3m1'\n"\
"symbol patt '-P 3 2\"' '-3m1'\n"\
"symbol pgrp ' P 3 2\"' '321'\n"\
"hklasu ccp4 'h>=k and k>=0 and (h>k or l>=0)'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<=1/6\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<=1/3\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/2; 0<=z<=1/3\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x-y,z+2/3\n"\
"symop -x+y,-x,z+1/3\n"\
"symop y,x,-z\n"\
"symop -x,-x+y,-z+2/3\n"\
"symop x-y,-y,-z+1/3\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  155\n"\
"basisop x,y,z\n"\
"symbol ccp4 155\n"\
"symbol Hall ' R 3 2\"'\n"\
"symbol xHM  'R 3 2 :H'\n"\
"symbol old  'H 3 2'\n"\
"symbol laue '-P 3 2\"' '-3m1'\n"\
"symbol patt '-R 3 2\"' '-3m'\n"\
"symbol pgrp ' P 3 2\"' '321'\n"\
"hklasu ccp4 'h>=k and k>=0 and (h>k or l>=0)'\n"\
"mapasu ccp4 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/6\n"\
"mapasu zero 0<=x<=1/3; 0<=y<=1/3; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/3; 0<=y<=1/3; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x-y,z\n"\
"symop -x+y,-x,z\n"\
"symop y,x,-z\n"\
"symop -x,-x+y,-z\n"\
"symop x-y,-y,-z\n"\
"cenop x,y,z\n"\
"cenop x+2/3,y+1/3,z+1/3\n"\
"cenop x+1/3,y+2/3,z+2/3\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  155\n"\
"basisop -y+z,x+z,-x+y+z\n"\
"symbol ccp4 1155\n"\
"symbol Hall ' R 3 2\" (-y+z,x+z,-x+y+z)'\n"\
"symbol xHM  'R 3 2 :R'\n"\
"symbol old  'R 3 2'\n"\
"symbol laue '-P 3* 2' '-3m'\n"\
"symbol patt '-P 3* 2' '-3m'\n"\
"symbol pgrp ' P 3* 2' '32'\n"\
"hklasu ccp4 'h>=k and k>=0 and (h>k or l>=0)'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop z,x,y\n"\
"symop y,z,x\n"\
"symop -y,-x,-z\n"\
"symop -z,-y,-x\n"\
"symop -x,-z,-y\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  156\n"\
"basisop x,y,z\n"\
"symbol ccp4 156\n"\
"symbol Hall ' P 3 -2\"'\n"\
"symbol xHM  'P 3 m 1'\n"\
"symbol old  'P 3 m 1'\n"\
"symbol laue '-P 3 2\"' '-3m1'\n"\
"symbol patt '-P 3 2\"' '-3m1'\n"\
"symbol pgrp ' P 3 -2\"' '3m1'\n"\
"hklasu ccp4 'h>=k and k>=0 and (h>k or l>=0)'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=2/3; 0<=z<1\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=2/3; 0<=z<1\n"\
"cheshire 0<=x<=2/3; 0<=y<=2/3; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x-y,z\n"\
"symop -x+y,-x,z\n"\
"symop -y,-x,z\n"\
"symop x,x-y,z\n"\
"symop -x+y,y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  157\n"\
"basisop x,y,z\n"\
"symbol ccp4 157\n"\
"symbol Hall ' P 3 -2'\n"\
"symbol xHM  'P 3 1 m'\n"\
"symbol old  'P 3 1 m'\n"\
"symbol laue '-P 3 2' '-31m'\n"\
"symbol patt '-P 3 2' '-31m'\n"\
"symbol pgrp ' P 3 -2' '31m'\n"\
"hklasu ccp4 'h>=k and k>=0 and (k>0 or l>=0)'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=1/3; 0<=z<1\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=1/3; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x-y,z\n"\
"symop -x+y,-x,z\n"\
"symop y,x,z\n"\
"symop -x,-x+y,z\n"\
"symop x-y,-y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  158\n"\
"basisop x,y,z\n"\
"symbol ccp4 158\n"\
"symbol Hall ' P 3 -2\"c'\n"\
"symbol xHM  'P 3 c 1'\n"\
"symbol old  'P 3 c 1'\n"\
"symbol laue '-P 3 2\"' '-3m1'\n"\
"symbol patt '-P 3 2\"' '-3m1'\n"\
"symbol pgrp ' P 3 -2\"' '3m1'\n"\
"hklasu ccp4 'h>=k and k>=0 and (h>k or l>=0)'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=2/3; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=2/3; 0<=z<1/2\n"\
"cheshire 0<=x<=2/3; 0<=y<=2/3; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x-y,z\n"\
"symop -x+y,-x,z\n"\
"symop -y,-x,z+1/2\n"\
"symop x,x-y,z+1/2\n"\
"symop -x+y,y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  159\n"\
"basisop x,y,z\n"\
"symbol ccp4 159\n"\
"symbol Hall ' P 3 -2c'\n"\
"symbol xHM  'P 3 1 c'\n"\
"symbol old  'P 3 1 c'\n"\
"symbol laue '-P 3 2' '-31m'\n"\
"symbol patt '-P 3 2' '-31m'\n"\
"symbol pgrp ' P 3 -2' '31m'\n"\
"hklasu ccp4 'h>=k and k>=0 and (k>0 or l>=0)'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=1/3; 0<=z<1\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=1/3; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x-y,z\n"\
"symop -x+y,-x,z\n"\
"symop y,x,z+1/2\n"\
"symop -x,-x+y,z+1/2\n"\
"symop x-y,-y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  160\n"\
"basisop x,y,z\n"\
"symbol ccp4 160\n"\
"symbol Hall ' R 3 -2\"'\n"\
"symbol xHM  'R 3 m :H'\n"\
"symbol old  'H 3 m'\n"\
"symbol laue '-P 3 2\"' '-3m1'\n"\
"symbol patt '-R 3 2\"' '-3m'\n"\
"symbol pgrp ' P 3 -2\"' '3m1'\n"\
"hklasu ccp4 'h>=k and k>=0 and (h>k or l>=0)'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/3; 0<=y<1/3; 0<=z<1\n"\
"mapasu nonz 0<=x<=5/12; 0<=y<1/4; 0<=z<1\n"\
"cheshire 0<=x<=2/3; 0<=y<=2/3; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x-y,z\n"\
"symop -x+y,-x,z\n"\
"symop -y,-x,z\n"\
"symop x,x-y,z\n"\
"symop -x+y,y,z\n"\
"cenop x,y,z\n"\
"cenop x+2/3,y+1/3,z+1/3\n"\
"cenop x+1/3,y+2/3,z+2/3\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  160\n"\
"basisop -y+z,x+z,-x+y+z\n"\
"symbol ccp4 1160\n"\
"symbol Hall ' R 3 -2\" (-y+z,x+z,-x+y+z)'\n"\
"symbol xHM  'R 3 m :R'\n"\
"symbol old  'R 3 m'\n"\
"symbol laue '-P 3* 2' '-3m'\n"\
"symbol patt '-P 3* 2' '-3m'\n"\
"symbol pgrp ' P 3* -2' '3m'\n"\
"hklasu ccp4 'h>=k and k>=0 and (h>k or l>=0)'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<1; 0<=z<1\n"\
"mapasu nonz 0<=x<1; 0<=y<1; 0<=z<1\n"\
"cheshire 0<=x<=2/3; 0<=y<=2/3; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop z,x,y\n"\
"symop y,z,x\n"\
"symop y,x,z\n"\
"symop z,y,x\n"\
"symop x,z,y\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  161\n"\
"basisop x,y,z\n"\
"symbol ccp4 161\n"\
"symbol Hall ' R 3 -2\"c'\n"\
"symbol xHM  'R 3 c :H'\n"\
"symbol old  'H 3 c'\n"\
"symbol laue '-P 3 2\"' '-3m1'\n"\
"symbol patt '-R 3 2\"' '-3m'\n"\
"symbol pgrp ' P 3 -2\"' '3m1'\n"\
"hklasu ccp4 'h>=k and k>=0 and (h>k or l>=0)'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/3; 0<=y<1/3; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/3; 0<=y<1/3; 0<=z<1/2\n"\
"cheshire 0<=x<=2/3; 0<=y<=2/3; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop -y,x-y,z\n"\
"symop -x+y,-x,z\n"\
"symop -y,-x,z+1/2\n"\
"symop x,x-y,z+1/2\n"\
"symop -x+y,y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+2/3,y+1/3,z+1/3\n"\
"cenop x+1/3,y+2/3,z+2/3\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  161\n"\
"basisop -y+z,x+z,-x+y+z\n"\
"symbol ccp4 1161\n"\
"symbol Hall ' R 3 -2\"c (-y+z,x+z,-x+y+z)'\n"\
"symbol xHM  'R 3 c :R'\n"\
"symbol old  'R 3 c'\n"\
"symbol laue '-P 3* 2' '-3m'\n"\
"symbol patt '-P 3* 2' '-3m'\n"\
"symbol pgrp ' P 3* -2' '3m'\n"\
"hklasu ccp4 'h>=k and k>=0 and (h>k or l>=0)'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<1/2; 0<=y<1/2; 0<=z<1\n"\
"cheshire 0<=x<=2/3; 0<=y<=2/3; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop z,x,y\n"\
"symop y,z,x\n"\
"symop y+1/2,x+1/2,z+1/2\n"\
"symop z+1/2,y+1/2,x+1/2\n"\
"symop x+1/2,z+1/2,y+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  162\n"\
"basisop x,y,z\n"\
"symbol ccp4 162\n"\
"symbol Hall '-P 3 2'\n"\
"symbol xHM  'P -3 1 m'\n"\
"symbol old  'P -3 1 2/m' 'P -3 1 m'\n"\
"symbol laue '-P 3 2' '-31m'\n"\
"symbol patt '-P 3 2' '-31m'\n"\
"symbol pgrp '-P 3 2' '-31m'\n"\
"hklasu ccp4 'h>=k and k>=0 and (k>0 or l>=0)'\n"\
"mapasu ccp4 0<=x<=2/3; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=1/3; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=1/3; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x-y,z\n"\
"symop -x+y,-x,z\n"\
"symop -y,-x,-z\n"\
"symop x,x-y,-z\n"\
"symop -x+y,y,-z\n"\
"symop -x,-y,-z\n"\
"symop y,-x+y,-z\n"\
"symop x-y,x,-z\n"\
"symop y,x,z\n"\
"symop -x,-x+y,z\n"\
"symop x-y,-y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  163\n"\
"basisop x,y,z\n"\
"symbol ccp4 163\n"\
"symbol Hall '-P 3 2c'\n"\
"symbol xHM  'P -3 1 c'\n"\
"symbol old  'P -3 1 2/c' 'P -3 1 c'\n"\
"symbol laue '-P 3 2' '-31m'\n"\
"symbol patt '-P 3 2' '-31m'\n"\
"symbol pgrp '-P 3 2' '-31m'\n"\
"hklasu ccp4 'h>=k and k>=0 and (k>0 or l>=0)'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=2/3; 0<=z<1/3\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=1/3; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x-y,z\n"\
"symop -x+y,-x,z\n"\
"symop -y,-x,-z+1/2\n"\
"symop x,x-y,-z+1/2\n"\
"symop -x+y,y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop y,-x+y,-z\n"\
"symop x-y,x,-z\n"\
"symop y,x,z+1/2\n"\
"symop -x,-x+y,z+1/2\n"\
"symop x-y,-y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  164\n"\
"basisop x,y,z\n"\
"symbol ccp4 164\n"\
"symbol Hall '-P 3 2\"'\n"\
"symbol xHM  'P -3 m 1'\n"\
"symbol old  'P -3 2/m 1' 'P -3 m 1'\n"\
"symbol laue '-P 3 2\"' '-3m1'\n"\
"symbol patt '-P 3 2\"' '-3m1'\n"\
"symbol pgrp '-P 3 2\"' '-3m1'\n"\
"hklasu ccp4 'h>=k and k>=0 and (h>k or l>=0)'\n"\
"mapasu ccp4 0<=x<=2/3; 0<=y<=1/3; 0<=z<=1\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=1/3; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; -1/3<=y<=0; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x-y,z\n"\
"symop -x+y,-x,z\n"\
"symop y,x,-z\n"\
"symop -x,-x+y,-z\n"\
"symop x-y,-y,-z\n"\
"symop -x,-y,-z\n"\
"symop y,-x+y,-z\n"\
"symop x-y,x,-z\n"\
"symop -y,-x,z\n"\
"symop x,x-y,z\n"\
"symop -x+y,y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  165\n"\
"basisop x,y,z\n"\
"symbol ccp4 165\n"\
"symbol Hall '-P 3 2\"c'\n"\
"symbol xHM  'P -3 c 1'\n"\
"symbol old  'P -3 2/c 1' 'P -3 c 1'\n"\
"symbol laue '-P 3 2\"' '-3m1'\n"\
"symbol patt '-P 3 2\"' '-3m1'\n"\
"symbol pgrp '-P 3 2\"' '-3m1'\n"\
"hklasu ccp4 'h>=k and k>=0 and (h>k or l>=0)'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=1/3; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=1/3; 0<=z<1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x-y,z\n"\
"symop -x+y,-x,z\n"\
"symop y,x,-z+1/2\n"\
"symop -x,-x+y,-z+1/2\n"\
"symop x-y,-y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop y,-x+y,-z\n"\
"symop x-y,x,-z\n"\
"symop -y,-x,z+1/2\n"\
"symop x,x-y,z+1/2\n"\
"symop -x+y,y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  166\n"\
"basisop x,y,z\n"\
"symbol ccp4 166\n"\
"symbol Hall '-R 3 2\"'\n"\
"symbol xHM  'R -3 m :H'\n"\
"symbol old  'H -3 2/m' 'H -3 m'\n"\
"symbol laue '-P 3 2\"' '-3m1'\n"\
"symbol patt '-R 3 2\"' '-3m'\n"\
"symbol pgrp '-P 3 2\"' '-3m1'\n"\
"hklasu ccp4 'h>=k and k>=0 and (h>k or l>=0)'\n"\
"mapasu ccp4 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/6\n"\
"mapasu zero 0<=x<=1/3; 0<=y<=1/6; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/3; 0<=y<=1/6; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x-y,z\n"\
"symop -x+y,-x,z\n"\
"symop y,x,-z\n"\
"symop -x,-x+y,-z\n"\
"symop x-y,-y,-z\n"\
"symop -x,-y,-z\n"\
"symop y,-x+y,-z\n"\
"symop x-y,x,-z\n"\
"symop -y,-x,z\n"\
"symop x,x-y,z\n"\
"symop -x+y,y,z\n"\
"cenop x,y,z\n"\
"cenop x+2/3,y+1/3,z+1/3\n"\
"cenop x+1/3,y+2/3,z+2/3\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  166\n"\
"basisop -y+z,x+z,-x+y+z\n"\
"symbol ccp4 1166\n"\
"symbol Hall '-R 3 2\" (-y+z,x+z,-x+y+z)'\n"\
"symbol xHM  'R -3 m :R'\n"\
"symbol old  'R -3 2/m' 'R -3 m'\n"\
"symbol laue '-P 3* 2' '-3m'\n"\
"symbol patt '-P 3* 2' '-3m'\n"\
"symbol pgrp '-P 3* 2' '-3m'\n"\
"hklasu ccp4 'h>=k and k>=0 and (h>k or l>=0)'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop z,x,y\n"\
"symop y,z,x\n"\
"symop -y,-x,-z\n"\
"symop -z,-y,-x\n"\
"symop -x,-z,-y\n"\
"symop -x,-y,-z\n"\
"symop -z,-x,-y\n"\
"symop -y,-z,-x\n"\
"symop y,x,z\n"\
"symop z,y,x\n"\
"symop x,z,y\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  167\n"\
"basisop x,y,z\n"\
"symbol ccp4 167\n"\
"symbol Hall '-R 3 2\"c'\n"\
"symbol xHM  'R -3 c :H'\n"\
"symbol old  'H -3 2/c' 'H -3 c'\n"\
"symbol laue '-P 3 2\"' '-3m1'\n"\
"symbol patt '-R 3 2\"' '-3m'\n"\
"symbol pgrp '-P 3 2\"' '-3m1'\n"\
"hklasu ccp4 'h>=k and k>=0 and (h>k or l>=0)'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=1/6; 0<=z<=1/3\n"\
"mapasu nonz 0<=x<=1/3; -1/6<=y<=0; 1/12<=z<=7/12\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x-y,z\n"\
"symop -x+y,-x,z\n"\
"symop y,x,-z+1/2\n"\
"symop -x,-x+y,-z+1/2\n"\
"symop x-y,-y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop y,-x+y,-z\n"\
"symop x-y,x,-z\n"\
"symop -y,-x,z+1/2\n"\
"symop x,x-y,z+1/2\n"\
"symop -x+y,y,z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+2/3,y+1/3,z+1/3\n"\
"cenop x+1/3,y+2/3,z+2/3\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  167\n"\
"basisop -y+z,x+z,-x+y+z\n"\
"symbol ccp4 1167\n"\
"symbol Hall '-R 3 2\"c (-y+z,x+z,-x+y+z)'\n"\
"symbol xHM  'R -3 c :R'\n"\
"symbol old  'R -3 2/c' 'R -3 c'\n"\
"symbol laue '-P 3* 2' '-3m'\n"\
"symbol patt '-P 3* 2' '-3m'\n"\
"symbol pgrp '-P 3* 2' '-3m'\n"\
"hklasu ccp4 'h>=k and k>=0 and (h>k or l>=0)'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; -1/4<=y<=1/4; 0<=z<3/4\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop z,x,y\n"\
"symop y,z,x\n"\
"symop -y+1/2,-x+1/2,-z+1/2\n"\
"symop -z+1/2,-y+1/2,-x+1/2\n"\
"symop -x+1/2,-z+1/2,-y+1/2\n"\
"symop -x,-y,-z\n"\
"symop -z,-x,-y\n"\
"symop -y,-z,-x\n"\
"symop y+1/2,x+1/2,z+1/2\n"\
"symop z+1/2,y+1/2,x+1/2\n"\
"symop x+1/2,z+1/2,y+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  168\n"\
"basisop x,y,z\n"\
"symbol ccp4 168\n"\
"symbol Hall ' P 6'\n"\
"symbol xHM  'P 6'\n"\
"symbol old  'P 6'\n"\
"symbol laue '-P 6' '6/m'\n"\
"symbol patt '-P 6' '6/m'\n"\
"symbol pgrp ' P 6' '6'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<=2/3; 0<=y<=1/2; 0<=z<1\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=1/3; 0<=z<1\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=1/3; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x-y,x,z\n"\
"symop -y,x-y,z\n"\
"symop -x,-y,z\n"\
"symop -x+y,-x,z\n"\
"symop y,-x+y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  169\n"\
"basisop x,y,z\n"\
"symbol ccp4 169\n"\
"symbol Hall ' P 61'\n"\
"symbol xHM  'P 61'\n"\
"symbol old  'P 61'\n"\
"symbol laue '-P 6' '6/m'\n"\
"symbol patt '-P 6' '6/m'\n"\
"symbol pgrp ' P 6' '6'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<1/6\n"\
"mapasu zero 0<=x<1; 0<=y<1; 0<=z<1/6\n"\
"mapasu nonz 0<=x<1; 0<=y<1; 0<=z<1/6\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x-y,x,z+1/6\n"\
"symop -y,x-y,z+1/3\n"\
"symop -x,-y,z+1/2\n"\
"symop -x+y,-x,z+2/3\n"\
"symop y,-x+y,z+5/6\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  170\n"\
"basisop x,y,z\n"\
"symbol ccp4 170\n"\
"symbol Hall ' P 65'\n"\
"symbol xHM  'P 65'\n"\
"symbol old  'P 65'\n"\
"symbol laue '-P 6' '6/m'\n"\
"symbol patt '-P 6' '6/m'\n"\
"symbol pgrp ' P 6' '6'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<1/6\n"\
"mapasu zero 0<=x<1; 0<=y<1; 0<=z<1/6\n"\
"mapasu nonz 0<=x<1; 0<=y<1; 0<=z<1/6\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x-y,x,z+5/6\n"\
"symop -y,x-y,z+2/3\n"\
"symop -x,-y,z+1/2\n"\
"symop -x+y,-x,z+1/3\n"\
"symop y,-x+y,z+1/6\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  171\n"\
"basisop x,y,z\n"\
"symbol ccp4 171\n"\
"symbol Hall ' P 62'\n"\
"symbol xHM  'P 62'\n"\
"symbol old  'P 62'\n"\
"symbol laue '-P 6' '6/m'\n"\
"symbol patt '-P 6' '6/m'\n"\
"symbol pgrp ' P 6' '6'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<1/3\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<1/3\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/2; 0<=z<1/3\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x-y,x,z+1/3\n"\
"symop -y,x-y,z+2/3\n"\
"symop -x,-y,z\n"\
"symop -x+y,-x,z+1/3\n"\
"symop y,-x+y,z+2/3\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  172\n"\
"basisop x,y,z\n"\
"symbol ccp4 172\n"\
"symbol Hall ' P 64'\n"\
"symbol xHM  'P 64'\n"\
"symbol old  'P 64'\n"\
"symbol laue '-P 6' '6/m'\n"\
"symbol patt '-P 6' '6/m'\n"\
"symbol pgrp ' P 6' '6'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<1/3\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<1/3\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/2; 0<=z<1/3\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x-y,x,z+2/3\n"\
"symop -y,x-y,z+1/3\n"\
"symop -x,-y,z\n"\
"symop -x+y,-x,z+2/3\n"\
"symop y,-x+y,z+1/3\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  173\n"\
"basisop x,y,z\n"\
"symbol ccp4 173\n"\
"symbol Hall ' P 6c'\n"\
"symbol xHM  'P 63'\n"\
"symbol old  'P 63'\n"\
"symbol laue '-P 6' '6/m'\n"\
"symbol patt '-P 6' '6/m'\n"\
"symbol pgrp ' P 6' '6'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<=2/3; 0<=y<=2/3; 0<=z<1/2\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=1/3; 0<=z<1\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=1/3; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x-y,x,z+1/2\n"\
"symop -y,x-y,z\n"\
"symop -x,-y,z+1/2\n"\
"symop -x+y,-x,z\n"\
"symop y,-x+y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  174\n"\
"basisop x,y,z\n"\
"symbol ccp4 174\n"\
"symbol Hall ' P -6'\n"\
"symbol xHM  'P -6'\n"\
"symbol old  'P -6'\n"\
"symbol laue '-P 6' '6/m'\n"\
"symbol patt '-P 6' '6/m'\n"\
"symbol pgrp ' P -6' '-6'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/2\n"\
"cheshire 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+y,-x,-z\n"\
"symop -y,x-y,z\n"\
"symop x,y,-z\n"\
"symop -x+y,-x,z\n"\
"symop -y,x-y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  175\n"\
"basisop x,y,z\n"\
"symbol ccp4 175\n"\
"symbol Hall '-P 6'\n"\
"symbol xHM  'P 6/m'\n"\
"symbol old  'P 6/m'\n"\
"symbol laue '-P 6' '6/m'\n"\
"symbol patt '-P 6' '6/m'\n"\
"symbol pgrp '-P 6' '6/m'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=1/3; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=1/3; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x-y,x,z\n"\
"symop -y,x-y,z\n"\
"symop -x,-y,z\n"\
"symop -x+y,-x,z\n"\
"symop y,-x+y,z\n"\
"symop -x,-y,-z\n"\
"symop -x+y,-x,-z\n"\
"symop y,-x+y,-z\n"\
"symop x,y,-z\n"\
"symop x-y,x,-z\n"\
"symop -y,x-y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  176\n"\
"basisop x,y,z\n"\
"symbol ccp4 176\n"\
"symbol Hall '-P 6c'\n"\
"symbol xHM  'P 63/m'\n"\
"symbol old  'P 63/m'\n"\
"symbol laue '-P 6' '6/m'\n"\
"symbol patt '-P 6' '6/m'\n"\
"symbol pgrp '-P 6' '6/m'\n"\
"hklasu ccp4 'l>=0 and ((h>=0 and k>0) or (h=0 and k=0))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=2/3; 0<=z<1/3\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=1/3; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x-y,x,z+1/2\n"\
"symop -y,x-y,z\n"\
"symop -x,-y,z+1/2\n"\
"symop -x+y,-x,z\n"\
"symop y,-x+y,z+1/2\n"\
"symop -x,-y,-z\n"\
"symop -x+y,-x,-z+1/2\n"\
"symop y,-x+y,-z\n"\
"symop x,y,-z+1/2\n"\
"symop x-y,x,-z\n"\
"symop -y,x-y,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  177\n"\
"basisop x,y,z\n"\
"symbol ccp4 177\n"\
"symbol Hall ' P 6 2'\n"\
"symbol xHM  'P 6 2 2'\n"\
"symbol old  'P 6 2 2'\n"\
"symbol laue '-P 6 2' '6/mmm'\n"\
"symbol patt '-P 6 2' '6/mmm'\n"\
"symbol pgrp ' P 6 2' '622'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=2/3; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=1/3; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=1/3; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x-y,x,z\n"\
"symop -y,x-y,z\n"\
"symop -x,-y,z\n"\
"symop -x+y,-x,z\n"\
"symop y,-x+y,z\n"\
"symop -y,-x,-z\n"\
"symop x-y,-y,-z\n"\
"symop x,x-y,-z\n"\
"symop y,x,-z\n"\
"symop -x+y,y,-z\n"\
"symop -x,-x+y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  178\n"\
"basisop x,y,z\n"\
"symbol ccp4 178\n"\
"symbol Hall ' P 61 2 (x,y,z+5/12)'\n"\
"symbol xHM  'P 61 2 2'\n"\
"symbol old  'P 61 2 2'\n"\
"symbol laue '-P 6 2' '6/mmm'\n"\
"symbol patt '-P 6 2' '6/mmm'\n"\
"symbol pgrp ' P 6 2' '622'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<=1/12\n"\
"mapasu zero 0<=x<1; 0<=y<1; 0<=z<1/6\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/2; -1/12<=z<=1/12\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x-y,x,z+1/6\n"\
"symop -y,x-y,z+1/3\n"\
"symop -x,-y,z+1/2\n"\
"symop -x+y,-x,z+2/3\n"\
"symop y,-x+y,z+5/6\n"\
"symop -y,-x,-z+5/6\n"\
"symop x-y,-y,-z\n"\
"symop x,x-y,-z+1/6\n"\
"symop y,x,-z+1/3\n"\
"symop -x+y,y,-z+1/2\n"\
"symop -x,-x+y,-z+2/3\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  179\n"\
"basisop x,y,z\n"\
"symbol ccp4 179\n"\
"symbol Hall ' P 65 2 (x,y,z+1/12)'\n"\
"symbol xHM  'P 65 2 2'\n"\
"symbol old  'P 65 2 2'\n"\
"symbol laue '-P 6 2' '6/mmm'\n"\
"symbol patt '-P 6 2' '6/mmm'\n"\
"symbol pgrp ' P 6 2' '622'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<=1/12\n"\
"mapasu zero 0<=x<=1/2; 0<=y<1; 0<=z<1/3\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<1; 1/12<=z<=1/4\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x-y,x,z+5/6\n"\
"symop -y,x-y,z+2/3\n"\
"symop -x,-y,z+1/2\n"\
"symop -x+y,-x,z+1/3\n"\
"symop y,-x+y,z+1/6\n"\
"symop -y,-x,-z+1/6\n"\
"symop x-y,-y,-z\n"\
"symop x,x-y,-z+5/6\n"\
"symop y,x,-z+2/3\n"\
"symop -x+y,y,-z+1/2\n"\
"symop -x,-x+y,-z+1/3\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  180\n"\
"basisop x,y,z\n"\
"symbol ccp4 180\n"\
"symbol Hall ' P 62 2 (x,y,z+1/3)'\n"\
"symbol xHM  'P 62 2 2'\n"\
"symbol old  'P 62 2 2'\n"\
"symbol laue '-P 6 2' '6/mmm'\n"\
"symbol patt '-P 6 2' '6/mmm'\n"\
"symbol pgrp ' P 6 2' '622'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<=1/6\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<=1/6\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/2; 0<=z<=1/6\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x-y,x,z+1/3\n"\
"symop -y,x-y,z+2/3\n"\
"symop -x,-y,z\n"\
"symop -x+y,-x,z+1/3\n"\
"symop y,-x+y,z+2/3\n"\
"symop -y,-x,-z+2/3\n"\
"symop x-y,-y,-z\n"\
"symop x,x-y,-z+1/3\n"\
"symop y,x,-z+2/3\n"\
"symop -x+y,y,-z\n"\
"symop -x,-x+y,-z+1/3\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  181\n"\
"basisop x,y,z\n"\
"symbol ccp4 181\n"\
"symbol Hall ' P 64 2 (x,y,z+1/6)'\n"\
"symbol xHM  'P 64 2 2'\n"\
"symbol old  'P 64 2 2'\n"\
"symbol laue '-P 6 2' '6/mmm'\n"\
"symbol patt '-P 6 2' '6/mmm'\n"\
"symbol pgrp ' P 6 2' '622'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<=1/6\n"\
"mapasu zero 0<=x<1; 0<=y<=1/2; 0<=z<=1/6\n"\
"mapasu nonz 0<=x<1; 0<=y<=1/2; 0<=z<=1/6\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x-y,x,z+2/3\n"\
"symop -y,x-y,z+1/3\n"\
"symop -x,-y,z\n"\
"symop -x+y,-x,z+2/3\n"\
"symop y,-x+y,z+1/3\n"\
"symop -y,-x,-z+1/3\n"\
"symop x-y,-y,-z\n"\
"symop x,x-y,-z+2/3\n"\
"symop y,x,-z+1/3\n"\
"symop -x+y,y,-z\n"\
"symop -x,-x+y,-z+2/3\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  182\n"\
"basisop x,y,z\n"\
"symbol ccp4 182\n"\
"symbol Hall ' P 6c 2c'\n"\
"symbol xHM  'P 63 2 2'\n"\
"symbol old  'P 63 2 2'\n"\
"symbol laue '-P 6 2' '6/mmm'\n"\
"symbol patt '-P 6 2' '6/mmm'\n"\
"symbol pgrp ' P 6 2' '622'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/4\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=2/3; 0<=z<1/3\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=1/3; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x-y,x,z+1/2\n"\
"symop -y,x-y,z\n"\
"symop -x,-y,z+1/2\n"\
"symop -x+y,-x,z\n"\
"symop y,-x+y,z+1/2\n"\
"symop -y,-x,-z+1/2\n"\
"symop x-y,-y,-z\n"\
"symop x,x-y,-z+1/2\n"\
"symop y,x,-z\n"\
"symop -x+y,y,-z+1/2\n"\
"symop -x,-x+y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  183\n"\
"basisop x,y,z\n"\
"symbol ccp4 183\n"\
"symbol Hall ' P 6 -2'\n"\
"symbol xHM  'P 6 m m'\n"\
"symbol old  'P 6 m m'\n"\
"symbol laue '-P 6 2' '6/mmm'\n"\
"symbol patt '-P 6 2' '6/mmm'\n"\
"symbol pgrp ' P 6 -2' '6mm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=1/3; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; -1/3<=y<=0; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x-y,x,z\n"\
"symop -y,x-y,z\n"\
"symop -x,-y,z\n"\
"symop -x+y,-x,z\n"\
"symop y,-x+y,z\n"\
"symop y,x,z\n"\
"symop -x+y,y,z\n"\
"symop -x,-x+y,z\n"\
"symop -y,-x,z\n"\
"symop x-y,-y,z\n"\
"symop x,x-y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  184\n"\
"basisop x,y,z\n"\
"symbol ccp4 184\n"\
"symbol Hall ' P 6 -2c'\n"\
"symbol xHM  'P 6 c c'\n"\
"symbol old  'P 6 c c'\n"\
"symbol laue '-P 6 2' '6/mmm'\n"\
"symbol patt '-P 6 2' '6/mmm'\n"\
"symbol pgrp ' P 6 -2' '6mm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=1/3; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=1/3; 0<=z<1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x-y,x,z\n"\
"symop -y,x-y,z\n"\
"symop -x,-y,z\n"\
"symop -x+y,-x,z\n"\
"symop y,-x+y,z\n"\
"symop y,x,z+1/2\n"\
"symop -x+y,y,z+1/2\n"\
"symop -x,-x+y,z+1/2\n"\
"symop -y,-x,z+1/2\n"\
"symop x-y,-y,z+1/2\n"\
"symop x,x-y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  185\n"\
"basisop x,y,z\n"\
"symbol ccp4 185\n"\
"symbol Hall ' P 6c -2'\n"\
"symbol xHM  'P 63 c m'\n"\
"symbol old  'P 63 c m'\n"\
"symbol laue '-P 6 2' '6/mmm'\n"\
"symbol patt '-P 6 2' '6/mmm'\n"\
"symbol pgrp ' P 6 -2' '6mm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=1/3; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=1/3; 0<=z<1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x-y,x,z+1/2\n"\
"symop -y,x-y,z\n"\
"symop -x,-y,z+1/2\n"\
"symop -x+y,-x,z\n"\
"symop y,-x+y,z+1/2\n"\
"symop y,x,z\n"\
"symop -x+y,y,z+1/2\n"\
"symop -x,-x+y,z\n"\
"symop -y,-x,z+1/2\n"\
"symop x-y,-y,z\n"\
"symop x,x-y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  186\n"\
"basisop x,y,z\n"\
"symbol ccp4 186\n"\
"symbol Hall ' P 6c -2c'\n"\
"symbol xHM  'P 63 m c'\n"\
"symbol old  'P 63 m c'\n"\
"symbol laue '-P 6 2' '6/mmm'\n"\
"symbol patt '-P 6 2' '6/mmm'\n"\
"symbol pgrp ' P 6 -2' '6mm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=1/3; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; -1/3<=y<=0; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=0\n"\
"symop x,y,z\n"\
"symop x-y,x,z+1/2\n"\
"symop -y,x-y,z\n"\
"symop -x,-y,z+1/2\n"\
"symop -x+y,-x,z\n"\
"symop y,-x+y,z+1/2\n"\
"symop y,x,z+1/2\n"\
"symop -x+y,y,z\n"\
"symop -x,-x+y,z+1/2\n"\
"symop -y,-x,z\n"\
"symop x-y,-y,z+1/2\n"\
"symop x,x-y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  187\n"\
"basisop x,y,z\n"\
"symbol ccp4 187\n"\
"symbol Hall ' P -6 2'\n"\
"symbol xHM  'P -6 m 2'\n"\
"symbol old  'P -6 m 2'\n"\
"symbol laue '-P 6 2' '6/mmm'\n"\
"symbol patt '-P 6 2' '6/mmm'\n"\
"symbol pgrp ' P -6 2' '-62m'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/2\n"\
"cheshire 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+y,-x,-z\n"\
"symop -y,x-y,z\n"\
"symop x,y,-z\n"\
"symop -x+y,-x,z\n"\
"symop -y,x-y,-z\n"\
"symop -y,-x,-z\n"\
"symop -x+y,y,z\n"\
"symop x,x-y,-z\n"\
"symop -y,-x,z\n"\
"symop -x+y,y,-z\n"\
"symop x,x-y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  188\n"\
"basisop x,y,z\n"\
"symbol ccp4 188\n"\
"symbol Hall ' P -6c 2'\n"\
"symbol xHM  'P -6 c 2'\n"\
"symbol old  'P -6 c 2'\n"\
"symbol laue '-P 6 2' '6/mmm'\n"\
"symbol patt '-P 6 2' '6/mmm'\n"\
"symbol pgrp ' P -6 2' '-62m'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=2/3; 0<=z<1/3\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/4\n"\
"cheshire 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+y,-x,-z+1/2\n"\
"symop -y,x-y,z\n"\
"symop x,y,-z+1/2\n"\
"symop -x+y,-x,z\n"\
"symop -y,x-y,-z+1/2\n"\
"symop -y,-x,-z\n"\
"symop -x+y,y,z+1/2\n"\
"symop x,x-y,-z\n"\
"symop -y,-x,z+1/2\n"\
"symop -x+y,y,-z\n"\
"symop x,x-y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  189\n"\
"basisop x,y,z\n"\
"symbol ccp4 189\n"\
"symbol Hall ' P -6 -2'\n"\
"symbol xHM  'P -6 2 m'\n"\
"symbol old  'P -6 2 m'\n"\
"symbol laue '-P 6 2' '6/mmm'\n"\
"symbol patt '-P 6 2' '6/mmm'\n"\
"symbol pgrp ' P -6 -2' '-6m2'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=1/3; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=1/3; 0<=z<=1/2\n"\
"cheshire 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+y,-x,-z\n"\
"symop -y,x-y,z\n"\
"symop x,y,-z\n"\
"symop -x+y,-x,z\n"\
"symop -y,x-y,-z\n"\
"symop y,x,z\n"\
"symop x-y,-y,-z\n"\
"symop -x,-x+y,z\n"\
"symop y,x,-z\n"\
"symop x-y,-y,z\n"\
"symop -x,-x+y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  190\n"\
"basisop x,y,z\n"\
"symbol ccp4 190\n"\
"symbol Hall ' P -6c -2c'\n"\
"symbol xHM  'P -6 2 c'\n"\
"symbol old  'P -6 2 c'\n"\
"symbol laue '-P 6 2' '6/mmm'\n"\
"symbol patt '-P 6 2' '6/mmm'\n"\
"symbol pgrp ' P -6 -2' '-6m2'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=2/3; 0<=z<1/3\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=1/3; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+y,-x,-z+1/2\n"\
"symop -y,x-y,z\n"\
"symop x,y,-z+1/2\n"\
"symop -x+y,-x,z\n"\
"symop -y,x-y,-z+1/2\n"\
"symop y,x,z+1/2\n"\
"symop x-y,-y,-z\n"\
"symop -x,-x+y,z+1/2\n"\
"symop y,x,-z\n"\
"symop x-y,-y,z+1/2\n"\
"symop -x,-x+y,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  191\n"\
"basisop x,y,z\n"\
"symbol ccp4 191\n"\
"symbol Hall '-P 6 2'\n"\
"symbol xHM  'P 6/m m m'\n"\
"symbol old  'P 6/m 2/m 2/m' 'P 6/m m m'\n"\
"symbol laue '-P 6 2' '6/mmm'\n"\
"symbol patt '-P 6 2' '6/mmm'\n"\
"symbol pgrp '-P 6 2' '6/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=2/3; 0<=y<=1/3; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=1/3; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; -1/3<=y<=0; 0<=z<=1/2\n"\
"cheshire 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x-y,x,z\n"\
"symop -y,x-y,z\n"\
"symop -x,-y,z\n"\
"symop -x+y,-x,z\n"\
"symop y,-x+y,z\n"\
"symop -y,-x,-z\n"\
"symop x-y,-y,-z\n"\
"symop x,x-y,-z\n"\
"symop y,x,-z\n"\
"symop -x+y,y,-z\n"\
"symop -x,-x+y,-z\n"\
"symop -x,-y,-z\n"\
"symop -x+y,-x,-z\n"\
"symop y,-x+y,-z\n"\
"symop x,y,-z\n"\
"symop x-y,x,-z\n"\
"symop -y,x-y,-z\n"\
"symop y,x,z\n"\
"symop -x+y,y,z\n"\
"symop -x,-x+y,z\n"\
"symop -y,-x,z\n"\
"symop x-y,-y,z\n"\
"symop x,x-y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  192\n"\
"basisop x,y,z\n"\
"symbol ccp4 192\n"\
"symbol Hall '-P 6 2c'\n"\
"symbol xHM  'P 6/m c c'\n"\
"symbol old  'P 6/m 2/c 2/c' 'P 6/m c c'\n"\
"symbol laue '-P 6 2' '6/mmm'\n"\
"symbol patt '-P 6 2' '6/mmm'\n"\
"symbol pgrp '-P 6 2' '6/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=1/3; 0<=z<1/3\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=1/3; 0<=z<=1/4\n"\
"cheshire 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x-y,x,z\n"\
"symop -y,x-y,z\n"\
"symop -x,-y,z\n"\
"symop -x+y,-x,z\n"\
"symop y,-x+y,z\n"\
"symop -y,-x,-z+1/2\n"\
"symop x-y,-y,-z+1/2\n"\
"symop x,x-y,-z+1/2\n"\
"symop y,x,-z+1/2\n"\
"symop -x+y,y,-z+1/2\n"\
"symop -x,-x+y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop -x+y,-x,-z\n"\
"symop y,-x+y,-z\n"\
"symop x,y,-z\n"\
"symop x-y,x,-z\n"\
"symop -y,x-y,-z\n"\
"symop y,x,z+1/2\n"\
"symop -x+y,y,z+1/2\n"\
"symop -x,-x+y,z+1/2\n"\
"symop -y,-x,z+1/2\n"\
"symop x-y,-y,z+1/2\n"\
"symop x,x-y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  193\n"\
"basisop x,y,z\n"\
"symbol ccp4 193\n"\
"symbol Hall '-P 6c 2'\n"\
"symbol xHM  'P 63/m c m'\n"\
"symbol old  'P 63/m 2/c 2/m' 'P 63/m c m'\n"\
"symbol laue '-P 6 2' '6/mmm'\n"\
"symbol patt '-P 6 2' '6/mmm'\n"\
"symbol pgrp '-P 6 2' '6/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=1/3; 0<=z<1/3\n"\
"mapasu nonz 0<=x<=2/3; 0<=y<=1/3; 0<=z<=1/4\n"\
"cheshire 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x-y,x,z+1/2\n"\
"symop -y,x-y,z\n"\
"symop -x,-y,z+1/2\n"\
"symop -x+y,-x,z\n"\
"symop y,-x+y,z+1/2\n"\
"symop -y,-x,-z\n"\
"symop x-y,-y,-z+1/2\n"\
"symop x,x-y,-z\n"\
"symop y,x,-z+1/2\n"\
"symop -x+y,y,-z\n"\
"symop -x,-x+y,-z+1/2\n"\
"symop -x,-y,-z\n"\
"symop -x+y,-x,-z+1/2\n"\
"symop y,-x+y,-z\n"\
"symop x,y,-z+1/2\n"\
"symop x-y,x,-z\n"\
"symop -y,x-y,-z+1/2\n"\
"symop y,x,z\n"\
"symop -x+y,y,z+1/2\n"\
"symop -x,-x+y,z\n"\
"symop -y,-x,z+1/2\n"\
"symop x-y,-y,z\n"\
"symop x,x-y,z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  194\n"\
"basisop x,y,z\n"\
"symbol ccp4 194\n"\
"symbol Hall '-P 6c 2c'\n"\
"symbol xHM  'P 63/m m c'\n"\
"symbol old  'P 63/m 2/m 2/c' 'P 63/m m c'\n"\
"symbol laue '-P 6 2' '6/mmm'\n"\
"symbol patt '-P 6 2' '6/mmm'\n"\
"symbol pgrp '-P 6 2' '6/mmm'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=2/3; 0<=y<=2/3; 0<=z<1/3\n"\
"mapasu nonz 0<=x<=1/2; -1/3<=y<=0; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=2/3; 0<=y<=2/3; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop x-y,x,z+1/2\n"\
"symop -y,x-y,z\n"\
"symop -x,-y,z+1/2\n"\
"symop -x+y,-x,z\n"\
"symop y,-x+y,z+1/2\n"\
"symop -y,-x,-z+1/2\n"\
"symop x-y,-y,-z\n"\
"symop x,x-y,-z+1/2\n"\
"symop y,x,-z\n"\
"symop -x+y,y,-z+1/2\n"\
"symop -x,-x+y,-z\n"\
"symop -x,-y,-z\n"\
"symop -x+y,-x,-z+1/2\n"\
"symop y,-x+y,-z\n"\
"symop x,y,-z+1/2\n"\
"symop x-y,x,-z\n"\
"symop -y,x-y,-z+1/2\n"\
"symop y,x,z+1/2\n"\
"symop -x+y,y,z\n"\
"symop -x,-x+y,z+1/2\n"\
"symop -y,-x,z\n"\
"symop x-y,-y,z+1/2\n"\
"symop x,x-y,z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  195\n"\
"basisop x,y,z\n"\
"symbol ccp4 195\n"\
"symbol Hall ' P 2 2 3'\n"\
"symbol xHM  'P 2 3'\n"\
"symbol old  'P 2 3'\n"\
"symbol laue '-P 2 2 3' 'm-3'\n"\
"symbol patt '-P 2 2 3' 'm-3'\n"\
"symbol pgrp ' P 2 2 3' '23'\n"\
"hklasu ccp4 'h>=0 and ((l>=h and k>h) or (l=h and k=h))'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z\n"\
"symop -x,y,-z\n"\
"symop z,x,y\n"\
"symop -z,-x,y\n"\
"symop z,-x,-y\n"\
"symop -z,x,-y\n"\
"symop y,z,x\n"\
"symop y,-z,-x\n"\
"symop -y,z,-x\n"\
"symop -y,-z,x\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  196\n"\
"basisop x,y,z\n"\
"symbol ccp4 196\n"\
"symbol Hall ' F 2 2 3'\n"\
"symbol xHM  'F 2 3'\n"\
"symbol old  'F 2 3'\n"\
"symbol laue '-P 2 2 3' 'm-3'\n"\
"symbol patt '-F 2 2 3' 'm-3'\n"\
"symbol pgrp ' P 2 2 3' '23'\n"\
"hklasu ccp4 'h>=0 and ((l>=h and k>h) or (l=h and k=h))'\n"\
"mapasu ccp4 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z\n"\
"symop -x,y,-z\n"\
"symop z,x,y\n"\
"symop -z,-x,y\n"\
"symop z,-x,-y\n"\
"symop -z,x,-y\n"\
"symop y,z,x\n"\
"symop y,-z,-x\n"\
"symop -y,z,-x\n"\
"symop -y,-z,x\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"cenop x+1/2,y,z+1/2\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  197\n"\
"basisop x,y,z\n"\
"symbol ccp4 197\n"\
"symbol Hall ' I 2 2 3'\n"\
"symbol xHM  'I 2 3'\n"\
"symbol old  'I 2 3'\n"\
"symbol laue '-P 2 2 3' 'm-3'\n"\
"symbol patt '-I 2 2 3' 'm-3'\n"\
"symbol pgrp ' P 2 2 3' '23'\n"\
"hklasu ccp4 'h>=0 and ((l>=h and k>h) or (l=h and k=h))'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z\n"\
"symop -x,y,-z\n"\
"symop z,x,y\n"\
"symop -z,-x,y\n"\
"symop z,-x,-y\n"\
"symop -z,x,-y\n"\
"symop y,z,x\n"\
"symop y,-z,-x\n"\
"symop -y,z,-x\n"\
"symop -y,-z,x\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  198\n"\
"basisop x,y,z\n"\
"symbol ccp4 198\n"\
"symbol Hall ' P 2ac 2ab 3'\n"\
"symbol xHM  'P 21 3'\n"\
"symbol old  'P 21 3'\n"\
"symbol laue '-P 2 2 3' 'm-3'\n"\
"symbol patt '-P 2 2 3' 'm-3'\n"\
"symbol pgrp ' P 2 2 3' '23'\n"\
"hklasu ccp4 'h>=0 and ((l>=h and k>h) or (l=h and k=h))'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz -1/4<=x<1/4; 0<=y<1/2; 0<=z<=3/4\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop z,x,y\n"\
"symop -z+1/2,-x,y+1/2\n"\
"symop z+1/2,-x+1/2,-y\n"\
"symop -z,x+1/2,-y+1/2\n"\
"symop y,z,x\n"\
"symop y+1/2,-z+1/2,-x\n"\
"symop -y,z+1/2,-x+1/2\n"\
"symop -y+1/2,-z,x+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  199\n"\
"basisop x,y,z\n"\
"symbol ccp4 199\n"\
"symbol Hall ' I 2b 2c 3'\n"\
"symbol xHM  'I 21 3'\n"\
"symbol old  'I 21 3'\n"\
"symbol laue '-P 2 2 3' 'm-3'\n"\
"symbol patt '-I 2 2 3' 'm-3'\n"\
"symbol pgrp ' P 2 2 3' '23'\n"\
"hklasu ccp4 'h>=0 and ((l>=h and k>h) or (l=h and k=h))'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<1/2; 0<z<3/4\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop -x,-y+1/2,z\n"\
"symop x,-y,-z+1/2\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop z,x,y\n"\
"symop -z,-x+1/2,y\n"\
"symop z,-x,-y+1/2\n"\
"symop -z,x+1/2,-y+1/2\n"\
"symop y,z,x\n"\
"symop y,-z,-x+1/2\n"\
"symop -y,z+1/2,-x+1/2\n"\
"symop -y+1/2,-z,x+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  200\n"\
"basisop x,y,z\n"\
"symbol ccp4 200\n"\
"symbol Hall '-P 2 2 3'\n"\
"symbol xHM  'P m -3'\n"\
"symbol old  'P 2/m -3' 'P m -3'\n"\
"symbol laue '-P 2 2 3' 'm-3'\n"\
"symbol patt '-P 2 2 3' 'm-3'\n"\
"symbol pgrp '-P 2 2 3' 'm-3'\n"\
"hklasu ccp4 'h>=0 and ((l>=h and k>h) or (l=h and k=h))'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z\n"\
"symop -x,y,-z\n"\
"symop z,x,y\n"\
"symop -z,-x,y\n"\
"symop z,-x,-y\n"\
"symop -z,x,-y\n"\
"symop y,z,x\n"\
"symop y,-z,-x\n"\
"symop -y,z,-x\n"\
"symop -y,-z,x\n"\
"symop -x,-y,-z\n"\
"symop x,y,-z\n"\
"symop -x,y,z\n"\
"symop x,-y,z\n"\
"symop -z,-x,-y\n"\
"symop z,x,-y\n"\
"symop -z,x,y\n"\
"symop z,-x,y\n"\
"symop -y,-z,-x\n"\
"symop -y,z,x\n"\
"symop y,-z,x\n"\
"symop y,z,-x\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  201\n"\
"basisop x-1/4,y-1/4,z-1/4\n"\
"symbol ccp4 201\n"\
"symbol Hall '-P 2ab 2bc 3 (x-1/4,y-1/4,z-1/4)'\n"\
"symbol xHM  'P n -3 :1'\n"\
"symbol old  'P 2/n -3' 'P n -3'\n"\
"symbol laue '-P 2 2 3' 'm-3'\n"\
"symbol patt '-P 2 2 3' 'm-3'\n"\
"symbol pgrp '-P 2 2 3' 'm-3'\n"\
"hklasu ccp4 'h>=0 and ((l>=h and k>h) or (l=h and k=h))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z\n"\
"symop -x,y,-z\n"\
"symop z,x,y\n"\
"symop -z,-x,y\n"\
"symop z,-x,-y\n"\
"symop -z,x,-y\n"\
"symop y,z,x\n"\
"symop y,-z,-x\n"\
"symop -y,z,-x\n"\
"symop -y,-z,x\n"\
"symop -x+1/2,-y+1/2,-z+1/2\n"\
"symop x+1/2,y+1/2,-z+1/2\n"\
"symop -x+1/2,y+1/2,z+1/2\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"symop -z+1/2,-x+1/2,-y+1/2\n"\
"symop z+1/2,x+1/2,-y+1/2\n"\
"symop -z+1/2,x+1/2,y+1/2\n"\
"symop z+1/2,-x+1/2,y+1/2\n"\
"symop -y+1/2,-z+1/2,-x+1/2\n"\
"symop -y+1/2,z+1/2,x+1/2\n"\
"symop y+1/2,-z+1/2,x+1/2\n"\
"symop y+1/2,z+1/2,-x+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  201\n"\
"basisop x,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 2ab 2bc 3'\n"\
"symbol xHM  'P n -3 :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2 3' 'm-3'\n"\
"symbol patt '-P 2 2 3' 'm-3'\n"\
"symbol pgrp '-P 2 2 3' 'm-3'\n"\
"hklasu ccp4 'h>=0 and ((l>=h and k>h) or (l=h and k=h))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop z,x,y\n"\
"symop -z+1/2,-x+1/2,y\n"\
"symop z,-x+1/2,-y+1/2\n"\
"symop -z+1/2,x,-y+1/2\n"\
"symop y,z,x\n"\
"symop y,-z+1/2,-x+1/2\n"\
"symop -y+1/2,z,-x+1/2\n"\
"symop -y+1/2,-z+1/2,x\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -x,y+1/2,z+1/2\n"\
"symop x+1/2,-y,z+1/2\n"\
"symop -z,-x,-y\n"\
"symop z+1/2,x+1/2,-y\n"\
"symop -z,x+1/2,y+1/2\n"\
"symop z+1/2,-x,y+1/2\n"\
"symop -y,-z,-x\n"\
"symop -y,z+1/2,x+1/2\n"\
"symop y+1/2,-z,x+1/2\n"\
"symop y+1/2,z+1/2,-x\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  202\n"\
"basisop x,y,z\n"\
"symbol ccp4 202\n"\
"symbol Hall '-F 2 2 3'\n"\
"symbol xHM  'F m -3'\n"\
"symbol old  'F 2/m -3' 'F m -3'\n"\
"symbol laue '-P 2 2 3' 'm-3'\n"\
"symbol patt '-F 2 2 3' 'm-3'\n"\
"symbol pgrp '-P 2 2 3' 'm-3'\n"\
"hklasu ccp4 'h>=0 and ((l>=h and k>h) or (l=h and k=h))'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/4\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z\n"\
"symop -x,y,-z\n"\
"symop z,x,y\n"\
"symop -z,-x,y\n"\
"symop z,-x,-y\n"\
"symop -z,x,-y\n"\
"symop y,z,x\n"\
"symop y,-z,-x\n"\
"symop -y,z,-x\n"\
"symop -y,-z,x\n"\
"symop -x,-y,-z\n"\
"symop x,y,-z\n"\
"symop -x,y,z\n"\
"symop x,-y,z\n"\
"symop -z,-x,-y\n"\
"symop z,x,-y\n"\
"symop -z,x,y\n"\
"symop z,-x,y\n"\
"symop -y,-z,-x\n"\
"symop -y,z,x\n"\
"symop y,-z,x\n"\
"symop y,z,-x\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"cenop x+1/2,y,z+1/2\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  203\n"\
"basisop x+1/8,y+1/8,z+1/8\n"\
"symbol ccp4 203\n"\
"symbol Hall '-F 2uv 2vw 3 (x+1/8,y+1/8,z+1/8)'\n"\
"symbol xHM  'F d -3 :1'\n"\
"symbol old  'F 2/d -3' 'F d -3'\n"\
"symbol laue '-P 2 2 3' 'm-3'\n"\
"symbol patt '-F 2 2 3' 'm-3'\n"\
"symbol pgrp '-P 2 2 3' 'm-3'\n"\
"hklasu ccp4 'h>=0 and ((l>=h and k>h) or (l=h and k=h))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/8; 0<=y<=1/8; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/8; 0<=y<=1/8; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop z,x,y\n"\
"symop -z+1/2,-x+1/2,y\n"\
"symop z,-x+1/2,-y+1/2\n"\
"symop -z+1/2,x,-y+1/2\n"\
"symop y,z,x\n"\
"symop y,-z+1/2,-x+1/2\n"\
"symop -y+1/2,z,-x+1/2\n"\
"symop -y+1/2,-z+1/2,x\n"\
"symop -x+1/4,-y+1/4,-z+1/4\n"\
"symop x+3/4,y+3/4,-z+1/4\n"\
"symop -x+1/4,y+3/4,z+3/4\n"\
"symop x+3/4,-y+1/4,z+3/4\n"\
"symop -z+1/4,-x+1/4,-y+1/4\n"\
"symop z+3/4,x+3/4,-y+1/4\n"\
"symop -z+1/4,x+3/4,y+3/4\n"\
"symop z+3/4,-x+1/4,y+3/4\n"\
"symop -y+1/4,-z+1/4,-x+1/4\n"\
"symop -y+1/4,z+3/4,x+3/4\n"\
"symop y+3/4,-z+1/4,x+3/4\n"\
"symop y+3/4,z+3/4,-x+1/4\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"cenop x+1/2,y,z+1/2\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  203\n"\
"basisop x,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-F 2uv 2vw 3'\n"\
"symbol xHM  'F d -3 :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 2 2 3' 'm-3'\n"\
"symbol patt '-F 2 2 3' 'm-3'\n"\
"symbol pgrp '-P 2 2 3' 'm-3'\n"\
"hklasu ccp4 'h>=0 and ((l>=h and k>h) or (l=h and k=h))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/8; 0<=y<=1/8; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/8; 0<=y<=1/8; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -x+1/4,-y+1/4,z\n"\
"symop x,-y+1/4,-z+1/4\n"\
"symop -x+1/4,y,-z+1/4\n"\
"symop z,x,y\n"\
"symop -z+1/4,-x+1/4,y\n"\
"symop z,-x+1/4,-y+1/4\n"\
"symop -z+1/4,x,-y+1/4\n"\
"symop y,z,x\n"\
"symop y,-z+1/4,-x+1/4\n"\
"symop -y+1/4,z,-x+1/4\n"\
"symop -y+1/4,-z+1/4,x\n"\
"symop -x,-y,-z\n"\
"symop x+3/4,y+3/4,-z\n"\
"symop -x,y+3/4,z+3/4\n"\
"symop x+3/4,-y,z+3/4\n"\
"symop -z,-x,-y\n"\
"symop z+3/4,x+3/4,-y\n"\
"symop -z,x+3/4,y+3/4\n"\
"symop z+3/4,-x,y+3/4\n"\
"symop -y,-z,-x\n"\
"symop -y,z+3/4,x+3/4\n"\
"symop y+3/4,-z,x+3/4\n"\
"symop y+3/4,z+3/4,-x\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"cenop x+1/2,y,z+1/2\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  204\n"\
"basisop x,y,z\n"\
"symbol ccp4 204\n"\
"symbol Hall '-I 2 2 3'\n"\
"symbol xHM  'I m -3'\n"\
"symbol old  'I 2/m -3' 'I m -3'\n"\
"symbol laue '-P 2 2 3' 'm-3'\n"\
"symbol patt '-I 2 2 3' 'm-3'\n"\
"symbol pgrp '-P 2 2 3' 'm-3'\n"\
"hklasu ccp4 'h>=0 and ((l>=h and k>h) or (l=h and k=h))'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z\n"\
"symop -x,y,-z\n"\
"symop z,x,y\n"\
"symop -z,-x,y\n"\
"symop z,-x,-y\n"\
"symop -z,x,-y\n"\
"symop y,z,x\n"\
"symop y,-z,-x\n"\
"symop -y,z,-x\n"\
"symop -y,-z,x\n"\
"symop -x,-y,-z\n"\
"symop x,y,-z\n"\
"symop -x,y,z\n"\
"symop x,-y,z\n"\
"symop -z,-x,-y\n"\
"symop z,x,-y\n"\
"symop -z,x,y\n"\
"symop z,-x,y\n"\
"symop -y,-z,-x\n"\
"symop -y,z,x\n"\
"symop y,-z,x\n"\
"symop y,z,-x\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  205\n"\
"basisop x,y,z\n"\
"symbol ccp4 205\n"\
"symbol Hall '-P 2ac 2ab 3'\n"\
"symbol xHM  'P a -3'\n"\
"symbol old  'P 21/a -3' 'P a -3'\n"\
"symbol laue '-P 2 2 3' 'm-3'\n"\
"symbol patt '-P 2 2 3' 'm-3'\n"\
"symbol pgrp '-P 2 2 3' 'm-3'\n"\
"hklasu ccp4 'h>=0 and ((l>=h and k>h) or (l=h and k=h))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/4; -1/4<=z<1/4\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop z,x,y\n"\
"symop -z+1/2,-x,y+1/2\n"\
"symop z+1/2,-x+1/2,-y\n"\
"symop -z,x+1/2,-y+1/2\n"\
"symop y,z,x\n"\
"symop y+1/2,-z+1/2,-x\n"\
"symop -y,z+1/2,-x+1/2\n"\
"symop -y+1/2,-z,x+1/2\n"\
"symop -x,-y,-z\n"\
"symop x+1/2,y,-z+1/2\n"\
"symop -x+1/2,y+1/2,z\n"\
"symop x,-y+1/2,z+1/2\n"\
"symop -z,-x,-y\n"\
"symop z+1/2,x,-y+1/2\n"\
"symop -z+1/2,x+1/2,y\n"\
"symop z,-x+1/2,y+1/2\n"\
"symop -y,-z,-x\n"\
"symop -y+1/2,z+1/2,x\n"\
"symop y,-z+1/2,x+1/2\n"\
"symop y+1/2,z,-x+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  206\n"\
"basisop x,y,z\n"\
"symbol ccp4 206\n"\
"symbol Hall '-I 2b 2c 3'\n"\
"symbol xHM  'I a -3'\n"\
"symbol old  'I 21/a -3' 'I a -3'\n"\
"symbol laue '-P 2 2 3' 'm-3'\n"\
"symbol patt '-I 2 2 3' 'm-3'\n"\
"symbol pgrp '-P 2 2 3' 'm-3'\n"\
"hklasu ccp4 'h>=0 and ((l>=h and k>h) or (l=h and k=h))'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop -x,-y+1/2,z\n"\
"symop x,-y,-z+1/2\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop z,x,y\n"\
"symop -z,-x+1/2,y\n"\
"symop z,-x,-y+1/2\n"\
"symop -z,x+1/2,-y+1/2\n"\
"symop y,z,x\n"\
"symop y,-z,-x+1/2\n"\
"symop -y,z+1/2,-x+1/2\n"\
"symop -y+1/2,-z,x+1/2\n"\
"symop -x,-y,-z\n"\
"symop x,y+1/2,-z\n"\
"symop -x,y,z+1/2\n"\
"symop x,-y+1/2,z+1/2\n"\
"symop -z,-x,-y\n"\
"symop z,x+1/2,-y\n"\
"symop -z,x,y+1/2\n"\
"symop z,-x+1/2,y+1/2\n"\
"symop -y,-z,-x\n"\
"symop -y,z,x+1/2\n"\
"symop y,-z+1/2,x+1/2\n"\
"symop y+1/2,z,-x+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  207\n"\
"basisop x,y,z\n"\
"symbol ccp4 207\n"\
"symbol Hall ' P 4 2 3'\n"\
"symbol xHM  'P 4 3 2'\n"\
"symbol old  'P 4 3 2'\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-P 4 2 3' 'm-3m'\n"\
"symbol pgrp ' P 4 2 3' '432'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<1; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop x,-y,-z\n"\
"symop y,x,-z\n"\
"symop -x,y,-z\n"\
"symop -y,-x,-z\n"\
"symop z,x,y\n"\
"symop -x,z,y\n"\
"symop -z,-x,y\n"\
"symop x,-z,y\n"\
"symop z,-x,-y\n"\
"symop x,z,-y\n"\
"symop -z,x,-y\n"\
"symop -x,-z,-y\n"\
"symop y,z,x\n"\
"symop y,-z,-x\n"\
"symop z,y,-x\n"\
"symop -y,z,-x\n"\
"symop -z,-y,-x\n"\
"symop -y,-z,x\n"\
"symop z,-y,x\n"\
"symop -z,y,x\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  208\n"\
"basisop x,y,z\n"\
"symbol ccp4 208\n"\
"symbol Hall ' P 4n 2 3'\n"\
"symbol xHM  'P 42 3 2'\n"\
"symbol old  'P 42 3 2'\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-P 4 2 3' 'm-3m'\n"\
"symbol pgrp ' P 4 2 3' '432'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<1; 0<=z<=1/4\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop -y+1/2,x+1/2,z+1/2\n"\
"symop -x,-y,z\n"\
"symop y+1/2,-x+1/2,z+1/2\n"\
"symop x,-y,-z\n"\
"symop y+1/2,x+1/2,-z+1/2\n"\
"symop -x,y,-z\n"\
"symop -y+1/2,-x+1/2,-z+1/2\n"\
"symop z,x,y\n"\
"symop -x+1/2,z+1/2,y+1/2\n"\
"symop -z,-x,y\n"\
"symop x+1/2,-z+1/2,y+1/2\n"\
"symop z,-x,-y\n"\
"symop x+1/2,z+1/2,-y+1/2\n"\
"symop -z,x,-y\n"\
"symop -x+1/2,-z+1/2,-y+1/2\n"\
"symop y,z,x\n"\
"symop y,-z,-x\n"\
"symop z+1/2,y+1/2,-x+1/2\n"\
"symop -y,z,-x\n"\
"symop -z+1/2,-y+1/2,-x+1/2\n"\
"symop -y,-z,x\n"\
"symop z+1/2,-y+1/2,x+1/2\n"\
"symop -z+1/2,y+1/2,x+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  209\n"\
"basisop x,y,z\n"\
"symbol ccp4 209\n"\
"symbol Hall ' F 4 2 3'\n"\
"symbol xHM  'F 4 3 2'\n"\
"symbol old  'F 4 3 2'\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-F 4 2 3' 'm-3m'\n"\
"symbol pgrp ' P 4 2 3' '432'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop x,-y,-z\n"\
"symop y,x,-z\n"\
"symop -x,y,-z\n"\
"symop -y,-x,-z\n"\
"symop z,x,y\n"\
"symop -x,z,y\n"\
"symop -z,-x,y\n"\
"symop x,-z,y\n"\
"symop z,-x,-y\n"\
"symop x,z,-y\n"\
"symop -z,x,-y\n"\
"symop -x,-z,-y\n"\
"symop y,z,x\n"\
"symop y,-z,-x\n"\
"symop z,y,-x\n"\
"symop -y,z,-x\n"\
"symop -z,-y,-x\n"\
"symop -y,-z,x\n"\
"symop z,-y,x\n"\
"symop -z,y,x\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"cenop x+1/2,y,z+1/2\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  210\n"\
"basisop x,y,z\n"\
"symbol ccp4 210\n"\
"symbol Hall ' F 4d 2 3'\n"\
"symbol xHM  'F 41 3 2'\n"\
"symbol old  'F 41 3 2'\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-F 4 2 3' 'm-3m'\n"\
"symbol pgrp ' P 4 2 3' '432'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<1; 0<=z<=1/8\n"\
"mapasu zero 0<=x<=1/8; 0<=y<=1/8; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/8; 0<=y<=1/8; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/4,x+1/4,z+1/4\n"\
"symop -x,-y+1/2,z+1/2\n"\
"symop y+3/4,-x+1/4,z+3/4\n"\
"symop x,-y,-z\n"\
"symop y+1/4,x+1/4,-z+1/4\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop -y+3/4,-x+1/4,-z+3/4\n"\
"symop z,x,y\n"\
"symop -x+1/4,z+1/4,y+1/4\n"\
"symop -z,-x+1/2,y+1/2\n"\
"symop x+3/4,-z+1/4,y+3/4\n"\
"symop z,-x,-y\n"\
"symop x+1/4,z+1/4,-y+1/4\n"\
"symop -z,x+1/2,-y+1/2\n"\
"symop -x+3/4,-z+1/4,-y+3/4\n"\
"symop y,z,x\n"\
"symop y+1/2,-z,-x+1/2\n"\
"symop z+1/4,y+3/4,-x+3/4\n"\
"symop -y+1/2,z+1/2,-x\n"\
"symop -z+1/4,-y+1/4,-x+1/4\n"\
"symop -y,-z,x\n"\
"symop z+1/4,-y+3/4,x+3/4\n"\
"symop -z+3/4,y+3/4,x+1/4\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"cenop x+1/2,y,z+1/2\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  211\n"\
"basisop x,y,z\n"\
"symbol ccp4 211\n"\
"symbol Hall ' I 4 2 3'\n"\
"symbol xHM  'I 4 3 2'\n"\
"symbol old  'I 4 3 2'\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-I 4 2 3' 'm-3m'\n"\
"symbol pgrp ' P 4 2 3' '432'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/4\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop x,-y,-z\n"\
"symop y,x,-z\n"\
"symop -x,y,-z\n"\
"symop -y,-x,-z\n"\
"symop z,x,y\n"\
"symop -x,z,y\n"\
"symop -z,-x,y\n"\
"symop x,-z,y\n"\
"symop z,-x,-y\n"\
"symop x,z,-y\n"\
"symop -z,x,-y\n"\
"symop -x,-z,-y\n"\
"symop y,z,x\n"\
"symop y,-z,-x\n"\
"symop z,y,-x\n"\
"symop -y,z,-x\n"\
"symop -z,-y,-x\n"\
"symop -y,-z,x\n"\
"symop z,-y,x\n"\
"symop -z,y,x\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  212\n"\
"basisop x,y,z\n"\
"symbol ccp4 212\n"\
"symbol Hall ' P 4acd 2ab 3'\n"\
"symbol xHM  'P 43 3 2'\n"\
"symbol old  'P 43 3 2'\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-P 4 2 3' 'm-3m'\n"\
"symbol pgrp ' P 4 2 3' '432'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<=1/8\n"\
"mapasu zero 0<=x<1; 0<=y<=1/8; 0<=z<1\n"\
"mapasu nonz 1/8<=x<=3/8; 1/8<=y<=3/8; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop -y+3/4,x+1/4,z+3/4\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop y+3/4,-x+3/4,z+1/4\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop y+1/4,x+3/4,-z+3/4\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop -y+1/4,-x+1/4,-z+1/4\n"\
"symop z,x,y\n"\
"symop -x+3/4,z+1/4,y+3/4\n"\
"symop -z+1/2,-x,y+1/2\n"\
"symop x+3/4,-z+3/4,y+1/4\n"\
"symop z+1/2,-x+1/2,-y\n"\
"symop x+1/4,z+3/4,-y+3/4\n"\
"symop -z,x+1/2,-y+1/2\n"\
"symop -x+1/4,-z+1/4,-y+1/4\n"\
"symop y,z,x\n"\
"symop y+1/2,-z+1/2,-x\n"\
"symop z+1/4,y+3/4,-x+3/4\n"\
"symop -y,z+1/2,-x+1/2\n"\
"symop -z+1/4,-y+1/4,-x+1/4\n"\
"symop -y+1/2,-z,x+1/2\n"\
"symop z+3/4,-y+3/4,x+1/4\n"\
"symop -z+3/4,y+1/4,x+3/4\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  213\n"\
"basisop x,y,z\n"\
"symbol ccp4 213\n"\
"symbol Hall ' P 4bd 2ab 3'\n"\
"symbol xHM  'P 41 3 2'\n"\
"symbol old  'P 41 3 2'\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-P 4 2 3' 'm-3m'\n"\
"symbol pgrp ' P 4 2 3' '432'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<=1/8\n"\
"mapasu zero 0<=x<1; 0<=y<=1/8; 0<=z<1\n"\
"mapasu nonz 1/8<=x<=3/8; 1/8<=y<=3/8; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop -y+1/4,x+3/4,z+1/4\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop y+1/4,-x+1/4,z+3/4\n"\
"symop x+1/2,-y+1/2,-z\n"\
"symop y+3/4,x+1/4,-z+1/4\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop -y+3/4,-x+3/4,-z+3/4\n"\
"symop z,x,y\n"\
"symop -x+1/4,z+3/4,y+1/4\n"\
"symop -z+1/2,-x,y+1/2\n"\
"symop x+1/4,-z+1/4,y+3/4\n"\
"symop z+1/2,-x+1/2,-y\n"\
"symop x+3/4,z+1/4,-y+1/4\n"\
"symop -z,x+1/2,-y+1/2\n"\
"symop -x+3/4,-z+3/4,-y+3/4\n"\
"symop y,z,x\n"\
"symop y+1/2,-z+1/2,-x\n"\
"symop z+3/4,y+1/4,-x+1/4\n"\
"symop -y,z+1/2,-x+1/2\n"\
"symop -z+3/4,-y+3/4,-x+3/4\n"\
"symop -y+1/2,-z,x+1/2\n"\
"symop z+1/4,-y+1/4,x+3/4\n"\
"symop -z+1/4,y+3/4,x+1/4\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  214\n"\
"basisop x,y,z\n"\
"symbol ccp4 214\n"\
"symbol Hall ' I 4bd 2c 3'\n"\
"symbol xHM  'I 41 3 2'\n"\
"symbol old  'I 41 3 2'\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-I 4 2 3' 'm-3m'\n"\
"symbol pgrp ' P 4 2 3' '432'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<1; 0<=z<=1/8\n"\
"mapasu zero 0<=x<1; 0<=y<=1/8; 0<=z<1/2\n"\
"mapasu nonz -1/8<=x<=1/8; 0<=y<=1/8; 0<z<7/8\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop -y+1/4,x+3/4,z+1/4\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop y+1/4,-x+1/4,z+3/4\n"\
"symop x,-y,-z+1/2\n"\
"symop y+1/4,x+3/4,-z+3/4\n"\
"symop -x+1/2,y,-z\n"\
"symop -y+1/4,-x+1/4,-z+1/4\n"\
"symop z,x,y\n"\
"symop -x+1/4,z+3/4,y+1/4\n"\
"symop -z+1/2,-x,y+1/2\n"\
"symop x+1/4,-z+1/4,y+3/4\n"\
"symop z,-x,-y+1/2\n"\
"symop x+1/4,z+3/4,-y+3/4\n"\
"symop -z+1/2,x,-y\n"\
"symop -x+1/4,-z+1/4,-y+1/4\n"\
"symop y,z,x\n"\
"symop y+1/2,-z+1/2,-x\n"\
"symop z+3/4,y+1/4,-x+1/4\n"\
"symop -y,z+1/2,-x+1/2\n"\
"symop -z+1/4,-y+1/4,-x+1/4\n"\
"symop -y+1/2,-z,x+1/2\n"\
"symop z+3/4,-y+3/4,x+1/4\n"\
"symop -z+3/4,y+1/4,x+3/4\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  215\n"\
"basisop x,y,z\n"\
"symbol ccp4 215\n"\
"symbol Hall ' P -4 2 3'\n"\
"symbol xHM  'P -4 3 m'\n"\
"symbol old  'P -4 3 m'\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-P 4 2 3' 'm-3m'\n"\
"symbol pgrp ' P -4 2 3' '-43m'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop y,-x,-z\n"\
"symop -x,-y,z\n"\
"symop -y,x,-z\n"\
"symop x,-y,-z\n"\
"symop -y,-x,z\n"\
"symop -x,y,-z\n"\
"symop y,x,z\n"\
"symop z,x,y\n"\
"symop x,-z,-y\n"\
"symop -z,-x,y\n"\
"symop -x,z,-y\n"\
"symop z,-x,-y\n"\
"symop -x,-z,y\n"\
"symop -z,x,-y\n"\
"symop x,z,y\n"\
"symop y,z,x\n"\
"symop y,-z,-x\n"\
"symop -z,-y,x\n"\
"symop -y,z,-x\n"\
"symop z,y,x\n"\
"symop -y,-z,x\n"\
"symop -z,y,-x\n"\
"symop z,-y,-x\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  216\n"\
"basisop x,y,z\n"\
"symbol ccp4 216\n"\
"symbol Hall ' F -4 2 3'\n"\
"symbol xHM  'F -4 3 m'\n"\
"symbol old  'F -4 3 m'\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-F 4 2 3' 'm-3m'\n"\
"symbol pgrp ' P -4 2 3' '-43m'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop y,-x,-z\n"\
"symop -x,-y,z\n"\
"symop -y,x,-z\n"\
"symop x,-y,-z\n"\
"symop -y,-x,z\n"\
"symop -x,y,-z\n"\
"symop y,x,z\n"\
"symop z,x,y\n"\
"symop x,-z,-y\n"\
"symop -z,-x,y\n"\
"symop -x,z,-y\n"\
"symop z,-x,-y\n"\
"symop -x,-z,y\n"\
"symop -z,x,-y\n"\
"symop x,z,y\n"\
"symop y,z,x\n"\
"symop y,-z,-x\n"\
"symop -z,-y,x\n"\
"symop -y,z,-x\n"\
"symop z,y,x\n"\
"symop -y,-z,x\n"\
"symop -z,y,-x\n"\
"symop z,-y,-x\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"cenop x+1/2,y,z+1/2\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  217\n"\
"basisop x,y,z\n"\
"symbol ccp4 217\n"\
"symbol Hall ' I -4 2 3'\n"\
"symbol xHM  'I -4 3 m'\n"\
"symbol old  'I -4 3 m'\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-I 4 2 3' 'm-3m'\n"\
"symbol pgrp ' P -4 2 3' '-43m'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop y,-x,-z\n"\
"symop -x,-y,z\n"\
"symop -y,x,-z\n"\
"symop x,-y,-z\n"\
"symop -y,-x,z\n"\
"symop -x,y,-z\n"\
"symop y,x,z\n"\
"symop z,x,y\n"\
"symop x,-z,-y\n"\
"symop -z,-x,y\n"\
"symop -x,z,-y\n"\
"symop z,-x,-y\n"\
"symop -x,-z,y\n"\
"symop -z,x,-y\n"\
"symop x,z,y\n"\
"symop y,z,x\n"\
"symop y,-z,-x\n"\
"symop -z,-y,x\n"\
"symop -y,z,-x\n"\
"symop z,y,x\n"\
"symop -y,-z,x\n"\
"symop -z,y,-x\n"\
"symop z,-y,-x\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  218\n"\
"basisop x,y,z\n"\
"symbol ccp4 218\n"\
"symbol Hall ' P -4n 2 3'\n"\
"symbol xHM  'P -4 3 n'\n"\
"symbol old  'P -4 3 n'\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-P 4 2 3' 'm-3m'\n"\
"symbol pgrp ' P -4 2 3' '-43m'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop y+1/2,-x+1/2,-z+1/2\n"\
"symop -x,-y,z\n"\
"symop -y+1/2,x+1/2,-z+1/2\n"\
"symop x,-y,-z\n"\
"symop -y+1/2,-x+1/2,z+1/2\n"\
"symop -x,y,-z\n"\
"symop y+1/2,x+1/2,z+1/2\n"\
"symop z,x,y\n"\
"symop x+1/2,-z+1/2,-y+1/2\n"\
"symop -z,-x,y\n"\
"symop -x+1/2,z+1/2,-y+1/2\n"\
"symop z,-x,-y\n"\
"symop -x+1/2,-z+1/2,y+1/2\n"\
"symop -z,x,-y\n"\
"symop x+1/2,z+1/2,y+1/2\n"\
"symop y,z,x\n"\
"symop y,-z,-x\n"\
"symop -z+1/2,-y+1/2,x+1/2\n"\
"symop -y,z,-x\n"\
"symop z+1/2,y+1/2,x+1/2\n"\
"symop -y,-z,x\n"\
"symop -z+1/2,y+1/2,-x+1/2\n"\
"symop z+1/2,-y+1/2,-x+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  219\n"\
"basisop x,y,z\n"\
"symbol ccp4 219\n"\
"symbol Hall ' F -4a 2 3'\n"\
"symbol xHM  'F -4 3 c'\n"\
"symbol old  'F -4 3 c'\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-F 4 2 3' 'm-3m'\n"\
"symbol pgrp ' P -4 2 3' '-43m'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop y+1/2,-x,-z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop -y,x+1/2,-z\n"\
"symop x,-y,-z\n"\
"symop -y+1/2,-x,z\n"\
"symop -x+1/2,y+1/2,-z\n"\
"symop y,x+1/2,z\n"\
"symop z,x,y\n"\
"symop x+1/2,-z,-y\n"\
"symop -z+1/2,-x+1/2,y\n"\
"symop -x,z+1/2,-y\n"\
"symop z,-x,-y\n"\
"symop -x+1/2,-z,y\n"\
"symop -z+1/2,x+1/2,-y\n"\
"symop x,z+1/2,y\n"\
"symop y,z,x\n"\
"symop y,-z+1/2,-x+1/2\n"\
"symop -z,-y,x+1/2\n"\
"symop -y+1/2,z,-x+1/2\n"\
"symop z+1/2,y,x\n"\
"symop -y,-z,x\n"\
"symop -z,y,-x+1/2\n"\
"symop z+1/2,-y+1/2,-x+1/2\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"cenop x+1/2,y,z+1/2\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  220\n"\
"basisop x,y,z\n"\
"symbol ccp4 220\n"\
"symbol Hall ' I -4bd 2c 3'\n"\
"symbol xHM  'I -4 3 d'\n"\
"symbol old  'I -4 3 d'\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-I 4 2 3' 'm-3m'\n"\
"symbol pgrp ' P -4 2 3' '-43m'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz -1/8<=x<=1/8; 0<=y<=1/8; 0<z<7/8\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop y+1/4,-x+3/4,-z+1/4\n"\
"symop -x,-y+1/2,z\n"\
"symop -y+3/4,x+3/4,-z+1/4\n"\
"symop x,-y,-z+1/2\n"\
"symop -y+1/4,-x+3/4,z+3/4\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop y+3/4,x+3/4,z+3/4\n"\
"symop z,x,y\n"\
"symop x+1/4,-z+3/4,-y+1/4\n"\
"symop -z,-x+1/2,y\n"\
"symop -x+3/4,z+3/4,-y+1/4\n"\
"symop z,-x,-y+1/2\n"\
"symop -x+1/4,-z+3/4,y+3/4\n"\
"symop -z,x+1/2,-y+1/2\n"\
"symop x+3/4,z+3/4,y+3/4\n"\
"symop y,z,x\n"\
"symop y,-z,-x+1/2\n"\
"symop -z+1/4,-y+3/4,x+3/4\n"\
"symop -y,z+1/2,-x+1/2\n"\
"symop z+1/4,y+1/4,x+1/4\n"\
"symop -y+1/2,-z,x+1/2\n"\
"symop -z+1/4,y+1/4,-x+3/4\n"\
"symop z+3/4,-y+1/4,-x+3/4\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  221\n"\
"basisop x,y,z\n"\
"symbol ccp4 221\n"\
"symbol Hall '-P 4 2 3'\n"\
"symbol xHM  'P m -3 m'\n"\
"symbol old  'P 4/m -3 2/m' 'P m -3 m'\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-P 4 2 3' 'm-3m'\n"\
"symbol pgrp '-P 4 2 3' 'm-3m'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop x,-y,-z\n"\
"symop y,x,-z\n"\
"symop -x,y,-z\n"\
"symop -y,-x,-z\n"\
"symop z,x,y\n"\
"symop -x,z,y\n"\
"symop -z,-x,y\n"\
"symop x,-z,y\n"\
"symop z,-x,-y\n"\
"symop x,z,-y\n"\
"symop -z,x,-y\n"\
"symop -x,-z,-y\n"\
"symop y,z,x\n"\
"symop y,-z,-x\n"\
"symop z,y,-x\n"\
"symop -y,z,-x\n"\
"symop -z,-y,-x\n"\
"symop -y,-z,x\n"\
"symop z,-y,x\n"\
"symop -z,y,x\n"\
"symop -x,-y,-z\n"\
"symop y,-x,-z\n"\
"symop x,y,-z\n"\
"symop -y,x,-z\n"\
"symop -x,y,z\n"\
"symop -y,-x,z\n"\
"symop x,-y,z\n"\
"symop y,x,z\n"\
"symop -z,-x,-y\n"\
"symop x,-z,-y\n"\
"symop z,x,-y\n"\
"symop -x,z,-y\n"\
"symop -z,x,y\n"\
"symop -x,-z,y\n"\
"symop z,-x,y\n"\
"symop x,z,y\n"\
"symop -y,-z,-x\n"\
"symop -y,z,x\n"\
"symop -z,-y,x\n"\
"symop y,-z,x\n"\
"symop z,y,x\n"\
"symop y,z,-x\n"\
"symop -z,y,-x\n"\
"symop z,-y,-x\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  222\n"\
"basisop x-1/4,y-1/4,z-1/4\n"\
"symbol ccp4 222\n"\
"symbol Hall '-P 4a 2bc 3 (x-1/4,y-1/4,z-1/4)'\n"\
"symbol xHM  'P n -3 n :1'\n"\
"symbol old  'P 4/n -3 2/n' 'P n -3 n'\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-P 4 2 3' 'm-3m'\n"\
"symbol pgrp '-P 4 2 3' 'm-3m'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop x,-y,-z\n"\
"symop y,x,-z\n"\
"symop -x,y,-z\n"\
"symop -y,-x,-z\n"\
"symop z,x,y\n"\
"symop -x,z,y\n"\
"symop -z,-x,y\n"\
"symop x,-z,y\n"\
"symop z,-x,-y\n"\
"symop x,z,-y\n"\
"symop -z,x,-y\n"\
"symop -x,-z,-y\n"\
"symop y,z,x\n"\
"symop y,-z,-x\n"\
"symop z,y,-x\n"\
"symop -y,z,-x\n"\
"symop -z,-y,-x\n"\
"symop -y,-z,x\n"\
"symop z,-y,x\n"\
"symop -z,y,x\n"\
"symop -x+1/2,-y+1/2,-z+1/2\n"\
"symop y+1/2,-x+1/2,-z+1/2\n"\
"symop x+1/2,y+1/2,-z+1/2\n"\
"symop -y+1/2,x+1/2,-z+1/2\n"\
"symop -x+1/2,y+1/2,z+1/2\n"\
"symop -y+1/2,-x+1/2,z+1/2\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"symop y+1/2,x+1/2,z+1/2\n"\
"symop -z+1/2,-x+1/2,-y+1/2\n"\
"symop x+1/2,-z+1/2,-y+1/2\n"\
"symop z+1/2,x+1/2,-y+1/2\n"\
"symop -x+1/2,z+1/2,-y+1/2\n"\
"symop -z+1/2,x+1/2,y+1/2\n"\
"symop -x+1/2,-z+1/2,y+1/2\n"\
"symop z+1/2,-x+1/2,y+1/2\n"\
"symop x+1/2,z+1/2,y+1/2\n"\
"symop -y+1/2,-z+1/2,-x+1/2\n"\
"symop -y+1/2,z+1/2,x+1/2\n"\
"symop -z+1/2,-y+1/2,x+1/2\n"\
"symop y+1/2,-z+1/2,x+1/2\n"\
"symop z+1/2,y+1/2,x+1/2\n"\
"symop y+1/2,z+1/2,-x+1/2\n"\
"symop -z+1/2,y+1/2,-x+1/2\n"\
"symop z+1/2,-y+1/2,-x+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  222\n"\
"basisop x,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 4a 2bc 3'\n"\
"symbol xHM  'P n -3 n :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-P 4 2 3' 'm-3m'\n"\
"symbol pgrp '-P 4 2 3' 'm-3m'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 1/4<=z<=3/4\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop -y+1/2,x,z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop y,-x+1/2,z\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop y,x,-z+1/2\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop -y+1/2,-x+1/2,-z+1/2\n"\
"symop z,x,y\n"\
"symop -x+1/2,z,y\n"\
"symop -z+1/2,-x+1/2,y\n"\
"symop x,-z+1/2,y\n"\
"symop z,-x+1/2,-y+1/2\n"\
"symop x,z,-y+1/2\n"\
"symop -z+1/2,x,-y+1/2\n"\
"symop -x+1/2,-z+1/2,-y+1/2\n"\
"symop y,z,x\n"\
"symop y,-z+1/2,-x+1/2\n"\
"symop z,y,-x+1/2\n"\
"symop -y+1/2,z,-x+1/2\n"\
"symop -z+1/2,-y+1/2,-x+1/2\n"\
"symop -y+1/2,-z+1/2,x\n"\
"symop z,-y+1/2,x\n"\
"symop -z+1/2,y,x\n"\
"symop -x,-y,-z\n"\
"symop y+1/2,-x,-z\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -y,x+1/2,-z\n"\
"symop -x,y+1/2,z+1/2\n"\
"symop -y,-x,z+1/2\n"\
"symop x+1/2,-y,z+1/2\n"\
"symop y+1/2,x+1/2,z+1/2\n"\
"symop -z,-x,-y\n"\
"symop x+1/2,-z,-y\n"\
"symop z+1/2,x+1/2,-y\n"\
"symop -x,z+1/2,-y\n"\
"symop -z,x+1/2,y+1/2\n"\
"symop -x,-z,y+1/2\n"\
"symop z+1/2,-x,y+1/2\n"\
"symop x+1/2,z+1/2,y+1/2\n"\
"symop -y,-z,-x\n"\
"symop -y,z+1/2,x+1/2\n"\
"symop -z,-y,x+1/2\n"\
"symop y+1/2,-z,x+1/2\n"\
"symop z+1/2,y+1/2,x+1/2\n"\
"symop y+1/2,z+1/2,-x\n"\
"symop -z,y+1/2,-x\n"\
"symop z+1/2,-y,-x\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  223\n"\
"basisop x,y,z\n"\
"symbol ccp4 223\n"\
"symbol Hall '-P 4n 2 3'\n"\
"symbol xHM  'P m -3 n'\n"\
"symbol old  'P 42/m -3 2/n' 'P m -3 n'\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-P 4 2 3' 'm-3m'\n"\
"symbol pgrp '-P 4 2 3' 'm-3m'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop -y+1/2,x+1/2,z+1/2\n"\
"symop -x,-y,z\n"\
"symop y+1/2,-x+1/2,z+1/2\n"\
"symop x,-y,-z\n"\
"symop y+1/2,x+1/2,-z+1/2\n"\
"symop -x,y,-z\n"\
"symop -y+1/2,-x+1/2,-z+1/2\n"\
"symop z,x,y\n"\
"symop -x+1/2,z+1/2,y+1/2\n"\
"symop -z,-x,y\n"\
"symop x+1/2,-z+1/2,y+1/2\n"\
"symop z,-x,-y\n"\
"symop x+1/2,z+1/2,-y+1/2\n"\
"symop -z,x,-y\n"\
"symop -x+1/2,-z+1/2,-y+1/2\n"\
"symop y,z,x\n"\
"symop y,-z,-x\n"\
"symop z+1/2,y+1/2,-x+1/2\n"\
"symop -y,z,-x\n"\
"symop -z+1/2,-y+1/2,-x+1/2\n"\
"symop -y,-z,x\n"\
"symop z+1/2,-y+1/2,x+1/2\n"\
"symop -z+1/2,y+1/2,x+1/2\n"\
"symop -x,-y,-z\n"\
"symop y+1/2,-x+1/2,-z+1/2\n"\
"symop x,y,-z\n"\
"symop -y+1/2,x+1/2,-z+1/2\n"\
"symop -x,y,z\n"\
"symop -y+1/2,-x+1/2,z+1/2\n"\
"symop x,-y,z\n"\
"symop y+1/2,x+1/2,z+1/2\n"\
"symop -z,-x,-y\n"\
"symop x+1/2,-z+1/2,-y+1/2\n"\
"symop z,x,-y\n"\
"symop -x+1/2,z+1/2,-y+1/2\n"\
"symop -z,x,y\n"\
"symop -x+1/2,-z+1/2,y+1/2\n"\
"symop z,-x,y\n"\
"symop x+1/2,z+1/2,y+1/2\n"\
"symop -y,-z,-x\n"\
"symop -y,z,x\n"\
"symop -z+1/2,-y+1/2,x+1/2\n"\
"symop y,-z,x\n"\
"symop z+1/2,y+1/2,x+1/2\n"\
"symop y,z,-x\n"\
"symop -z+1/2,y+1/2,-x+1/2\n"\
"symop z+1/2,-y+1/2,-x+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  224\n"\
"basisop x+1/4,y+1/4,z+1/4\n"\
"symbol ccp4 224\n"\
"symbol Hall '-P 4bc 2bc 3 (x+1/4,y+1/4,z+1/4)'\n"\
"symbol xHM  'P n -3 m :1'\n"\
"symbol old  'P 42/n -3 2/m' 'P n -3 m'\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-P 4 2 3' 'm-3m'\n"\
"symbol pgrp '-P 4 2 3' 'm-3m'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop -y+1/2,x+1/2,z+1/2\n"\
"symop -x,-y,z\n"\
"symop y+1/2,-x+1/2,z+1/2\n"\
"symop x,-y,-z\n"\
"symop y+1/2,x+1/2,-z+1/2\n"\
"symop -x,y,-z\n"\
"symop -y+1/2,-x+1/2,-z+1/2\n"\
"symop z,x,y\n"\
"symop -x+1/2,z+1/2,y+1/2\n"\
"symop -z,-x,y\n"\
"symop x+1/2,-z+1/2,y+1/2\n"\
"symop z,-x,-y\n"\
"symop x+1/2,z+1/2,-y+1/2\n"\
"symop -z,x,-y\n"\
"symop -x+1/2,-z+1/2,-y+1/2\n"\
"symop y,z,x\n"\
"symop y,-z,-x\n"\
"symop z+1/2,y+1/2,-x+1/2\n"\
"symop -y,z,-x\n"\
"symop -z+1/2,-y+1/2,-x+1/2\n"\
"symop -y,-z,x\n"\
"symop z+1/2,-y+1/2,x+1/2\n"\
"symop -z+1/2,y+1/2,x+1/2\n"\
"symop -x+1/2,-y+1/2,-z+1/2\n"\
"symop y,-x,-z\n"\
"symop x+1/2,y+1/2,-z+1/2\n"\
"symop -y,x,-z\n"\
"symop -x+1/2,y+1/2,z+1/2\n"\
"symop -y,-x,z\n"\
"symop x+1/2,-y+1/2,z+1/2\n"\
"symop y,x,z\n"\
"symop -z+1/2,-x+1/2,-y+1/2\n"\
"symop x,-z,-y\n"\
"symop z+1/2,x+1/2,-y+1/2\n"\
"symop -x,z,-y\n"\
"symop -z+1/2,x+1/2,y+1/2\n"\
"symop -x,-z,y\n"\
"symop z+1/2,-x+1/2,y+1/2\n"\
"symop x,z,y\n"\
"symop -y+1/2,-z+1/2,-x+1/2\n"\
"symop -y+1/2,z+1/2,x+1/2\n"\
"symop -z,-y,x\n"\
"symop y+1/2,-z+1/2,x+1/2\n"\
"symop z,y,x\n"\
"symop y+1/2,z+1/2,-x+1/2\n"\
"symop -z,y,-x\n"\
"symop z,-y,-x\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  224\n"\
"basisop x,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-P 4bc 2bc 3'\n"\
"symbol xHM  'P n -3 m :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-P 4 2 3' 'm-3m'\n"\
"symbol pgrp '-P 4 2 3' 'm-3m'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop -y,x+1/2,z+1/2\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop y+1/2,-x,z+1/2\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop y+1/2,x+1/2,-z\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop -y,-x,-z\n"\
"symop z,x,y\n"\
"symop -x,z+1/2,y+1/2\n"\
"symop -z+1/2,-x+1/2,y\n"\
"symop x+1/2,-z,y+1/2\n"\
"symop z,-x+1/2,-y+1/2\n"\
"symop x+1/2,z+1/2,-y\n"\
"symop -z+1/2,x,-y+1/2\n"\
"symop -x,-z,-y\n"\
"symop y,z,x\n"\
"symop y,-z+1/2,-x+1/2\n"\
"symop z+1/2,y+1/2,-x\n"\
"symop -y+1/2,z,-x+1/2\n"\
"symop -z,-y,-x\n"\
"symop -y+1/2,-z+1/2,x\n"\
"symop z+1/2,-y,x+1/2\n"\
"symop -z,y+1/2,x+1/2\n"\
"symop -x,-y,-z\n"\
"symop y,-x+1/2,-z+1/2\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -y+1/2,x,-z+1/2\n"\
"symop -x,y+1/2,z+1/2\n"\
"symop -y+1/2,-x+1/2,z\n"\
"symop x+1/2,-y,z+1/2\n"\
"symop y,x,z\n"\
"symop -z,-x,-y\n"\
"symop x,-z+1/2,-y+1/2\n"\
"symop z+1/2,x+1/2,-y\n"\
"symop -x+1/2,z,-y+1/2\n"\
"symop -z,x+1/2,y+1/2\n"\
"symop -x+1/2,-z+1/2,y\n"\
"symop z+1/2,-x,y+1/2\n"\
"symop x,z,y\n"\
"symop -y,-z,-x\n"\
"symop -y,z+1/2,x+1/2\n"\
"symop -z+1/2,-y+1/2,x\n"\
"symop y+1/2,-z,x+1/2\n"\
"symop z,y,x\n"\
"symop y+1/2,z+1/2,-x\n"\
"symop -z+1/2,y,-x+1/2\n"\
"symop z,-y+1/2,-x+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  225\n"\
"basisop x,y,z\n"\
"symbol ccp4 225\n"\
"symbol Hall '-F 4 2 3'\n"\
"symbol xHM  'F m -3 m'\n"\
"symbol old  'F 4/m -3 2/m' 'F m -3 m'\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-F 4 2 3' 'm-3m'\n"\
"symbol pgrp '-P 4 2 3' 'm-3m'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/4; 0<=z<=1/4\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop x,-y,-z\n"\
"symop y,x,-z\n"\
"symop -x,y,-z\n"\
"symop -y,-x,-z\n"\
"symop z,x,y\n"\
"symop -x,z,y\n"\
"symop -z,-x,y\n"\
"symop x,-z,y\n"\
"symop z,-x,-y\n"\
"symop x,z,-y\n"\
"symop -z,x,-y\n"\
"symop -x,-z,-y\n"\
"symop y,z,x\n"\
"symop y,-z,-x\n"\
"symop z,y,-x\n"\
"symop -y,z,-x\n"\
"symop -z,-y,-x\n"\
"symop -y,-z,x\n"\
"symop z,-y,x\n"\
"symop -z,y,x\n"\
"symop -x,-y,-z\n"\
"symop y,-x,-z\n"\
"symop x,y,-z\n"\
"symop -y,x,-z\n"\
"symop -x,y,z\n"\
"symop -y,-x,z\n"\
"symop x,-y,z\n"\
"symop y,x,z\n"\
"symop -z,-x,-y\n"\
"symop x,-z,-y\n"\
"symop z,x,-y\n"\
"symop -x,z,-y\n"\
"symop -z,x,y\n"\
"symop -x,-z,y\n"\
"symop z,-x,y\n"\
"symop x,z,y\n"\
"symop -y,-z,-x\n"\
"symop -y,z,x\n"\
"symop -z,-y,x\n"\
"symop y,-z,x\n"\
"symop z,y,x\n"\
"symop y,z,-x\n"\
"symop -z,y,-x\n"\
"symop z,-y,-x\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"cenop x+1/2,y,z+1/2\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  226\n"\
"basisop x,y,z\n"\
"symbol ccp4 226\n"\
"symbol Hall '-F 4a 2 3'\n"\
"symbol xHM  'F m -3 c'\n"\
"symbol old  'F 4/m -3 2/c' 'F m -3 c'\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-F 4 2 3' 'm-3m'\n"\
"symbol pgrp '-P 4 2 3' 'm-3m'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<=1/4\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<=1/4\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/2,x,z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop y,-x+1/2,z\n"\
"symop x,-y,-z\n"\
"symop y+1/2,x,-z\n"\
"symop -x+1/2,y+1/2,-z\n"\
"symop -y,-x+1/2,-z\n"\
"symop z,x,y\n"\
"symop -x+1/2,z,y\n"\
"symop -z+1/2,-x+1/2,y\n"\
"symop x,-z+1/2,y\n"\
"symop z,-x,-y\n"\
"symop x+1/2,z,-y\n"\
"symop -z+1/2,x+1/2,-y\n"\
"symop -x,-z+1/2,-y\n"\
"symop y,z,x\n"\
"symop y,-z+1/2,-x+1/2\n"\
"symop z,y,-x+1/2\n"\
"symop -y+1/2,z,-x+1/2\n"\
"symop -z+1/2,-y,-x\n"\
"symop -y,-z,x\n"\
"symop z,-y,x+1/2\n"\
"symop -z+1/2,y+1/2,x+1/2\n"\
"symop -x,-y,-z\n"\
"symop y+1/2,-x,-z\n"\
"symop x+1/2,y+1/2,-z\n"\
"symop -y,x+1/2,-z\n"\
"symop -x,y,z\n"\
"symop -y+1/2,-x,z\n"\
"symop x+1/2,-y+1/2,z\n"\
"symop y,x+1/2,z\n"\
"symop -z,-x,-y\n"\
"symop x+1/2,-z,-y\n"\
"symop z+1/2,x+1/2,-y\n"\
"symop -x,z+1/2,-y\n"\
"symop -z,x,y\n"\
"symop -x+1/2,-z,y\n"\
"symop z+1/2,-x+1/2,y\n"\
"symop x,z+1/2,y\n"\
"symop -y,-z,-x\n"\
"symop -y,z+1/2,x+1/2\n"\
"symop -z,-y,x+1/2\n"\
"symop y+1/2,-z,x+1/2\n"\
"symop z+1/2,y,x\n"\
"symop y,z,-x\n"\
"symop -z,y,-x+1/2\n"\
"symop z+1/2,-y+1/2,-x+1/2\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"cenop x+1/2,y,z+1/2\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  227\n"\
"basisop x+1/8,y+1/8,z+1/8\n"\
"symbol ccp4 227\n"\
"symbol Hall '-F 4vw 2vw 3 (x+1/8,y+1/8,z+1/8)'\n"\
"symbol xHM  'F d -3 m :1'\n"\
"symbol old  'F 41/d -3 2/m' 'F d -3 m'\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-F 4 2 3' 'm-3m'\n"\
"symbol pgrp '-P 4 2 3' 'm-3m'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/8; 0<=y<=1/8; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/8; 0<=y<=1/8; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/4,x+1/4,z+1/4\n"\
"symop -x,-y+1/2,z+1/2\n"\
"symop y+3/4,-x+1/4,z+3/4\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop y+3/4,x+1/4,-z+3/4\n"\
"symop -x,y,-z\n"\
"symop -y+1/4,-x+1/4,-z+1/4\n"\
"symop z,x,y\n"\
"symop -x+1/4,z+1/4,y+1/4\n"\
"symop -z,-x+1/2,y+1/2\n"\
"symop x+3/4,-z+1/4,y+3/4\n"\
"symop z,-x+1/2,-y+1/2\n"\
"symop x+3/4,z+1/4,-y+3/4\n"\
"symop -z,x,-y\n"\
"symop -x+1/4,-z+1/4,-y+1/4\n"\
"symop y,z,x\n"\
"symop y+1/2,-z,-x+1/2\n"\
"symop z+1/4,y+3/4,-x+3/4\n"\
"symop -y+1/2,z+1/2,-x\n"\
"symop -z+1/4,-y+3/4,-x+3/4\n"\
"symop -y+1/2,-z+1/2,x\n"\
"symop z+1/4,-y+1/4,x+1/4\n"\
"symop -z+3/4,y+1/4,x+3/4\n"\
"symop -x+1/4,-y+1/4,-z+1/4\n"\
"symop y,-x,-z\n"\
"symop x+1/4,y+3/4,-z+3/4\n"\
"symop -y+1/2,x,-z+1/2\n"\
"symop -x+1/4,y+3/4,z+3/4\n"\
"symop -y+1/2,-x,z+1/2\n"\
"symop x+1/4,-y+1/4,z+1/4\n"\
"symop y,x,z\n"\
"symop -z+1/4,-x+1/4,-y+1/4\n"\
"symop x,-z,-y\n"\
"symop z+1/4,x+3/4,-y+3/4\n"\
"symop -x+1/2,z,-y+1/2\n"\
"symop -z+1/4,x+3/4,y+3/4\n"\
"symop -x+1/2,-z,y+1/2\n"\
"symop z+1/4,-x+1/4,y+1/4\n"\
"symop x,z,y\n"\
"symop -y+1/4,-z+1/4,-x+1/4\n"\
"symop -y+3/4,z+1/4,x+3/4\n"\
"symop -z,-y+1/2,x+1/2\n"\
"symop y+3/4,-z+3/4,x+1/4\n"\
"symop z,y+1/2,x+1/2\n"\
"symop y+3/4,z+3/4,-x+1/4\n"\
"symop -z,y,-x\n"\
"symop z+1/2,-y,-x+1/2\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"cenop x+1/2,y,z+1/2\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  227\n"\
"basisop x,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-F 4vw 2vw 3'\n"\
"symbol xHM  'F d -3 m :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-F 4 2 3' 'm-3m'\n"\
"symbol pgrp '-P 4 2 3' 'm-3m'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/8; 0<=y<=1/8; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/8; 0<=y<=1/8; 0<=z<1\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y,x+1/4,z+1/4\n"\
"symop -x+3/4,-y+1/4,z+1/2\n"\
"symop y+3/4,-x,z+3/4\n"\
"symop x,-y+1/4,-z+1/4\n"\
"symop y+3/4,x+1/4,-z+1/2\n"\
"symop -x+3/4,y,-z+3/4\n"\
"symop -y,-x,-z\n"\
"symop z,x,y\n"\
"symop -x,z+1/4,y+1/4\n"\
"symop -z+3/4,-x+1/4,y+1/2\n"\
"symop x+3/4,-z,y+3/4\n"\
"symop z,-x+1/4,-y+1/4\n"\
"symop x+3/4,z+1/4,-y+1/2\n"\
"symop -z+3/4,x,-y+3/4\n"\
"symop -x,-z,-y\n"\
"symop y,z,x\n"\
"symop y+1/2,-z+3/4,-x+1/4\n"\
"symop z+1/4,y+3/4,-x+1/2\n"\
"symop -y+1/4,z+1/2,-x+3/4\n"\
"symop -z,-y+1/2,-x+1/2\n"\
"symop -y+1/4,-z+1/4,x\n"\
"symop z+1/4,-y,x+1/4\n"\
"symop -z+1/2,y+1/4,x+3/4\n"\
"symop -x,-y,-z\n"\
"symop y,-x+3/4,-z+3/4\n"\
"symop x+1/4,y+3/4,-z+1/2\n"\
"symop -y+1/4,x,-z+1/4\n"\
"symop -x,y+3/4,z+3/4\n"\
"symop -y+1/4,-x+3/4,z+1/2\n"\
"symop x+1/4,-y,z+1/4\n"\
"symop y,x,z\n"\
"symop -z,-x,-y\n"\
"symop x,-z+3/4,-y+3/4\n"\
"symop z+1/4,x+3/4,-y+1/2\n"\
"symop -x+1/4,z,-y+1/4\n"\
"symop -z,x+3/4,y+3/4\n"\
"symop -x+1/4,-z+3/4,y+1/2\n"\
"symop z+1/4,-x,y+1/4\n"\
"symop x,z,y\n"\
"symop -y,-z,-x\n"\
"symop -y+1/2,z+1/4,x+3/4\n"\
"symop -z+3/4,-y+1/4,x+1/2\n"\
"symop y+3/4,-z+1/2,x+1/4\n"\
"symop z,y+1/2,x+1/2\n"\
"symop y+3/4,z+3/4,-x\n"\
"symop -z+3/4,y,-x+3/4\n"\
"symop z+1/2,-y+3/4,-x+1/4\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"cenop x+1/2,y,z+1/2\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  228\n"\
"basisop x-1/8,y-1/8,z-1/8\n"\
"symbol ccp4 228\n"\
"symbol Hall '-F 4ud 2vw 3 (x-1/8,y-1/8,z-1/8)'\n"\
"symbol xHM  'F d -3 c :1'\n"\
"symbol old  'F d -3 c' 'F 41/d -3 2/c'\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-F 4 2 3' 'm-3m'\n"\
"symbol pgrp '-P 4 2 3' 'm-3m'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/8; 0<=y<=1/8; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/8; 0<=y<=1/8; 0<=z<1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/4,x+1/4,z+1/4\n"\
"symop -x,-y+1/2,z+1/2\n"\
"symop y+3/4,-x+1/4,z+3/4\n"\
"symop x,-y,-z\n"\
"symop y+1/4,x+1/4,-z+1/4\n"\
"symop -x,y+1/2,-z+1/2\n"\
"symop -y+3/4,-x+1/4,-z+3/4\n"\
"symop z,x,y\n"\
"symop -x+1/4,z+1/4,y+1/4\n"\
"symop -z,-x+1/2,y+1/2\n"\
"symop x+3/4,-z+1/4,y+3/4\n"\
"symop z,-x,-y\n"\
"symop x+1/4,z+1/4,-y+1/4\n"\
"symop -z,x+1/2,-y+1/2\n"\
"symop -x+3/4,-z+1/4,-y+3/4\n"\
"symop y,z,x\n"\
"symop y+1/2,-z,-x+1/2\n"\
"symop z+1/4,y+3/4,-x+3/4\n"\
"symop -y+1/2,z+1/2,-x\n"\
"symop -z+1/4,-y+1/4,-x+1/4\n"\
"symop -y,-z,x\n"\
"symop z+1/4,-y+3/4,x+3/4\n"\
"symop -z+3/4,y+3/4,x+1/4\n"\
"symop -x+3/4,-y+3/4,-z+3/4\n"\
"symop y+1/2,-x+1/2,-z+1/2\n"\
"symop x+3/4,y+1/4,-z+1/4\n"\
"symop -y,x+1/2,-z\n"\
"symop -x+3/4,y+3/4,z+3/4\n"\
"symop -y+1/2,-x+1/2,z+1/2\n"\
"symop x+3/4,-y+1/4,z+1/4\n"\
"symop y,x+1/2,z\n"\
"symop -z+3/4,-x+3/4,-y+3/4\n"\
"symop x+1/2,-z+1/2,-y+1/2\n"\
"symop z+3/4,x+1/4,-y+1/4\n"\
"symop -x,z+1/2,-y\n"\
"symop -z+3/4,x+3/4,y+3/4\n"\
"symop -x+1/2,-z+1/2,y+1/2\n"\
"symop z+3/4,-x+1/4,y+1/4\n"\
"symop x,z+1/2,y\n"\
"symop -y+3/4,-z+3/4,-x+3/4\n"\
"symop -y+1/4,z+3/4,x+1/4\n"\
"symop -z+1/2,-y,x\n"\
"symop y+1/4,-z+1/4,x+3/4\n"\
"symop z+1/2,y+1/2,x+1/2\n"\
"symop y+3/4,z+3/4,-x+3/4\n"\
"symop -z+1/2,y,-x\n"\
"symop z,-y,-x+1/2\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"cenop x+1/2,y,z+1/2\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  228\n"\
"basisop x,y,z\n"\
"symbol ccp4 0\n"\
"symbol Hall '-F 4ud 2vw 3'\n"\
"symbol xHM  'F d -3 c :2'\n"\
"symbol old  ''\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-F 4 2 3' 'm-3m'\n"\
"symbol pgrp '-P 4 2 3' 'm-3m'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/8; 0<=y<=1/8; 0<=z<1/2\n"\
"mapasu nonz 0<=x<=1/8; 0<=y<=1/8; 0<=z<1/2\n"\
"cheshire 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"symop x,y,z\n"\
"symop -y+1/2,x+1/4,z+1/4\n"\
"symop -x+1/4,-y+3/4,z+1/2\n"\
"symop y+3/4,-x+1/2,z+3/4\n"\
"symop x,-y+1/4,-z+1/4\n"\
"symop y+1/4,x+1/4,-z+1/2\n"\
"symop -x+1/4,y+1/2,-z+3/4\n"\
"symop -y,-x+1/2,-z\n"\
"symop z,x,y\n"\
"symop -x+1/2,z+1/4,y+1/4\n"\
"symop -z+1/4,-x+3/4,y+1/2\n"\
"symop x+3/4,-z+1/2,y+3/4\n"\
"symop z,-x+1/4,-y+1/4\n"\
"symop x+1/4,z+1/4,-y+1/2\n"\
"symop -z+1/4,x+1/2,-y+3/4\n"\
"symop -x,-z+1/2,-y\n"\
"symop y,z,x\n"\
"symop y+1/2,-z+1/4,-x+3/4\n"\
"symop z+1/4,y+3/4,-x\n"\
"symop -y+3/4,z+1/2,-x+1/4\n"\
"symop -z+1/2,-y+1/2,-x+1/2\n"\
"symop -y+1/4,-z+1/4,x\n"\
"symop z+1/4,-y,x+3/4\n"\
"symop -z,y+3/4,x+1/4\n"\
"symop -x,-y,-z\n"\
"symop y+1/2,-x+3/4,-z+3/4\n"\
"symop x+3/4,y+1/4,-z+1/2\n"\
"symop -y+1/4,x+1/2,-z+1/4\n"\
"symop -x,y+3/4,z+3/4\n"\
"symop -y+3/4,-x+3/4,z+1/2\n"\
"symop x+3/4,-y+1/2,z+1/4\n"\
"symop y,x+1/2,z\n"\
"symop -z,-x,-y\n"\
"symop x+1/2,-z+3/4,-y+3/4\n"\
"symop z+3/4,x+1/4,-y+1/2\n"\
"symop -x+1/4,z+1/2,-y+1/4\n"\
"symop -z,x+3/4,y+3/4\n"\
"symop -x+3/4,-z+3/4,y+1/2\n"\
"symop z+3/4,-x+1/2,y+1/4\n"\
"symop x,z+1/2,y\n"\
"symop -y,-z,-x\n"\
"symop -y+1/2,z+3/4,x+1/4\n"\
"symop -z+3/4,-y+1/4,x\n"\
"symop y+1/4,-z+1/2,x+3/4\n"\
"symop z+1/2,y+1/2,x+1/2\n"\
"symop y+3/4,z+3/4,-x\n"\
"symop -z+3/4,y,-x+1/4\n"\
"symop z,-y+1/4,-x+3/4\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"cenop x+1/2,y,z+1/2\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  229\n"\
"basisop x,y,z\n"\
"symbol ccp4 229\n"\
"symbol Hall '-I 4 2 3'\n"\
"symbol xHM  'I m -3 m'\n"\
"symbol old  'I 4/m -3 2/m' 'I m -3 m'\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-I 4 2 3' 'm-3m'\n"\
"symbol pgrp '-P 4 2 3' 'm-3m'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/4\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<=1/4; 0<=y<=1/4; 0<=z<=1/2\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop -y,x,z\n"\
"symop -x,-y,z\n"\
"symop y,-x,z\n"\
"symop x,-y,-z\n"\
"symop y,x,-z\n"\
"symop -x,y,-z\n"\
"symop -y,-x,-z\n"\
"symop z,x,y\n"\
"symop -x,z,y\n"\
"symop -z,-x,y\n"\
"symop x,-z,y\n"\
"symop z,-x,-y\n"\
"symop x,z,-y\n"\
"symop -z,x,-y\n"\
"symop -x,-z,-y\n"\
"symop y,z,x\n"\
"symop y,-z,-x\n"\
"symop z,y,-x\n"\
"symop -y,z,-x\n"\
"symop -z,-y,-x\n"\
"symop -y,-z,x\n"\
"symop z,-y,x\n"\
"symop -z,y,x\n"\
"symop -x,-y,-z\n"\
"symop y,-x,-z\n"\
"symop x,y,-z\n"\
"symop -y,x,-z\n"\
"symop -x,y,z\n"\
"symop -y,-x,z\n"\
"symop x,-y,z\n"\
"symop y,x,z\n"\
"symop -z,-x,-y\n"\
"symop x,-z,-y\n"\
"symop z,x,-y\n"\
"symop -x,z,-y\n"\
"symop -z,x,y\n"\
"symop -x,-z,y\n"\
"symop z,-x,y\n"\
"symop x,z,y\n"\
"symop -y,-z,-x\n"\
"symop -y,z,x\n"\
"symop -z,-y,x\n"\
"symop y,-z,x\n"\
"symop z,y,x\n"\
"symop y,z,-x\n"\
"symop -z,y,-x\n"\
"symop z,-y,-x\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  230\n"\
"basisop x,y,z\n"\
"symbol ccp4 230\n"\
"symbol Hall '-I 4bd 2c 3'\n"\
"symbol xHM  'I a -3 d'\n"\
"symbol old  'I 41/a -3 2/d' 'I a -3 d'\n"\
"symbol laue '-P 4 2 3' 'm-3m'\n"\
"symbol patt '-I 4 2 3' 'm-3m'\n"\
"symbol pgrp '-P 4 2 3' 'm-3m'\n"\
"hklasu ccp4 'k>=l and l>=h and h>=0'\n"\
"mapasu ccp4 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"mapasu zero 0<=x<=1/8; 0<=y<=1/8; 0<=z<1\n"\
"mapasu nonz 0<=x<=1/8; -1/8<=y<=0; 1/8<z<7/8\n"\
"cheshire 0<=x<=1; 0<=y<=1; 0<=z<=1\n"\
"symop x,y,z\n"\
"symop -y+1/4,x+3/4,z+1/4\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop y+1/4,-x+1/4,z+3/4\n"\
"symop x,-y,-z+1/2\n"\
"symop y+1/4,x+3/4,-z+3/4\n"\
"symop -x+1/2,y,-z\n"\
"symop -y+1/4,-x+1/4,-z+1/4\n"\
"symop z,x,y\n"\
"symop -x+1/4,z+3/4,y+1/4\n"\
"symop -z+1/2,-x,y+1/2\n"\
"symop x+1/4,-z+1/4,y+3/4\n"\
"symop z,-x,-y+1/2\n"\
"symop x+1/4,z+3/4,-y+3/4\n"\
"symop -z+1/2,x,-y\n"\
"symop -x+1/4,-z+1/4,-y+1/4\n"\
"symop y,z,x\n"\
"symop y+1/2,-z+1/2,-x\n"\
"symop z+3/4,y+1/4,-x+1/4\n"\
"symop -y,z+1/2,-x+1/2\n"\
"symop -z+1/4,-y+1/4,-x+1/4\n"\
"symop -y+1/2,-z,x+1/2\n"\
"symop z+3/4,-y+3/4,x+1/4\n"\
"symop -z+3/4,y+1/4,x+3/4\n"\
"symop -x,-y,-z\n"\
"symop y+3/4,-x+1/4,-z+3/4\n"\
"symop x+1/2,y,-z+1/2\n"\
"symop -y+3/4,x+3/4,-z+1/4\n"\
"symop -x,y,z+1/2\n"\
"symop -y+3/4,-x+1/4,z+1/4\n"\
"symop x+1/2,-y,z\n"\
"symop y+3/4,x+3/4,z+3/4\n"\
"symop -z,-x,-y\n"\
"symop x+3/4,-z+1/4,-y+3/4\n"\
"symop z+1/2,x,-y+1/2\n"\
"symop -x+3/4,z+3/4,-y+1/4\n"\
"symop -z,x,y+1/2\n"\
"symop -x+3/4,-z+1/4,y+1/4\n"\
"symop z+1/2,-x,y\n"\
"symop x+3/4,z+3/4,y+3/4\n"\
"symop -y,-z,-x\n"\
"symop -y+1/2,z+1/2,x\n"\
"symop -z+1/4,-y+3/4,x+3/4\n"\
"symop y,-z+1/2,x+1/2\n"\
"symop z+3/4,y+3/4,x+3/4\n"\
"symop y+1/2,z,-x+1/2\n"\
"symop -z+1/4,y+1/4,-x+3/4\n"\
"symop z+1/4,-y+3/4,-x+1/4\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  5\n"\
"basisop x+1/4,y+1/4,-x+z-1/4\n"\
"symbol ccp4 5005\n"\
"symbol Hall ' C 2y (x+1/4,y+1/4,-x+z-1/4)'\n"\
"symbol xHM  'I 1 21 1'\n"\
"symbol old  'I 1 21 1'\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-I 2y' '2/m'\n"\
"symbol pgrp ' P 2y' '2'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1/2; 0<=z<1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<1; 0<=z<1\n"\
"mapasu nonz 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"cheshire\n"\
"symop x,y,z\n"\
"symop -x+1/2,y,-z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  5\n"\
"basisop x+1/4,y+1/4,z\n"\
"symbol ccp4 3005\n"\
"symbol Hall ' C 2y (x+1/4,y+1/4,z)'\n"\
"symbol xHM  'C 1 21 1'\n"\
"symbol old  'C 1 21 1'\n"\
"symbol laue '-P 2y' '2/m'\n"\
"symbol patt '-C 2y' '2/m'\n"\
"symbol pgrp ' P 2y' '2'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<=1/4; 0<=y<1; 0<=z<1\n"\
"mapasu zero 0<=x<1; 0<=y<1/2; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"cheshire\n"\
"symop x,y,z\n"\
"symop -x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  8\n"\
"basisop -x,z,y\n"\
"symbol ccp4 1008\n"\
"symbol Hall ' B -2'\n"\
"symbol xHM  'B 1 1 m'\n"\
"symbol old  'B 1 1 m'\n"\
"symbol laue '-P 2' '2/m'\n"\
"symbol patt '-B 2' '2/m'\n"\
"symbol pgrp ' P -2' 'm'\n"\
"hklasu ccp4 'k>=0 and (l>0 or (l=0 and h>=0))'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1/4; 0<=z<1\n"\
"mapasu zero 0<=x<1/2; 0<=y<=1; 0<=z<1/2\n"\
"mapasu nonz 0<=x<1/2; 0<=y<=1; 0<=z<1/2\n"\
"cheshire\n"\
"symop x,y,z\n"\
"symop x,y,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  18\n"\
"basisop x+1/4,y+1/4,z\n"\
"symbol ccp4 1018\n"\
"symbol Hall ' P 2 2ab (x+1/4,y+1/4,z)'\n"\
"symbol xHM  ''\n"\
"symbol old  'P 21 21 2 (a)'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-P 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 2' '222'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"cheshire\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop x+1/2,-y,-z\n"\
"symop -x,y+1/2,-z\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  20\n"\
"basisop x+1/4,y,z\n"\
"symbol ccp4 1020\n"\
"symbol Hall ' C 2c 2 (x+1/4,y,z)'\n"\
"symbol xHM  ''\n"\
"symbol old  'C 2 2 21a)'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 2' '222'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/2; 0<=z<1\n"\
"mapasu nonz 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"cheshire\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y,z+1/2\n"\
"symop x,-y,-z\n"\
"symop -x+1/2,y,-z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  21\n"\
"basisop x+1/4,y+1/4,z\n"\
"symbol ccp4 1021\n"\
"symbol Hall ' C 2 2 (x+1/4,y+1/4,z)'\n"\
"symbol xHM  ''\n"\
"symbol old  'C 2 2 2a'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-C 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 2' '222'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<=1/2\n"\
"mapasu nonz 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"cheshire\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop x,-y+1/2,-z\n"\
"symop -x+1/2,y,-z\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  22\n"\
"basisop x,y,z+1/4\n"\
"symbol ccp4 1022\n"\
"symbol Hall ' F 2 2 (x,y,z+1/4)'\n"\
"symbol xHM  ''\n"\
"symbol old  'F 2 2 2a'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-F 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 2' '222'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"cheshire\n"\
"symop x,y,z\n"\
"symop -x,-y,z\n"\
"symop x,-y,-z+1/2\n"\
"symop -x,y,-z+1/2\n"\
"cenop x,y,z\n"\
"cenop x,y+1/2,z+1/2\n"\
"cenop x+1/2,y,z+1/2\n"\
"cenop x+1/2,y+1/2,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  23\n"\
"basisop x-1/4,y+1/4,z-1/4\n"\
"symbol ccp4 1023\n"\
"symbol Hall ' I 2 2 (x-1/4,y+1/4,z-1/4)'\n"\
"symbol xHM  ''\n"\
"symbol old  'I 2 2 2a'\n"\
"symbol laue '-P 2 2' 'mmm'\n"\
"symbol patt '-I 2 2' 'mmm'\n"\
"symbol pgrp ' P 2 2' '222'\n"\
"hklasu ccp4 'h>=0 and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<1; 0<=z<=1/4\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"cheshire\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop -x+1/2,y,-z+1/2\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  94\n"\
"basisop x-1/4,y-1/4,z-1/4\n"\
"symbol ccp4 1094\n"\
"symbol Hall ' P 4n 2n (x-1/4,y-1/4,z-1/4)'\n"\
"symbol xHM  ''\n"\
"symbol old  'P 42 21 2a'\n"\
"symbol laue '-P 4 2' '4/mmm'\n"\
"symbol patt '-P 4 2' '4/mmm'\n"\
"symbol pgrp ' P 4 2' '422'\n"\
"hklasu ccp4 'h>=k and k>=0 and l>=0'\n"\
"mapasu ccp4 0<=x<=1/2; 0<=y<=1/2; 0<=z<=1/2\n"\
"mapasu zero 0<=x<1; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"cheshire\n"\
"symop x,y,z\n"\
"symop -y,x+1/2,z+1/2\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop y+1/2,-x,z+1/2\n"\
"symop x+1/2,-y,-z\n"\
"symop y,x,-z+1/2\n"\
"symop -x,y+1/2,-z\n"\
"symop -y+1/2,-x+1/2,-z+1/2\n"\
"cenop x,y,z\n"\
"end_spacegroup\n"\
"\n"\
"begin_spacegroup\n"\
"number  197\n"\
"basisop x+1/4,y+1/4,z+1/4\n"\
"symbol ccp4 1197\n"\
"symbol Hall ' I 2 2 3 (x+1/4,y+1/4,z+1/4)'\n"\
"symbol xHM  ''\n"\
"symbol old  'I 2 3a'\n"\
"symbol laue '-P 2 2 3' 'm-3'\n"\
"symbol patt '-I 2 2 3' 'm-3'\n"\
"symbol pgrp ' P 2 2 3' '23'\n"\
"hklasu ccp4 'h>=0 and ((l>=h and k>h) or (l=h and k=h))'\n"\
"mapasu ccp4 0<=x<1; 0<=y<1; 0<=z<=1/2\n"\
"mapasu zero 0<=x<=1/4; 0<=y<=1/4; 0<=z<1\n"\
"mapasu nonz 0<=x<-1; 0<=y<-1; 0<=z<-1\n"\
"cheshire\n"\
"symop x,y,z\n"\
"symop -x+1/2,-y+1/2,z\n"\
"symop x,-y+1/2,-z+1/2\n"\
"symop -x+1/2,y,-z+1/2\n"\
"symop z,x,y\n"\
"symop -z+1/2,-x+1/2,y\n"\
"symop z,-x+1/2,-y+1/2\n"\
"symop -z+1/2,x,-y+1/2\n"\
"symop y,z,x\n"\
"symop y,-z+1/2,-x+1/2\n"\
"symop -y+1/2,z,-x+1/2\n"\
"symop -y+1/2,-z+1/2,x\n"\
"cenop x,y,z\n"\
"cenop x+1/2,y+1/2,z+1/2\n"\
"end_spacegroup\n";
