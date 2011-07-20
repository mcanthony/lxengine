//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2011 athile@athile.net (http://www.athile.net)

    Permission is hereby granted, free of charge, to any person obtaining a 
    copy of this software and associated documentation files (the "Software"), 
    to deal in the Software without restriction, including without limitation 
    the rights to use, copy, modify, merge, publish, distribute, sublicense, 
    and/or sell copies of the Software, and to permit persons to whom the 
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
    IN THE SOFTWARE.
*/
//===========================================================================//

#pragma once

#define _MAKE_ID__ '_'
#define _MAKE_ID_0 '0'
#define _MAKE_ID_1 '1'
#define _MAKE_ID_2 '2'
#define _MAKE_ID_3 '3'
#define _MAKE_ID_4 '4'
#define _MAKE_ID_5 '5'
#define _MAKE_ID_6 '6'
#define _MAKE_ID_7 '7'
#define _MAKE_ID_8 '8'
#define _MAKE_ID_9 '9'
#define _MAKE_ID_A 'A'
#define _MAKE_ID_B 'B'
#define _MAKE_ID_C 'C'
#define _MAKE_ID_D 'D'
#define _MAKE_ID_E 'E'
#define _MAKE_ID_F 'F'
#define _MAKE_ID_G 'G'
#define _MAKE_ID_H 'H'
#define _MAKE_ID_I 'I'
#define _MAKE_ID_J 'J'
#define _MAKE_ID_K 'K'
#define _MAKE_ID_L 'L'
#define _MAKE_ID_M 'M'
#define _MAKE_ID_N 'N'
#define _MAKE_ID_O 'O'
#define _MAKE_ID_P 'P'
#define _MAKE_ID_Q 'Q'
#define _MAKE_ID_R 'R'
#define _MAKE_ID_S 'S'
#define _MAKE_ID_T 'T'
#define _MAKE_ID_U 'U'
#define _MAKE_ID_V 'V'
#define _MAKE_ID_W 'W'
#define _MAKE_ID_X 'X'
#define _MAKE_ID_Y 'Y'
#define _MAKE_ID_Z 'Z'

#define _MAKE_ID(a,b,c,d) kId_ ## a ## b ## c ## d = ( (_MAKE_ID_ ## d << 24) | (_MAKE_ID_ ## c << 16) | (_MAKE_ID_ ## b << 8) | (_MAKE_ID_ ## a) ) 

enum
{
    _MAKE_ID(A,C,T,I),
    _MAKE_ID(A,M,B,I),
    _MAKE_ID(A,P,P,A),
    _MAKE_ID(A,R,M,O),
	_MAKE_ID(A,L,C,H),
	_MAKE_ID(B,O,D,Y),
    _MAKE_ID(B,O,O,K),
	_MAKE_ID(B,S,G,N),
    _MAKE_ID(C,E,L,L),
	_MAKE_ID(C,L,A,S),
    _MAKE_ID(C,L,O,T),
    _MAKE_ID(C,O,N,T),
    _MAKE_ID(C,R,E,A),
    _MAKE_ID(D,A,T,A),
	_MAKE_ID(D,I,A,L),
    _MAKE_ID(D,O,O,R),
	_MAKE_ID(E,N,C,H),
	_MAKE_ID(F,A,C,T),
    _MAKE_ID(F,N,A,M),  // kId_FNAM
    _MAKE_ID(F,R,M,R),
    _MAKE_ID(H,E,D,R),
    _MAKE_ID(G,L,O,B),
	_MAKE_ID(G,M,S,T),
	_MAKE_ID(I,N,A,M),
	_MAKE_ID(I,N,D,X),
	_MAKE_ID(I,N,F,O),
    _MAKE_ID(I,N,G,R),
    _MAKE_ID(I,N,T,V),  // Integer Value
    _MAKE_ID(L,A,N,D),
	_MAKE_ID(L,E,V,C),
	_MAKE_ID(L,E,V,I),
    _MAKE_ID(L,H,D,T),  
    _MAKE_ID(L,I,G,H),
    _MAKE_ID(L,O,C,K),
	_MAKE_ID(L,T,E,X),
	_MAKE_ID(M,G,E,F),
    _MAKE_ID(M,I,S,C),
    _MAKE_ID(M,O,D,L), 
    _MAKE_ID(N,A,M,E),
    _MAKE_ID(N,A,M,5),
    _MAKE_ID(N,P,C,_),
	_MAKE_ID(P,G,R,D),
    _MAKE_ID(P,R,O,B),
	_MAKE_ID(R,A,C,E),
    _MAKE_ID(R,E,G,N),
    _MAKE_ID(R,E,P,A),
    _MAKE_ID(R,G,N,N),
    _MAKE_ID(S,O,U,N),
	_MAKE_ID(S,C,H,D),
	_MAKE_ID(S,C,P,T),
	_MAKE_ID(S,K,I,L),
	_MAKE_ID(S,N,D,G),
	_MAKE_ID(S,P,E,L),
    _MAKE_ID(S,T,A,T),
    _MAKE_ID(T,E,S,3),
    _MAKE_ID(V,C,L,R),
    _MAKE_ID(V,H,G,T),
    _MAKE_ID(V,N,M,L),
    _MAKE_ID(V,T,E,X),
    _MAKE_ID(W,E,A,P),
    _MAKE_ID(W,H,G,T),
    _MAKE_ID(W,N,A,M),
    _MAKE_ID(X,S,C,L),
};

