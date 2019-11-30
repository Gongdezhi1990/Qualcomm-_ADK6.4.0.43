from __future__ import print_function
import xml.etree.ElementTree as ET
import argparse
import os.path
import binascii
import sys


class smbXmlConfigFile(object):

    def __init__(self, xmlConfigFilePath):
        tree = ET.parse(xmlConfigFilePath)
        self.root = tree.getroot()

    def getConfigRegisters(self):
        configRegs = self.root.find("config/R00_To_R14")
        return configRegs.text

    def getStatusRegisters(self):
        statusRegs = self.root.find("status/R36_To_R47")
        return statusRegs.text

    def getSummitRegisters(self):
        summitRegs = self.root.find("Summit_Values/T15_To_T2E")
        return summitRegs.text


def addPadding(amountOfPadding):
    return "00 " * amountOfPadding


def xml_to_hex(inputFile, outputDirectory):
        outputFile = "chargerConfig.hex"
        if not os.path.exists(outputDirectory):
            os.makedirs(outputDirectory)
        outputFile = os.path.join(outputDirectory, outputFile)
        smbConfig = smbXmlConfigFile(inputFile)
        hexString = smbConfig.getConfigRegisters() + " " + smbConfig.getStatusRegisters() + " " + smbConfig.getSummitRegisters()
        checkAlignedToBoundary = len(hexString.split()) % 16
        if (checkAlignedToBoundary != 0):
            hexString = hexString + addPadding(16 - checkAlignedToBoundary)
        with open(outputFile, 'wb') as fout:
            fout.write(binascii.unhexlify(''.join(hexString.split())))
        return outputFile


def parse_args(args):
    parser = argparse.ArgumentParser(description="Converts the XML configuration file produced by the SMB13XX GUI to a Hex file")
    requiredArgs = parser.add_argument_group('Required Arguments')
    requiredArgs.add_argument('-smbXmlConfig', help='Path to the XML configuration file', type=str, required=True)
    requiredArgs.add_argument('-smbHexConfigOutputDir', help='Output directory for the SMB hex configuration file', type=str, required=True)
    return parser.parse_args(args)


def main():
    if __name__ == "__main__":
        args = parse_args(sys.argv[1:])
        try:
            xml_to_hex(args.smbXmlConfig, args.smbHexConfigOutputDir)
            sys.exit(0)
        except IOError as e:
            print(e)
            sys.exit(e.errno)


main()
