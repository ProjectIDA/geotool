#serial 0.0.1

#
# IDC_UNIQ_VAR(VARIABLE)
# ----------------------
# Discard all but first of identical fields (run of non-whitespace characters)
# from value of variable VARIABLE by splitting value on whitespace into
# fields, then dropping duplicate fields (keeping the first field), and then
# concatenating all remaining fields with one space into a new value.
#
# This macro is useful when reducing the size of an argument list where
# precedence (added order) is important.
#
# Example:   configure.ac:  FOO="foo bar fie fie fum baz fum fie bar foo"
#                           IDC_UNIQ_FIRST_VAR([FOO])
#                           ${ECHO} "<${FOO}>"
#           
#            Output:        <foo bar fie fum baz>
#
# Note/Bug:  New value will end with an extraneous space
#
# Remark:    Deleting an entire array is a gawk extension. For standard nawk,
#            for an array a, one needs to do "for (x in a) {delete a[x]};"
#            instead of "delete a;"
#
# HE (IDC/SA/SI, CTBTO PrepCom) - 24 November 2008
#
AC_DEFUN([IDC_UNIQ_VAR],
[AC_REQUIRE([AC_PROG_AWK])
$1=`${AWK} '{i=0; for (j=NF; j>0; j--) if (!($j in a)) {a@<:@$j@:>@; printf("%s ", $j)}; printf("\n"); for (x in a) {delete a@<:@xj@:>@}}' << EOF
${$1}
EOF
`
])
