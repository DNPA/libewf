#!/bin/bash
# Export tool testing script
#
# Version: 20160320

EXIT_SUCCESS=0;
EXIT_FAILURE=1;
EXIT_IGNORE=77;

TEST_PREFIX=`pwd`;
TEST_PREFIX=`dirname ${TEST_PREFIX}`;
TEST_PREFIX=`basename ${TEST_PREFIX} | sed 's/^lib//'`;

OPTION_SETS="format:encase1 format:encase2 format:encase3 format:encase4 format:encase5 format:encase6 format:encase7 format:encase7-v2 format:ewf format:ewfx format:ftk format:linen5 format:linen6 format:linen7 format:raw format:smart deflate:none deflate:empty-block deflate:fast deflate:best blocksize:16 blocksize:32 blocksize:128 blocksize:256 blocksize:512 blocksize:1024 blocksize:2048 blocksize:4096 blocksize:8192 blocksize:16384 blocksize:32768 hash:sha1 hash:sha256 hash:all";
INPUT_GLOB="*.[Ees]*01";

list_contains()
{
	LIST=$1;
	SEARCH=$2;

	for LINE in ${LIST};
	do
		if test ${LINE} = ${SEARCH};
		then
			return ${EXIT_SUCCESS};
		fi
	done

	return ${EXIT_FAILURE};
}

run_test()
{
	TEST_SET_DIR=$1;
	TEST_DESCRIPTION=$2;
	TEST_EXECUTABLE=$3;
	INPUT_FILE=$4;
	OPTION_SET=$5;

	TEST_RUNNER="tests/test_runner.sh";

	if ! test -x "${TEST_RUNNER}";
	then
		TEST_RUNNER="./test_runner.sh";
	fi

	if ! test -x "${TEST_RUNNER}";
	then
		echo "Missing test runner: ${TEST_RUNNER}";

		return ${EXIT_FAILURE};
	fi

	INPUT_NAME=`basename ${INPUT_FILE}`;

	if test -z "${OPTION_SET}";
	then
		OPTIONS=();
		TEST_OUTPUT="${INPUT_NAME}";
	else
		OPTIONS_STRING=`cat "${TEST_SET_DIR}/${INPUT_NAME}.${OPTION_SET}" | head -n 1 | sed 's/[\r\n]*$//'`;
		IFS=" " read -a OPTIONS <<< ${OPTIONS_STRING};
		TEST_OUTPUT="${INPUT_NAME}-${OPTION_SET}";
	fi
	TMPDIR="tmp$$";

	rm -rf ${TMPDIR};
	mkdir ${TMPDIR};

	STORED_TEST_RESULTS="${TEST_SET_DIR}/${TEST_OUTPUT}.log.gz";
	TEST_RESULTS="${TMPDIR}/${TEST_OUTPUT}.log";

	${TEST_RUNNER} ${TMPDIR} ${TEST_EXECUTABLE} -q -t ${TMPDIR}/${INPUT_NAME}.export -u ${OPTIONS[*]} ${INPUT_FILE} &> /dev/null;

	RESULT=$?;

	if test -f ${TMPDIR}/${INPUT_NAME}.export.raw;
	then
		(cd ${TMPDIR} && md5sum ${INPUT_NAME}.export.* | sort -k 2 > ${TEST_OUTPUT}.log);

		if test -f "${STORED_TEST_RESULTS}";
		then
			zdiff ${STORED_TEST_RESULTS} ${TEST_RESULTS};

			RESULT=$?;
		else
			gzip ${TEST_RESULTS};

			mv "${TEST_RESULTS}.gz" ${TEST_SET_DIR};
		fi
	else
		if test ${RESULT} -eq ${EXIT_SUCCESS};
        	then
	                ${VERIFY_TOOL} -q ${TMPDIR}/${INPUT_NAME}.export.* > /dev/null;

        	        RESULT=$?;
	        fi
	fi
	rm -rf ${TMPDIR};

	if test -z "${OPTION_SET}";
	then
		echo -n "Testing ${TEST_DESCRIPTION} with input: ${INPUT_FILE}";
	else
		echo -n "Testing ${TEST_DESCRIPTION} with option: ${OPTION_SET} and input: ${INPUT_FILE}";
	fi

	if test ${RESULT} -ne ${EXIT_SUCCESS};
	then
		echo " (FAIL)";
	else
		echo " (PASS)";
	fi
	return ${RESULT};
}

run_tests()
{
	TEST_PROFILE=$1;
	TEST_DESCRIPTION=$2;
	TEST_EXECUTABLE=$3;

	if ! test -d "input";
	then
		echo "No input directory found.";

		return ${EXIT_IGNORE};
	fi
	RESULT=`ls input/* | tr ' ' '\n' | wc -l`;

	if test ${RESULT} -eq 0;
	then
		echo "No files or directories found in the input directory.";

		return ${EXIT_IGNORE};
	fi
	TEST_PROFILE_DIR="input/.${TEST_PROFILE}";

	if ! test -d "${TEST_PROFILE_DIR}";
	then
		mkdir ${TEST_PROFILE_DIR};
	fi
	IGNORE_FILE="${TEST_PROFILE_DIR}/ignore";
	IGNORE_LIST="";

	if test -f "${IGNORE_FILE}";
	then
		IGNORE_LIST=`cat ${IGNORE_FILE} | sed '/^#/d'`;
	fi

	for INPUT_DIR in input/*;
	do
		if ! test -d "${INPUT_DIR}";
		then
			continue
		fi
		INPUT_NAME=`basename ${INPUT_DIR}`;

		if list_contains "${IGNORE_LIST}" "${INPUT_NAME}";
		then
			continue
		fi
		TEST_SET_DIR="${TEST_PROFILE_DIR}/${INPUT_NAME}";

		if ! test -d "${TEST_SET_DIR}";
		then
			mkdir "${TEST_SET_DIR}";
		fi

		if test -f "${TEST_SET_DIR}/files";
		then
			INPUT_FILES=`cat ${TEST_SET_DIR}/files | sed "s?^?${INPUT_DIR}/?"`;
		else
			INPUT_FILES=`ls ${INPUT_DIR}/${INPUT_GLOB}`;
		fi

		for INPUT_FILE in ${INPUT_FILES};
		do
			TESTED_WITH_OPTIONS=0;
			INPUT_NAME=`basename ${INPUT_FILE}`;

			for OPTION_SET in `echo ${OPTION_SETS} | tr ' ' '\n'`;
			do
				OPTION_FILE="${TEST_SET_DIR}/${INPUT_NAME}.${OPTION_SET}";

				if ! test -f "${OPTION_FILE}";
				then
					continue
				fi

				if ! run_test "${TEST_SET_DIR}" "${TEST_DESCRIPTION}" "${TEST_EXECUTABLE}" "${INPUT_FILE}" "${OPTION_SET}";
				then
					return ${EXIT_FAILURE};
				fi
				TESTED_WITH_OPTIONS=1;
			done

			if test ${TESTED_WITH_OPTIONS} -eq 0;
			then
				if ! run_test "${TEST_SET_DIR}" "${TEST_DESCRIPTION}" "${TEST_EXECUTABLE}" "${INPUT_FILE}" "";
				then
					return ${EXIT_FAILURE};
				fi
			fi
		done
	done

	return ${EXIT_SUCCESS};
}

if ! test -z ${SKIP_TOOLS_TESTS};
then
	exit ${EXIT_IGNORE};
fi

EXPORT_TOOL="../${TEST_PREFIX}tools/${TEST_PREFIX}export";

if ! test -x "${EXPORT_TOOL}";
then
	EXPORT_TOOL="../${TEST_PREFIX}tools/${TEST_PREFIX}export.exe";
fi

if ! test -x "${EXPORT_TOOL}";
then
	echo "Missing executable: ${EXPORT_TOOL}";

	exit ${EXIT_FAILURE};
fi

VERIFY_TOOL="../${TEST_PREFIX}tools/${TEST_PREFIX}verify";

if ! test -x "${VERIFY_TOOL}";
then
	VERIFY_TOOL="../${TEST_PREFIX}tools/${TEST_PREFIX}verify.exe";
fi

if ! test -x "${VERIFY_TOOL}";
then
	echo "Missing executable: ${VERIFY_TOOL}";

	exit ${EXIT_FAILURE};
fi

OLDIFS=${IFS};
IFS="
";

run_tests "${TEST_PREFIX}export" "${TEST_PREFIX}export" "${EXPORT_TOOL}";

RESULT=$?;

IFS=${OLDIFS};

exit ${RESULT};

