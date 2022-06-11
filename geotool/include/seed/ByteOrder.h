/** \file ByteOrder.h
 *  \brief Declares typedefs and methods for handling word byte order.
 *  \author Ivan Henson
 */
#ifndef _BYTE_ORDER_
#define _BYTE_ORDER

typedef union { char a[2]; unsigned short s; } UWORD;
typedef union { char a[2]; short s; } WORD;
typedef union { char a[4]; unsigned int i; } ULONG;
typedef union { char a[4]; int i; } LONG;
typedef union { char a[4]; float f; } FLOAT;

class ByteOrder
{
    public:
    static void getNativeWordOrder(LONG &u);
};

#endif
