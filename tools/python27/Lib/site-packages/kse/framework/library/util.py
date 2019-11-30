'''
Utility extensions
'''

import logging
import os
import sys
import threading
import time

from six.moves.queue import Queue, Empty


class RLockTimeout(object):
    '''
    Reentrant lock implementation with timeout as python 2 and 3.1 does not support this feature.

    A reentrant lock is a synchronization primitive that may be acquired multiple times by the same
    thread. Internally, it uses the concepts of "owning thread" and "recursion level" in addition
    to the locked/unlocked state used by primitive locks. In the locked state, some thread owns the
    lock; in the unlocked state, no thread owns it.

    To lock the lock, a thread calls its acquire() method; this returns once the thread owns the
    lock.
    To unlock the lock, a thread calls its release() method. acquire()/release() call pairs may be
    nested; only the final release() (the release() of the outermost pair) resets the lock to
    unlocked and allows another thread blocked in acquire() to proceed.

    Reentrant locks also support the context management protocol.

    Args:
        timeout (float): Default timeout value for acquire so that with lock can use non standard
            timeout value
    '''

    def __init__(self, timeout=None):
        self._queue = Queue()
        self._timeout = timeout
        self._owner = None
        self._waiting = 0
        self._last_owner = None
        self._rcount = 0
        self._queue.put(1)

    def acquire(self, blocking=True, timeout=None):
        '''
        Acquire a lock, blocking or non-blocking.

        When invoked without arguments:
        if this thread already owns the lock, increment the recursion level by one,
        and return immediately.
        Otherwise, if another thread owns the lock, block until the lock is unlocked.
        Once the lock is unlocked (not owned by any thread), then grab ownership,
        set the recursion level to one, and return.
        If more than one thread is blocked waiting until the lock is unlocked, only one at a time
        will be able to grab ownership of the lock. There is no return value in this case.

        When invoked with the blocking argument set to true, do the same thing as when called
        without arguments, and return true.

        When invoked with the blocking argument set to false, do not block.
        If a call without an argument would block, return false immediately; otherwise,
        do the same thing as when called without arguments, and return true.

        When invoked with the floating-point timeout argument set to a positive value, block for
        at most the number of seconds specified by timeout and as long as the lock cannot be
        acquired. Return true if the lock has been acquired, false if the timeout has elapsed.
        '''

        if self._owner == threading.current_thread().ident:
            # we already own the lock
            self._rcount += 1
        else:
            # protection if we are trying to get the lock again and someone is waiting
            if self._last_owner == threading.current_thread().ident and self._waiting != 0:
                logging.getLogger(__name__).debug(
                    'greedy %s forced to sleep', threading.current_thread().name)
                time.sleep(0.05)

            if not blocking:
                try:
                    self._queue.get(False, 0)
                    self._owner = threading.current_thread().ident
                    self._rcount = 1
                except Empty:
                    return False
            if timeout is None:
                timeout = self._timeout
            if timeout is None:
                timeout = 1
            try:
                self._waiting += 1
                self._queue.get(True, timeout)
                self._owner = threading.current_thread().ident
                self._waiting -= 1
                self._rcount = 1
            except Empty:
                raise RuntimeError('lock acquire timeout thread %s' %
                                   (threading.current_thread().name))
                # FIXME documentation says to return False
                # return False

        return True

    __enter__ = acquire

    def release(self):
        '''
        Release a lock, decrementing the recursion level.

        If after the decrement it is zero, reset the lock to unlocked (not owned by any thread),
        and if any other threads are blocked waiting for the lock to become unlocked,
        allow exactly one of them to proceed.
        If after the decrement the recursion level is still nonzero, the lock remains locked and
        owned by the calling thread.

        Only call this method when the calling thread owns the lock.
        A RuntimeError is raised if this method is called when the lock is unlocked.
        '''

        if self._owner != threading.current_thread().ident:
            raise RuntimeError('attempt to release lock not acquired by thread')

        if self._rcount:
            self._rcount -= 1

        if not self._rcount:
            self._last_owner = self._owner
            self._owner = None
            if self._queue.empty():
                self._queue.put(1)

    def __exit__(self, _ctype, _value, _traceback):
        self.release()


def install_public(self, instance, prefix=''):
    '''
    Monkey patch an instance of a class with methods coming from another instance

    This will include all the public methods of an instance in another instance with the same
    name or optionally the same name with a prefix

    Args:
        self (any): Instance of the class that imports the methods
        instance (any): Instance of the class that exports the methods
        prefix (str): Optional prefix to preprend to the imported methods
    '''

    methods = [method for method in dir(instance) if callable(getattr(instance, method))]
    methods = [method for method in methods if not method.startswith('_')]

    for entry in methods:
        setattr(self, '_'.join([prefix, entry]), getattr(instance, entry))


def check_package_editable(package):
    '''
    Check if package is installed in editable mode

    Args:
        package (str): Package name

    Returns:
        bool: Package installed in editable mode
    '''

    for path_item in sys.path:
        egg_link = os.path.join(path_item, package + '.egg-link')
        if os.path.isfile(egg_link):
            return True
    return False
