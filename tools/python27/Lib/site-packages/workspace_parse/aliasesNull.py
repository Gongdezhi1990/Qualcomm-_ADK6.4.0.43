class Null(object):
    """ Null objects always and reliably "do nothing."
    """

    def __init__(self, *args, **kwargs):
        print("[NULL] Using Null object for class: {}".format(self.__class__))
        pass

    def __call__(self, *args, **kwargs):
        return self

    def __repr__(self):
        return 'Null(  )'

    def __nonzero__(self):
        return 0

    def __getattr__(self, name):
        return self

    def __setattr__(self, name, value):
        return self

    def __delattr__(self, name):
        return self


class NullAttr(Null):
    def __init__(self, name):
        self.__name = name

    def __call__(self, *args, **kwargs):
        print("[NULL] Null method called: {}".format(self.__name))
        if len(args) == 1:
            return args[0]
        return args


class Aliases(Null):

    def __getattr__(self, name):
        return NullAttr(name)

