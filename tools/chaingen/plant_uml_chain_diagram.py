'''
 Copyright (c) 2017 Qualcomm Technologies International, Ltd.
    
'''

from c_code_gen import Indented

class PlantUmlChainDiagram(Indented):
    ''' This class abstracts the details of drawing plant UML diagrams for a chain '''
    def __enter__(self):
        self.indented_print("@startuml")
        return super(PlantUmlChainDiagram, self).__enter__()

    def __exit__(self, type, value, traceback):
        super(PlantUmlChainDiagram, self).__exit__(type, value, traceback)
        self.indented_print("@enduml")

    def object(self, name, id):
        ''' Print an object '''
        self.indented_print("object {}".format(name))
        self.indented_print("{} : id = {}".format(name, id))

    def wire(self, source_metadata, sink_metadata):
        ''' Print a wire based on the metadata '''
        str_a = '{operator} "{terminal}({terminal_num})"'.format(**sink_metadata)
        str_b = '<-- "{terminal}({terminal_num})" {operator}'.format(**source_metadata)
        self.indented_print(str_a + str_b)

    def input(self, sink_metadata):
        ''' Print a input based on the metadata '''
        self.indented_print("object {role} #lightgreen".format(**sink_metadata))
        self.indented_print('{operator} "{terminal}({terminal_num})" <-- {role}'.format(**sink_metadata))

    def output(self, source_metadata):
        ''' Print a output based on the metadata '''
        self.indented_print("object {role} #lightblue".format(**source_metadata))
        self.indented_print('{role} <-- "{terminal}({terminal_num})" {operator}'.format(**source_metadata))
