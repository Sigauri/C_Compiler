delim		[ \t\n]
ws			{delim}+
letter		[(A-Za-z)_]
digit		[0-9]

%%

{ws}		{}
if 			{return(IF);}

%%