/*	SccsId:	%W%	%G%	*/
 
#define WFCONV_WCS30 "%-6.6s %-8.8s %8ld %1s %-2.2s %-2.2s %8ld %1s %-2.2s %-2.2s \
%8ld %1s %8ld %-17.17s\n"
 
#define WFCONV_WVL30(SP) \
(SP)->sta, (SP)->chan, (SP)->chanid, (SP)->inauth, (SP)->incomp, (SP)->intype, (SP)->insamp, \
(SP)->outauth, (SP)->outcomp, (SP)->outtype, (SP)->outsamp, \
(SP)->strip, (SP)->commid, (SP)->lddate

