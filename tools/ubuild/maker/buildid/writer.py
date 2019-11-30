ID_STRING_FILE_HDR = (
    '#include <hydra_types.h>\n'
    '\n/** This file will be re-generated on each build */\n'
)

BUILD_ID_STRING_VARIABLE = 'const uint8 build_id_string[]'

BUILD_ID_NUMBER_VARIABLE = 'const uint32 build_id_number'


class BuildIdWriter(object):

    @staticmethod
    def write(filename, build_number, build_string):
        """Generates and writes the source file with the build Id information

        Arguments:
            filename {str} -- Path to the build_id_str.c file location
            build_number {int} -- Build Id number. Must fit a uint32 variable
            build_string {string} -- Build Id string.
        """

        try:
            build_id_string_line = '{:s} = "{:s}";'.format(BUILD_ID_STRING_VARIABLE, build_string)
        except ValueError:
            raise ValueError("build_string must be a string, got: {!s}".format(type(build_string)))

        try:
            build_id_number_line = '{:s} = {:d}UL;'.format(BUILD_ID_NUMBER_VARIABLE, build_number)
        except ValueError:
            raise ValueError("build_number must be an integer, got: {!s}".format(type(build_number)))

        with open(filename, 'w+') as f:
            output = '\n'.join((ID_STRING_FILE_HDR,
                                build_id_string_line,
                                build_id_number_line))
            f.write(output)
        print("BUILD_ID_STRING = {:s}".format(build_string))
        print("BUILD_ID_NUMBER = {:d}".format(build_number))
