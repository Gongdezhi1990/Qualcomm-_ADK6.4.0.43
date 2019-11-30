'''
 Copyright (c) 2017 Qualcomm Technologies International, Ltd.
    
'''

class Indented(object):
    ''' Maintains the current indent level. 
        Use indented_print() to print text at the correct current indent level. '''
    level = 0
    spaces_per_indent = 4
    def __enter__(self):
        Indented.level += 1
        return self
    def __exit__(self, type, value, traceback):
        assert Indented.level >= 0
        Indented.level -= 1
    @classmethod
    def indent(cls):
        return ' ' * cls.spaces_per_indent * cls.level
    @classmethod
    def indented_print(cls, string):
        print(cls.indent() + string)

class CDefinition(Indented):
    ''' Base class for creating enums, structs, unions '''
    def __init__(self, type, name, array):
        self.type = type
        self.name = name
        self.array = array
        self.list = []
        super(CDefinition, self).__init__()
    def __enter__(self):
        ''' On entry, print the start of the CDefinition '''
        self.indented_print('{} {}{}\n{{'.format(self.type, self.name, '[] =' if self.array else ''))
        return super(CDefinition, self).__enter__()
    def __exit__(self, type, value, traceback):
        ''' On exit, print the items and complete '''
        print(',\n'.join([self.indent() + s for s in self.list]))
        super(CDefinition, self).__exit__(type, value, traceback)
        self.indented_print('};\n')
    def extend(self, items):
        self.list.extend(items)

class Enumeration(CDefinition):
    ''' Generate a c enumeration '''
    def __init__(self, name):
        CDefinition.__init__(self, 'enum', name, False)

class Array(CDefinition):
    ''' Generate a c array '''
    def __init__(self, array_type, name):
        CDefinition.__init__(self, array_type, name, True)

class CommentBlock(Indented):
    ''' Generate a c comment '''
    def __enter__(self):
        self.indented_print('/*')
        return super(CommentBlock, self).__enter__()
    def __exit__(self, type, value, traceback):
        super(CommentBlock, self).__exit__(type, value, traceback)
        self.indented_print('*/\n')

class CommentBlockDoxygen(Indented):
    ''' Use this class to generate doxygen c comments '''
    def __enter__(self):
        self.indented_print('/*!')
        return super(CommentBlockDoxygen, self).__enter__()
    def __exit__(self, type, value, traceback):
        super(CommentBlockDoxygen, self).__exit__(type, value, traceback)
        self.indented_print('*/\n')
    def doxy_copyright(self):
        from datetime import datetime
        year = datetime.now().year
        self.indented_print("\\copyright Copyright (c) {} Qualcomm Technologies International, Ltd.".format(year))
        with Indented():
            self.indented_print("All Rights Reserved.")
            self.indented_print("Qualcomm Technologies International, Ltd. Confidential and Proprietary.")
    def doxy_version(self):
        self.indented_print("\\version ")
    def doxy_filename(self, filename):
        self.indented_print("\\file {}".format(filename))
    def doxy_brief(self, brief):
        self.indented_print("\\brief {}".format(brief))

class HeaderGuards(object):
    ''' Use this class to print header file guards '''
    def __init__(self, filename_base):
        self.guard_name = '_{}_H__'.format(filename_base.upper())
    def __enter__(self):
        print('#ifndef {}'.format(self.guard_name))
        print('#define {}\n'.format(self.guard_name))
    def __exit__(self, type, value, traceback):
        print('#endif /* {} */\n'.format(self.guard_name))
