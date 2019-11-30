# pylint: skip-file
# @PydevCodeAnalysisIgnore

import argparse

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Download capability startup kalsim shell script')
    parser.add_argument('dkcs_file', type=str, help='Input bundle file (dkcs file)')
    args = parser.parse_args()

    print('downloading file %s' % (args.dkcs_file))

    id = hydra_cap_download.download(args.dkcs_file)
    print('Bundle downloaded. Id is %s' % (id))
