import abc     # https://pymotw.com/2/abc/

class BaseSignal(object):
    @abc.abstractmethod
    def __add__(self):
        pass
    @abc.abstractmethod
    def __sub__(self):
        pass
    @abc.abstractmethod
    def set(self):
        """Given a number, set the value of this signal"""
        pass
    @abc.abstractmethod
    def value(self):
        """Return a number for the current value of this signal"""
        pass
