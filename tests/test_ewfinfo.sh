#!/bin/bash
#
# ewfinfo testing script
#
# Copyright (c) 2006-2010, Joachim Metz <jbmetz@users.sourceforge.net>
#
# Refer to AUTHORS for acknowledgements.
#
# This software is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this software.  If not, see <http://www.gnu.org/licenses/>.
#

EXIT_SUCCESS=0;
EXIT_FAILURE=1;
EXIT_IGNORE=77;

INPUT="input";
TMP="tmp";

LS="ls";
TR="tr";
WC="wc";

EWFINFO="../ewftools/ewfinfo";

function test_info
{ 
	echo "Testing ewfinfo of input:" $*;

	./${EWFINFO} $*;

	RESULT=$?;

	echo "";

	return ${RESULT};
}

if ! test -x ${EWFINFO};
then
	echo "Missing executable: ${EWFINFO}";

	exit ${EXIT_FAILURE};
fi

if ! test -d ${INPUT};
then
	echo "No input directory found, to test read create input directory and place test files in directory.";

	exit ${EXIT_IGNORE};
fi

RESULT=`${LS} ${INPUT}/*.[eE]01 | ${TR} ' ' '\n' | ${WC} -l`;

if test ${RESULT} -eq 0;
then
	echo "No files found in input directory, to test read place test files in directory.";

	exit ${EXIT_IGNORE};
fi

for FILENAME in `${LS} ${INPUT}/*.[eE]01 | ${TR} ' ' '\n'`;
do
	if ! test_info "${FILENAME}";
	then
		exit ${EXIT_FAILURE};
	fi
done

exit ${EXIT_SUCCESS};

