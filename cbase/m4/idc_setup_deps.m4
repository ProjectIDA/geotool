#serial 0.0.1

#
# IDC_SETUP_DEPS(LIBRARY)
# -----------------------
# This macro abstracts some repeatable used functionality for handling
# library dependencies.
#
# a) Remove library location (-L) and library name (-l) duplication to reduce
#    the size of linker command lines by discarding all but first of
#    identical library location fields and all but last of identical library 
#    name fields (field being run of non-whitespace characters) from list of
#    dependencies (except for cross-dependent library names where duplication
#    is needed) by splitting value on whitespace into fields, then dropping
#    duplicate fields (saving previously mentioned fields), and then
#    concatenating all remaining library location fields followed by all
#    remaining library name fields with one space between each field into a
#    new value. Note that unrecognized fields are ignored and removed.
#
# b) libtool's inter-library dependency support does not work when libraries
#    have cyclic dependencies. It must then be disabled and the classic way
#    of listing every library dependency (for each and every library listed)
#    each time a library is used must be used.
#
#    libtool library dependencies are set in library Makefile.am files via
#    libfoo_la_LIBADD (for the foo library).
#
#    Cyclic dependencies are common-place in ibase.
#
# Setup:    Set shell variable enable_libtool_deps to 0 to disable libtool
#           dependency support, anything else or if unset enables it
#
# Example:  configure.ac:  enable_libtool_deps=0
#                          FOO_LIBS="-L\$(top_srcdir)/libsrc/libfoo -lfoo"
#                          FOO_DEPS="${BAR_LIBS} ${FIE_LIBS}"
#                          IDC_SETUP_DEPS([FOO])
#
# Remark:   Deleting an entire array is a gawk extension. For standard nawk,
#           for an array a, one needs to do "for (x in a) {delete a[x]};"
#           instead of "delete a;"
#
# HE (IDC/SA/SI, CTBTO PrepCom) - 21 November 2008
#
AC_DEFUN([IDC_SETUP_DEPS],
[
# Remove duplicates as described in (a)
$1_DEPS=`${AWK} -v xdeps="${XDEPS}" -- 'BEGIN {n=split(xdeps, a); for (i=1; i<=n; i++) b@<:@a@<:@i@:>@@:>@; for (x in a) {delete a@<:@xj@:>@}} {for (i=1; i<=NF; i++) if (($i ~ /^-L/) && !($i in a)) {a@<:@$i@:>@; printf("%s ", $i)}; for (x in a) {delete a@<:@xj@:>@}; j=0; for (i=NF; i>0; i--) if (($i ~ /^-l/) && (!($i in a) || ($i in b))) {a@<:@$i@:>@; c@<:@j++@:>@=$i}; for (k=j-1; k>=0; k--) {printf("%s ", c@<:@k@:>@)}; printf("\n"); for (x in a) {delete a@<:@xj@:>@}; for (x in c) {delete c@<:@xj@:>@}}' << EOF
${$1_DEPS}
EOF
`

if test "${enable_libtool_deps}" = "0"; then
  # This is an ugly hack when one cannot use the correct way

  # $1_LIBS contains location (-L) and name (-l) of foo library
  # $1_LIBS contains locations (-L) and names (-l) of foo's dependencies
  $1_LIBS="${$1_LIBS} ${$1_DEPS}"

  # $1_LIBADD is empty so that is still can be used in Makefile.am files
  $1_LIBADD=""
else 
  # This is the beautiful and correct way of handling dependencies

  $1_LIBADD="${$1_LIBADD} ${$1_DEPS}"
  # $1_LIBS (untouched) contains location (-L) and name (-l) of foo library
  # $1_LIBADD contains locations (-L) and names (-l) of foo's dependencies
fi

# $1_DEPS is emptied so that more dependencies can be added later
$1_DEPS=""
])
