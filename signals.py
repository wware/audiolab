import abc     # https://pymotw.com/2/abc/


class BaseSignal(object):
    __metaclass__ = abc.ABCMeta

    @abc.abstractmethod
    def __add__(self):
        pass
    @abc.abstractmethod
    def __sub__(self):
        pass
    @abc.abstractproperty
    def value(self):
        """Return a number for the current value of this signal"""
        return 'NOPE'


class ReferenceSignal(BaseSignal):
    def __init__(self, v=None):
        if v is None:
            self._v = 0.
        else:
            self._v = float(v)

    def __add__(self, other):
        return self.__class__(self._v + other._v)

    def __sub__(self, other):
        return self.__class__(self._v - other._v)

    def value_getter(self):
        return self._v

    def value_setter(self, v):
        self._v = v

    value = property(value_getter, value_setter)

