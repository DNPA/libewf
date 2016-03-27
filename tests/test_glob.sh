#!/bin/bash
#
# Expert Witness Compression Format (EWF) library glob testing script
#
# Copyright (c) 2010-2011, Joachim Metz <jbmetz@users.sourceforge.net>
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

TMP="tmp";

CMP="cmp";
EGREP="egrep";

chr()
{
	CHR_VALUE=`expr \( \( $1 / 64 \) \* 100 \) + \( \( \( $1 % 64 \) / 8 \) \* 10 \) + \( $1 % 8 \)`;

	printf \\${CHR_VALUE};
} 

seq()
{
	VALUE=$1;
	SEQUENCE="";

	while [ ${VALUE} -le $2 ];
	do

		if [ ${VALUE} -le 9 ];
		then
			SEQUENCE="${SEQUENCE} 0${VALUE}";
		else
			SEQUENCE="${SEQUENCE} ${VALUE}";
		fi

		VALUE=`expr ${VALUE} + 1`;
	done

	echo ${SEQUENCE};
}

test_glob()
{ 
	BASENAME=$1;
	SCHEMA=$2;
	FILENAMES=$3;

	mkdir ${TMP};
	cd ${TMP};

	echo ${FILENAMES} > input;

	touch ${FILENAMES};

	../${EWF_TEST_GLOB} ${BASENAME} > output;

	RESULT=$?;

	if test ${RESULT} -eq ${EXIT_SUCCESS};
	then
		if ! ${CMP} -s input output;
		then
			RESULT=${EXIT_FAILURE};
		fi
	fi

	cd ..;
	rm -rf ${TMP};

	echo -n "Testing glob: for basename: ${BASENAME} and schema: ${SCHEMA} ";

	if test ${RESULT} -ne ${EXIT_SUCCESS};
	then
		echo " (FAIL)";
	else
		echo " (PASS)";
	fi
	return ${RESULT};
}

test_glob_sequence()
{ 
	BASENAME=$1;
	SCHEMA=$2;
	FILENAME=$3;
	LAST=$4;

	RESULT=`echo ${LAST} | ${EGREP} "^[esEL][0-9a-zA-Z][0-9a-zA-Z]$"`;
	LAST_IS_VALID=$?;

	if [ ${LAST_IS_VALID} -ne 0 ];
	then
		echo "Unsupported last: ${LAST}";

		exit ${EXIT_FAILURE};
	fi

	FIRST_LETTER=`echo ${LAST} | cut -c 1`;

	if [ ${LAST} = "${FIRST_LETTER}00" ];
	then
		echo "Unsupported last: ${LAST}";

		exit ${EXIT_FAILURE};
	fi

	RESULT=`echo ${LAST} | ${EGREP} "^[esEL][0-9][0-9]$"`;
	LAST_IS_NUMERIC=$?;

	if [ ${LAST_IS_NUMERIC} -eq 0 ];
	then
		LAST=`echo ${LAST} | cut -c '2 3'`;

		SEQUENCE=`seq 1 ${LAST}`;
	else
		SEQUENCE=`seq 1 99`;
	fi

	FILENAMES=`for NUMBER in ${SEQUENCE}; do echo -n "${FILENAME}.${FIRST_LETTER}${NUMBER} "; echo $FILE; done`;

	if [ ${LAST_IS_NUMERIC} -ne 0 ];
	then
		RESULT=`echo ${LAST} | ${EGREP} "^[esEL][A-Z][A-Z]$"`;
		LAST_IS_UPPER_CASE=$?;

		SECOND_ITERATOR=0;
		THIRD_ITERATOR=0;

		if [ ${LAST_IS_UPPER_CASE} -eq 0 ];
		then
			SECOND_BYTE_VALUE=`expr 65 + ${SECOND_ITERATOR}`;
			THIRD_BYTE_VALUE=`expr 65 + ${THIRD_ITERATOR}`;
		else
			SECOND_BYTE_VALUE=`expr 97 + ${SECOND_ITERATOR}`;
			THIRD_BYTE_VALUE=`expr 97 + ${THIRD_ITERATOR}`;
		fi

		SECOND_LETTER=`chr ${SECOND_BYTE_VALUE}`;
		THIRD_LETTER=`chr ${THIRD_BYTE_VALUE}`;

		EXTENSION="${FIRST_LETTER}${SECOND_LETTER}${THIRD_LETTER}";

		until [ ${EXTENSION} = ${LAST} ];
		do
			FILENAMES="${FILENAMES} ${FILENAME}.${EXTENSION}";

			THIRD_ITERATOR=`expr ${THIRD_ITERATOR} + 1`;

			if [ ${THIRD_ITERATOR} -ge 26 ];
			then
				SECOND_ITERATOR=`expr ${SECOND_ITERATOR} + 1`;

				THIRD_ITERATOR=0;
			fi

			if [ ${LAST_IS_UPPER_CASE} -eq 0 ];
			then
				SECOND_BYTE_VALUE=`expr 65 + ${SECOND_ITERATOR}`;
				THIRD_BYTE_VALUE=`expr 65 + ${THIRD_ITERATOR}`;
			else
				SECOND_BYTE_VALUE=`expr 97 + ${SECOND_ITERATOR}`;
				THIRD_BYTE_VALUE=`expr 97 + ${THIRD_ITERATOR}`;
			fi

			SECOND_LETTER=`chr ${SECOND_BYTE_VALUE}`;
			THIRD_LETTER=`chr ${THIRD_BYTE_VALUE}`;

			EXTENSION="${FIRST_LETTER}${SECOND_LETTER}${THIRD_LETTER}";
		done

		FILENAMES="${FILENAMES} ${FILENAME}.${EXTENSION}";
	fi

	mkdir ${TMP};
	cd ${TMP};

	echo ${FILENAMES} > input;

	touch ${FILENAMES};

	../${EWF_TEST_GLOB} ${BASENAME} > output;

	RESULT=$?;

	if test ${RESULT} -eq ${EXIT_SUCCESS};
	then
		if ! ${CMP} -s input output;
		then
			RESULT=${EXIT_FAILURE};
		fi
	fi

	cd ..;
	rm -rf ${TMP};

	echo -n "Testing glob: for basename: ${BASENAME} and schema: ${SCHEMA} ";

	if test ${RESULT} -ne ${EXIT_SUCCESS};
	then
		echo " (FAIL)";
	else
		echo " (PASS)";
	fi
	return ${RESULT};
}

EWF_TEST_GLOB="ewf_test_glob";

if ! test -x ${EWF_TEST_GLOB};
then
	EWF_TEST_GLOB="ewf_test_glob.exe";
fi

if ! test -x ${EWF_TEST_GLOB};
then
	echo "Missing executable: ${EWF_TEST_GLOB}";

	exit ${EXIT_FAILURE};
fi

rm -rf ${TMP};

if ! test_glob "PREFIX.e01" ".e01" "PREFIX.e01";
then
	exit ${EXIT_FAILURE};
fi

if ! test_glob "PREFIX.e01" ".e01" "PREFIX.e01 PREFIX.e02 PREFIX.e03 PREFIX.e04 PREFIX.e05 PREFIX.e06 PREFIX.e07 PREFIX.e08 PREFIX.e09";
then
	exit ${EXIT_FAILURE};
fi

if ! test_glob "PREFIX.e01" ".e01" "PREFIX.e01 PREFIX.e02 PREFIX.e03 PREFIX.e04 PREFIX.e05 PREFIX.e06 PREFIX.e07 PREFIX.e08 PREFIX.e09 PREFIX.e10 PREFIX.e11";
then
	exit ${EXIT_FAILURE};
fi

if ! test_glob_sequence "PREFIX.e01" ".e01" "PREFIX" "eba";
then
	exit ${EXIT_FAILURE};
fi

if ! test_glob "PREFIX.s01" ".s01" "PREFIX.s01";
then
	exit ${EXIT_FAILURE};
fi

if ! test_glob "PREFIX.s01" ".s01" "PREFIX.s01 PREFIX.s02 PREFIX.s03 PREFIX.s04 PREFIX.s05 PREFIX.s06 PREFIX.s07 PREFIX.s08 PREFIX.s09";
then
	exit ${EXIT_FAILURE};
fi

if ! test_glob "PREFIX.s01" ".s01" "PREFIX.s01 PREFIX.s02 PREFIX.s03 PREFIX.s04 PREFIX.s05 PREFIX.s06 PREFIX.s07 PREFIX.s08 PREFIX.s09 PREFIX.s10 PREFIX.s11";
then
	exit ${EXIT_FAILURE};
fi

if ! test_glob_sequence "PREFIX.s01" ".s01" "PREFIX" "sba";
then
	exit ${EXIT_FAILURE};
fi

if ! test_glob "PREFIX.E01" ".E01" "PREFIX.E01";
then
	exit ${EXIT_FAILURE};
fi

if ! test_glob "PREFIX.E01" ".E01" "PREFIX.E01 PREFIX.E02 PREFIX.E03 PREFIX.E04 PREFIX.E05 PREFIX.E06 PREFIX.E07 PREFIX.E08 PREFIX.E09";
then
	exit ${EXIT_FAILURE};
fi

if ! test_glob "PREFIX.E01" ".E01" "PREFIX.E01 PREFIX.E02 PREFIX.E03 PREFIX.E04 PREFIX.E05 PREFIX.E06 PREFIX.E07 PREFIX.E08 PREFIX.E09 PREFIX.E10 PREFIX.E11";
then
	exit ${EXIT_FAILURE};
fi

if ! test_glob_sequence "PREFIX.E01" ".E01" "PREFIX" "EBA";
then
	exit ${EXIT_FAILURE};
fi

if ! test_glob "PREFIX.L01" ".L01" "PREFIX.L01";
then
	exit ${EXIT_FAILURE};
fi

if ! test_glob "PREFIX.L01" ".L01" "PREFIX.L01 PREFIX.L02 PREFIX.L03 PREFIX.L04 PREFIX.L05 PREFIX.L06 PREFIX.L07 PREFIX.L08 PREFIX.L09";
then
	exit ${EXIT_FAILURE};
fi

if ! test_glob "PREFIX.L01" ".L01" "PREFIX.L01 PREFIX.L02 PREFIX.L03 PREFIX.L04 PREFIX.L05 PREFIX.L06 PREFIX.L07 PREFIX.L08 PREFIX.L09 PREFIX.L10 PREFIX.L11";
then
	exit ${EXIT_FAILURE};
fi

if ! test_glob_sequence "PREFIX.L01" ".L01" "PREFIX" "LBA";
then
	exit ${EXIT_FAILURE};
fi

exit ${EXIT_SUCCESS};

